/*

Copyright (c) 2015, Arvid Norberg
All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "simulator/simulator.hpp"
#include "simulator/packet.hpp"
#include "simulator/pcap.hpp"
#include "simulator/handler_allocator.hpp"

#include <functional>

#include "simulator/push_warnings.hpp"
#include <boost/system/error_code.hpp>
#include <boost/function.hpp>
#include "simulator/pop_warnings.hpp"

typedef sim::chrono::high_resolution_clock::time_point time_point;
typedef sim::chrono::high_resolution_clock::duration duration;

namespace sim {
namespace asio {
namespace ip {

	udp::socket::socket(io_context& ios)
		: socket_base(ios)
		, m_next_send(chrono::high_resolution_clock::now())
		, m_recv_sender(nullptr)
		, m_recv_timer(ios)
		, m_send_timer(ios)
		, m_recv_null_buffers(0)
		, m_queue_size(0)
		, m_is_v4(true)
	{}

	udp::socket::socket(socket&& s)
		: socket_base(std::move(s))
		, m_next_send(std::move(s.m_next_send))
		, m_send_handler(std::move(s.m_send_handler))
		, m_wait_send_handler(std::move(s.m_wait_send_handler))
		, m_recv_handler(std::move(s.m_recv_handler))
		, m_wait_recv_handler(std::move(s.m_wait_recv_handler))
		, m_recv_buffer(std::move(s.m_recv_buffer))
		, m_recv_sender(std::move(s.m_recv_sender))
		, m_recv_timer(std::move(s.m_recv_timer))
		, m_send_timer(std::move(s.m_send_timer))
		, m_incoming_queue(std::move(s.m_incoming_queue))
		, m_recv_null_buffers(std::move(s.m_recv_null_buffers))
		, m_queue_size(std::move(s.m_queue_size))
		, m_is_v4(s.m_is_v4)
	{
		if (m_forwarder) m_forwarder->reset(this);
		s.m_forwarder.reset();
		s.m_open = false;
		s.m_bound_to = ip::udp::endpoint();
		if (m_bound_to != ip::udp::endpoint())
			m_io_service.rebind_udp_socket(this, m_bound_to);
	}

	udp::socket::~socket()
	{
		boost::system::error_code ec;
		close(ec);
	}

	void udp::socket::bind(ip::udp::endpoint const& ep
		, boost::system::error_code& ec) try
	{
		if (!m_open)
		{
			ec = error::bad_descriptor;
			return;
		}

		if (ep.address().is_v4() != m_is_v4)
		{
			ec = error::address_family_not_supported;
			return;
		}

		ip::udp::endpoint addr = m_io_service.bind_udp_socket(this, ep, ec);
		if (ec) return;
		m_bound_to = addr;
		m_user_bound_to = addr;
	}
	catch (std::bad_alloc const&)
	{
		ec = make_error_code(boost::system::errc::not_enough_memory);
	}
	catch (boost::system::system_error const& err)
	{
		ec = err.code();
	}

	void udp::socket::bind(ip::udp::endpoint const& ep)
	{
		boost::system::error_code ec;
		bind(ep, ec);
		if (ec) throw boost::system::system_error(ec);
	}

	void udp::socket::open(udp protocol
		, boost::system::error_code& ec) try
	{
		// TODO: what if it's already open?
		close(ec);
		m_open = true;
		m_is_v4 = (protocol == ip::udp::v4());
		m_forwarder = std::make_shared<aux::sink_forwarder>(this);
	}
	catch (std::bad_alloc const&)
	{
		ec = make_error_code(boost::system::errc::not_enough_memory);
	}
	catch (boost::system::system_error const& err)
	{
		ec = err.code();
	}

	void udp::socket::open(udp protocol)
	{
		boost::system::error_code ec;
		open(protocol, ec);
		if (ec) throw boost::system::system_error(ec);
	}

	void udp::socket::close()
	{
		boost::system::error_code ec;
		close(ec);
		if (ec) throw boost::system::system_error(ec);
	}

	void udp::socket::close(boost::system::error_code& ec) try
	{
		if (m_bound_to != ip::udp::endpoint())
		{
			m_io_service.unbind_udp_socket(this, m_bound_to);
			m_bound_to = ip::udp::endpoint();
			m_user_bound_to = ip::udp::endpoint();
		}
		m_open = false;

		// prevent any more packets from being delivered to this socket
		if (m_forwarder)
		{
			m_forwarder->reset();
			m_forwarder.reset();
		}

		cancel(ec);
	}
	catch (std::bad_alloc const&)
	{
		ec = make_error_code(boost::system::errc::not_enough_memory);
	}
	catch (boost::system::system_error const& err)
	{
		ec = err.code();
	}

	void udp::socket::cancel(boost::system::error_code&)
	{
		// cancel outstanding async operations
		abort_recv_handlers();
		abort_send_handlers();
		m_recv_timer.cancel();
		m_send_timer.cancel();
		return;
	}

	void udp::socket::cancel()
	{
		boost::system::error_code ec;
		cancel(ec);
		if (ec) throw boost::system::system_error(ec);
	}

	void udp::socket::abort_send_handlers()
	{
		if (m_send_handler)
			post(m_io_service, make_malloc(std::bind(std::ref(m_send_handler)
				, boost::system::error_code(error::operation_aborted), std::size_t(0))));

		if (m_wait_send_handler)
			post(m_io_service, make_malloc(std::bind(std::ref(m_wait_send_handler)
				, boost::system::error_code(error::operation_aborted))));

		m_send_timer.cancel();
		m_send_handler = nullptr;
		m_wait_send_handler = nullptr;
//		m_send_buffer.clear();
	}

	void udp::socket::abort_recv_handlers()
	{
		if (m_recv_handler)
			post(m_io_service, make_malloc(std::bind(std::move(m_recv_handler)
				, boost::system::error_code(error::operation_aborted), std::size_t(0))));

		if (m_wait_recv_handler)
			post(m_io_service, make_malloc(std::bind(std::move(m_wait_recv_handler)
				, boost::system::error_code(error::operation_aborted))));

		m_recv_timer.cancel();
		m_recv_handler = nullptr;
		m_wait_recv_handler = nullptr;
		m_recv_buffer.clear();
	}

	void udp::socket::async_wait(socket_base::wait_type_t const w
		, aux::function<void(boost::system::error_code const&)> handler)
	{
		if (w == wait_type_t::wait_write)
		{
			abort_send_handlers();

			// TODO: make the send buffer size configurable
			time_point const now = chrono::high_resolution_clock::now();
			boost::system::error_code no_error;
			if (m_next_send - now > chrono::milliseconds(1000))
			{
				// our send queue is too large. Defer
				m_recv_timer.expires_at(m_next_send - chrono::milliseconds(1000) / 2);

				m_wait_send_handler = std::move(handler);
				m_recv_timer.async_wait(make_malloc(std::bind(std::ref(m_wait_send_handler), no_error)));
				return;
			}

			// the socket is writable, post the completion handler immediately
			post(m_io_service, make_malloc(std::bind(std::move(handler), no_error)));
		}
		else if (w == wait_type_t::wait_read)
		{
			abort_recv_handlers();
			async_wait_receive_impl(nullptr, std::move(handler));
		}
	}

	std::size_t udp::socket::receive_from_impl(
		std::vector<asio::mutable_buffer> const& bufs
		, udp::endpoint* sender
		, socket_base::message_flags /* flags */
		, boost::system::error_code& ec)
	{
		assert(!bufs.empty());
		if (!m_open)
		{
			ec = boost::system::error_code(error::bad_descriptor);
			return 0;
		}

		if (m_bound_to == udp::endpoint())
		{
			ec = boost::system::error_code(error::invalid_argument);
			return 0;
		}

		if (m_incoming_queue.empty())
		{
			ec = boost::system::error_code(error::would_block);
			return 0;
		}

		aux::packet& p = m_incoming_queue.front();
		if (sender) *sender = p.from;

		int read = 0;
		for (auto const& buf : bufs)
		{
			char* ptr = static_cast<char*>(buf.data());
			int const len = int(buf.size());
			int const to_copy = (std::min)(int(p.buffer.size()), len);
			memcpy(ptr, p.buffer.data(), to_copy);
			read += to_copy;
			p.buffer.erase(p.buffer.begin(), p.buffer.begin() + to_copy);
			m_queue_size -= to_copy;
			if (p.buffer.empty()) break;
		}

		m_incoming_queue.erase(m_incoming_queue.begin());
		return read;
	}

	void udp::socket::async_wait_receive_impl(
		udp::endpoint* sender
		, aux::function<void(boost::system::error_code const&)> handler)
	{
		if (!m_open)
		{
			post(m_io_service, make_malloc(std::bind(std::move(handler)
				, boost::system::error_code(error::bad_descriptor))));
			return;
		}

		if (m_bound_to == udp::endpoint())
		{
			post(m_io_service, make_malloc(std::bind(std::move(handler)
				, boost::system::error_code(error::invalid_argument))));
			return;
		}

		if (!m_incoming_queue.empty())
		{
			post(m_io_service, make_malloc(std::bind(std::move(handler), boost::system::error_code())));
			return;
		}

		m_recv_null_buffers = true;
		m_wait_recv_handler = std::move(handler);
		m_recv_sender = sender;
	}

	void udp::socket::async_receive_from_impl(
		std::vector<asio::mutable_buffer> const& bufs
		, udp::endpoint* sender
		, socket_base::message_flags /* flags */
		, aux::function<void(boost::system::error_code const&
			, std::size_t)> handler)
	{
		assert(!bufs.empty());

		boost::system::error_code ec;
		std::size_t bytes_transferred = receive_from_impl(bufs, sender, 0, ec);
		if (ec == boost::system::error_code(error::would_block))
		{
			m_recv_buffer = bufs;
			m_recv_handler = std::move(handler);
			m_recv_sender = sender;
			m_recv_null_buffers = false;

			return;
		}

		if (ec)
		{
			post(m_io_service, make_malloc(std::bind(std::move(handler), ec, std::size_t(0))));
			m_recv_handler = nullptr;
			m_recv_buffer.clear();
			m_recv_sender = nullptr;
			m_recv_null_buffers = false;
			return;
		}

		post(m_io_service, make_malloc(std::bind(std::move(handler), ec, bytes_transferred)));
		m_recv_handler = nullptr;
		m_recv_buffer.clear();
		m_recv_sender = nullptr;
		m_recv_null_buffers = false;
	}

	std::size_t udp::socket::send_to_impl(std::vector<asio::const_buffer> const& b
		, udp::endpoint const& dst, message_flags /* flags */
		, boost::system::error_code& ec)
	{
		assert(m_non_blocking && "blocking operations not supported");

		if (m_bound_to == ip::udp::endpoint())
		{
			// the socket was not bound, bind to anything
			bind(udp::endpoint(), ec);
			if (ec) return 0;
		}

		ec.clear();
		std::size_t ret = 0;
		for (std::vector<asio::const_buffer>::const_iterator i = b.begin()
			, end(b.end()); i != end; ++i)
		{
			ret += i->size();
		}
		if (ret == 0)
		{
			ec = boost::system::error_code(error::invalid_argument);
			return 0;
		}

		time_point now = chrono::high_resolution_clock::now();

		// this is outgoing NIC bandwidth
		// TODO: make this configurable
		const int bandwidth = 100000000; // 100 MB/s
		const int mtu = m_io_service.get_path_mtu(m_bound_to.address(), dst.address());

		if (int(ret) > 65535)
		{
			ec = boost::system::error_code(error::message_size);
			return 0;
		}

		if (m_dont_fragment && int(ret) > mtu)
		{
			// silently drop packet
			ec.clear();
			return ret;
		}

		// determine the bandwidth in terms of nanoseconds / byte
		const double nanoseconds_per_byte = 1000000000.0
			/ double(bandwidth);

		// TODO: make the send buffer size configurable
		if (m_next_send - now > chrono::milliseconds(500))
		{
			// our send queue is too large.
			ec = boost::system::error_code(asio::error::would_block);
			return 0;
		}

		route hops = m_io_service.find_udp_socket(*this, dst);
		if (hops.empty())
		{
			// the packet is silently dropped
			// TODO: it would be nice if this would result in a round-trip time
			// with an ICMP host unreachable or connection_refused error
			return ret;
		}

		hops.prepend(m_io_service.get_outgoing_route(m_bound_to.address()));

		m_next_send = std::max(now, m_next_send);

		aux::packet p;
		p.overhead = 28;
		p.type = aux::packet::type_t::payload;
		p.from = m_bound_to;
		p.hops = hops;
		for (std::vector<asio::const_buffer>::const_iterator i = b.begin()
			, end(b.end()); i != end; ++i)
		{
			p.buffer.insert(p.buffer.end(), static_cast<std::uint8_t const*>(i->data())
				, static_cast<std::uint8_t const*>(i->data()) + i->size());
		}

		auto* log = m_io_service.sim().get_pcap();
		if (log) log->log_udp(p, m_bound_to, dst);

		int const packet_size = int(p.buffer.size() + p.overhead);
		forward_packet(std::move(p));

		m_next_send += chrono::duration_cast<duration>(chrono::nanoseconds(
			boost::int64_t(nanoseconds_per_byte * packet_size)));

		return ret;
	}

	void udp::socket::incoming_packet(aux::packet p)
	{
		int const packet_size = int(p.buffer.size() + p.overhead);

		// silent drop. If the application isn't reading fast enough, drop packets
		// TODO: make this limit controlled by SO_RECVBUF socket option
		if (m_queue_size + packet_size > 256 * 1024) return;

		m_queue_size += int(p.buffer.size());
		m_incoming_queue.push_back(std::move(p));

		maybe_wakeup_reader();
	}

	void udp::socket::maybe_wakeup_reader()
	{
		if (m_incoming_queue.size() != 1 || (!m_recv_handler && !m_wait_recv_handler)) return;

		// there is an outstanding operation waiting for an incoming packet
		if (m_recv_null_buffers)
		{
			async_wait_receive_impl(m_recv_sender, std::move(m_wait_recv_handler));
		}
		else
		{
			async_receive_from_impl(m_recv_buffer, m_recv_sender, 0, std::move(m_recv_handler));
		}

//		m_recv_handler = nullptr;
//		m_recv_buffer.clear();
//		m_recv_sender = nullptr;
	}

} // ip
} // asio
} // sim

