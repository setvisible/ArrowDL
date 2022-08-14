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
#include <cinttypes>
#include <cstdio> // for printf

#include "simulator/push_warnings.hpp"
#include <boost/system/error_code.hpp>
#include <boost/function.hpp>
#include "simulator/pop_warnings.hpp"

typedef sim::chrono::high_resolution_clock::time_point time_point;
typedef sim::chrono::high_resolution_clock::duration duration;

using namespace std::placeholders;

namespace sim {
namespace asio {
namespace ip {

	tcp::socket::socket(io_context& ios)
		: socket_base(ios)
		, m_connect_timer(ios)
		, m_recv_timer(ios)
	{}

	tcp::socket::socket(socket&& s)
		: socket_base(std::move(s))
		, m_connect_handler(std::move(s.m_connect_handler))
		, m_connect_timer(std::move(s.m_connect_timer))
		, m_mss(s.m_mss)
		, m_send_handler(std::move(s.m_send_handler))
		, m_wait_send_handler(std::move(s.m_wait_send_handler))
		, m_send_buffer(std::move(s.m_send_buffer))
		, m_incoming_queue(std::move(s.m_incoming_queue))
		, m_queue_size(std::move(s.m_queue_size))
		, m_recv_handler(std::move(s.m_recv_handler))
		, m_wait_recv_handler(std::move(s.m_wait_recv_handler))
		, m_recv_buffer(std::move(s.m_recv_buffer))
		, m_recv_timer(std::move(s.m_recv_timer))
		, m_is_v4(std::move(s.m_is_v4))
		, m_recv_null_buffers(std::move(s.m_recv_null_buffers))
		, m_send_null_buffers(std::move(s.m_send_null_buffers))
		, m_channel(std::move(s.m_channel))
		, m_next_outgoing_seq(std::move(s.m_next_outgoing_seq))
		, m_next_incoming_seq(std::move(s.m_next_incoming_seq))
		, m_last_drop_seq(std::move(s.m_last_drop_seq))
		, m_cwnd(std::move(s.m_cwnd))
		, m_bytes_in_flight(std::move(s.m_bytes_in_flight))
		, m_reorder_buffer(std::move(s.m_reorder_buffer))
		, m_outstanding_packet_sizes(std::move(s.m_outstanding_packet_sizes))
		, m_outgoing_packets(std::move(s.m_outgoing_packets))
	{
		if (m_forwarder) m_forwarder->reset(this);
		s.m_forwarder.reset();
		s.m_open = false;
		s.m_bound_to = ip::tcp::endpoint();

		if (m_bound_to != ip::tcp::endpoint())
			m_io_service.rebind_socket(this, m_bound_to);
	}

	tcp::socket::~socket()
	{
		boost::system::error_code ec;

		m_channel.reset();

		if (m_bound_to != ip::tcp::endpoint())
		{
			m_io_service.unbind_socket(this, m_bound_to);
			m_bound_to = ip::tcp::endpoint();
			m_user_bound_to = ip::tcp::endpoint();
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

	void tcp::socket::open(tcp protocol, boost::system::error_code& ec) try
	{
		close(ec);
		m_open = true;
		m_is_v4 = (protocol == ip::tcp::v4());
		ec.clear();
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

	void tcp::socket::open(tcp protocol)
	{
		boost::system::error_code ec;
		open(protocol, ec);
		if (ec) throw boost::system::system_error(ec);
	}

	// used to attach an incoming connection to this
	void tcp::socket::internal_connect(tcp::endpoint const& bind_ip
		, std::shared_ptr<aux::channel> const& c
		, boost::system::error_code& ec)
	{
		open(m_is_v4 ? tcp::v4() : tcp::v6(), ec);
		if (ec)
		{
			std::printf("tcp::socket::internal_connect() error: (%d) %s\n"
				, ec.value(), ec.message().c_str());
			return;
		}
		m_bound_to = bind_ip;
		m_user_bound_to = bind_ip;
		m_channel = c;
		assert(m_forwarder);
		c->hops[1].replace_last(m_forwarder);
	}

	void tcp::socket::bind(ip::tcp::endpoint const& ep
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

		ip::tcp::endpoint addr = m_io_service.bind_socket(this, ep, ec);
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

	void tcp::socket::bind(ip::tcp::endpoint const& ep)
	{
		boost::system::error_code ec;
		bind(ep, ec);
		if (ec) throw boost::system::system_error(ec);
	}

	void tcp::socket::close()
	{
		boost::system::error_code ec;
		close(ec);
		if (ec) throw boost::system::system_error(ec);
	}

	void tcp::socket::close(boost::system::error_code& ec) try
	{
		if (m_channel)
		{
			int const remote = m_channel->remote_idx(m_bound_to);
			route hops = m_channel->hops[remote];

			// if m_connect_handler is still set, it means the connection hasn't
			// been established yet, and this channel points to the acceptor
			// socket, not another open TCP connection.
			if (!hops.empty() && !m_connect_handler)
			{
				aux::packet p;
				p.type = aux::packet::type_t::error;
				p.ec = asio::error::eof;
				p.from = asio::ip::udp::endpoint(
					m_bound_to.address(), m_bound_to.port());
				p.overhead = 40;
				p.hops = hops;
				p.seq_nr = m_next_outgoing_seq++;
				send_packet(std::move(p));
			}
			m_channel.reset();
		}

		if (m_bound_to != ip::tcp::endpoint())
		{
			m_io_service.unbind_socket(this, m_bound_to);
			m_bound_to = ip::tcp::endpoint();
			m_user_bound_to = ip::tcp::endpoint();
		}
		m_open = false;

		// prevent any more packets from being delivered to this socket
		if (m_forwarder)
		{
			m_forwarder->reset();
			m_forwarder.reset();
		}

		// reset socket state
		m_queue_size = 0;
		m_mss = 1475;
		m_cwnd = m_mss * 2;
		m_bytes_in_flight = 0;
		m_outstanding_packet_sizes.clear();
		m_recv_null_buffers = false;
		m_send_null_buffers = false;
		m_next_incoming_seq = 0;
		m_next_outgoing_seq = 0;
		m_last_drop_seq = 0;

		cancel(ec);

		ec.clear();
	}
	catch (std::bad_alloc const&)
	{
		ec = make_error_code(boost::system::errc::not_enough_memory);
	}
	catch (boost::system::system_error const& err)
	{
		ec = err.code();
	}

	std::size_t tcp::socket::available(boost::system::error_code& ec) const
	{
		if (!m_open)
		{
			ec = boost::system::error_code(error::bad_descriptor);
			return 0;
		}
		if (!m_channel)
		{
			ec = boost::system::error_code(error::not_connected);
			return 0;
		}
		if (m_incoming_queue.empty())
		{
			return 0;
		}

		std::size_t ret = 0;
		for (aux::packet const& p : m_incoming_queue)
		{
			if (p.type == aux::packet::type_t::error)
			{
				if (ret > 0) return ret;

				// if the read buffer is drained and there is an error, report that
				// error.
				ec = p.ec;
				return 0;
			}
			ret += p.buffer.size();
		}
		return ret;
	}

	std::size_t tcp::socket::available() const
	{
		boost::system::error_code ec;
		std::size_t ret = available(ec);
		if (ec) throw boost::system::system_error(ec);
		return ret;
	}

	void tcp::socket::cancel(boost::system::error_code&)
	{
		abort_recv_handlers();
		abort_send_handlers();

		if (m_connect_handler)
		{
			post(m_io_service, aux::make_malloc(std::bind(std::move(m_connect_handler)
				, boost::system::error_code(error::operation_aborted))));
			m_connect_handler = nullptr;
		}
	}

	void tcp::socket::cancel()
	{
		boost::system::error_code ec;
		cancel(ec);
		if (ec) throw boost::system::system_error(ec);
	}

	tcp::endpoint tcp::socket::remote_endpoint(boost::system::error_code& ec) const
	{
		if (!m_open)
		{
			ec = error::bad_descriptor;
			return tcp::endpoint();
		}

		if (!m_channel)
		{
			ec = error::not_connected;
			return tcp::endpoint();
		}

		int const remote = m_channel->remote_idx(m_bound_to);
		return m_channel->visible_ep[remote];
	}

	tcp::endpoint tcp::socket::remote_endpoint() const
	{
		boost::system::error_code ec;
		tcp::endpoint ret = remote_endpoint(ec);
		if (ec) throw boost::system::system_error(ec);
		return ret;
	}

	void tcp::socket::async_connect(tcp::endpoint const& target
		, aux::function<void(boost::system::error_code const&)> h)
	{
		if (!m_open) open(target.protocol());

		assert(h);
		assert(!m_connect_handler);

		// find remote socket
		boost::system::error_code ec;
		if (m_bound_to.address() == ip::address())
		{
			auto endpoint = ip::tcp::endpoint();
			if (target.address().is_v4()) {
				endpoint.address(ip::address_v4::any());
			} else {
				endpoint.address(ip::address_v6::any());
			}
			ip::tcp::endpoint addr = m_io_service.bind_socket(this
				, endpoint, ec);
			if (ec)
			{
				post(m_io_service, aux::make_malloc(std::bind(std::move(h), ec)));
				return;
			}
			m_bound_to = addr;
			m_user_bound_to = addr;
		}
		if (m_bound_to.address().is_v4() != target.address().is_v4())
		{
			post(m_io_service, aux::make_malloc(std::bind(std::move(h),
				boost::system::error_code(error::address_family_not_supported))));
			return;
		}
		m_channel = m_io_service.internal_connect(this, target, ec);
		m_mss = m_io_service.get_path_mtu(m_bound_to.address(), target.address());
		m_cwnd = m_mss * 2;
		if (ec)
		{
			m_channel.reset();
			// TODO: ask the policy object what the round-trip to this endpoint is
			m_connect_timer.expires_after(chrono::milliseconds(50));
			m_connect_timer.async_wait(aux::make_malloc(std::bind(std::move(h), ec)));
			return;
		}

		m_connect_handler = std::move(h);

		// the acceptor socket will call internal_connect_complete once the
		// connection is established
	}

	void tcp::socket::abort_recv_handlers()
	{
		if (m_recv_handler) post(m_io_service, aux::make_malloc(std::bind(std::move(m_recv_handler)
			, boost::system::error_code(error::operation_aborted), std::size_t(0))));

		if (m_wait_recv_handler) post(m_io_service, aux::make_malloc(std::bind(std::move(m_wait_recv_handler)
			, boost::system::error_code(error::operation_aborted))));

		m_recv_timer.cancel();
		m_recv_handler = nullptr;
		m_wait_recv_handler = nullptr;
		m_recv_buffer.clear();
		m_recv_null_buffers = false;
	}

	void tcp::socket::abort_send_handlers()
	{
		if (m_send_handler) post(m_io_service, aux::make_malloc(std::bind(std::move(m_send_handler)
			, boost::system::error_code(error::operation_aborted), std::size_t(0))));

		if (m_wait_send_handler) post(m_io_service, aux::make_malloc(std::bind(std::move(m_wait_send_handler)
			, boost::system::error_code(error::operation_aborted))));

		m_send_handler = nullptr;
		m_wait_send_handler = nullptr;
		m_send_buffer.clear();
		m_send_null_buffers = false;
	}

	void tcp::socket::async_write_some_impl(std::vector<boost::asio::const_buffer> const& bufs
		, aux::function<void(boost::system::error_code const&, std::size_t)> handler)
	{
		int buf_size = 0;
		for (int i = 0; i < int(bufs.size()); ++i)
			buf_size += int(bufs[i].size());

		boost::system::error_code ec;
		std::size_t const bytes_transferred = write_some_impl(bufs, ec);
		if (ec == boost::system::error_code(error::would_block))
		{
			m_send_handler = std::move(handler);
			m_send_buffer = bufs;
			return;
		}

		if (ec)
		{
			post(m_io_service, aux::make_malloc(std::bind(std::move(handler), ec, std::size_t(0))));
			m_send_handler = nullptr;
			m_send_buffer.clear();
			return;
		}

		boost::system::error_code no_error;
		post(m_io_service, aux::make_malloc(std::bind(std::move(handler), no_error
			, bytes_transferred)));
		m_send_handler = nullptr;
		m_send_buffer.clear();
	}

	std::size_t tcp::socket::write_some_impl(
		std::vector<boost::asio::const_buffer> const& bufs
		, boost::system::error_code& ec)
	{
		if (!m_open)
		{
			ec = boost::system::error_code(error::bad_descriptor);
			return 0;
		}
		if (!m_channel)
		{
			ec = boost::system::error_code(error::not_connected);
			return 0;
		}

		// the connect handler is used as proxy that this socket has not competed
		// the connection yet. We're still waiting for SYN+ACK
		if (m_connect_handler)
		{
			ec = boost::system::error_code(error::would_block);
			return 0;
		}

		int const remote = m_channel->remote_idx(m_bound_to);
		route hops = m_channel->hops[remote];
		if (hops.empty())
		{
			ec = boost::system::error_code(error::not_connected);
			return 0;
		}

		if (m_bytes_in_flight + m_mss > m_cwnd)
		{
			// this indicates that the send buffer is very large, we should
			// probably not be able to stuff more bytes down it
			// wait for the receiving end to pop some bytes off
			ec = boost::system::error_code(error::would_block);
			return 0;
		}

		std::size_t ret = 0;

		for (auto const& buf : bufs)
		{
			// split up in packets
			int buf_size = int(buf.size());
			std::uint8_t const* ptr = static_cast<std::uint8_t const*>(buf.data());
			while (buf_size > 0)
			{
				int packet_size = (std::min)(buf_size, m_mss);
				aux::packet p;
				p.type = aux::packet::type_t::payload;
				p.buffer.assign(ptr, ptr + packet_size);
				p.from = asio::ip::udp::endpoint(
					m_bound_to.address(), m_bound_to.port());
				p.overhead = 40;
				p.hops = hops;
				p.seq_nr = m_next_outgoing_seq++;
				p.drop_fun = std::bind(&tcp::socket::packet_dropped, this, _1);

				send_packet(std::move(p));
				ptr += packet_size;
				buf_size -= packet_size;
				ret += packet_size;

				if (m_bytes_in_flight + m_mss > m_cwnd)
				{
					// the congestion window is full
					if (ret == 0)
					{
						ec = boost::system::error_code(error::would_block);
						return 0;
					}

					return ret;
				}
			}
		}

		return ret;
	}

	std::size_t tcp::socket::read_some_impl(
		std::vector<boost::asio::mutable_buffer> const& bufs
		, boost::system::error_code& ec)
	{
		assert(!bufs.empty());
		if (!m_open)
		{
			ec = boost::system::error_code(error::bad_descriptor);
			return 0;
		}
		if (!m_channel)
		{
			ec = boost::system::error_code(error::not_connected);
			return 0;
		}
		if (m_connect_handler)
		{
			// the socket is not done connecting yet
			ec = boost::system::error_code(error::would_block);
			return 0;
		}

		if (m_incoming_queue.empty())
		{
			ec = boost::system::error_code(error::would_block);
			return 0;
		}

		typedef std::vector<boost::asio::mutable_buffer> buffers_t;
		m_recv_buffer = bufs;
		buffers_t::iterator recv_iter = m_recv_buffer.begin();
		int total_received = 0;
		// the offset in the current receive buffer we're writing to. i.e. the
		// buffer recv_iter points to
		int buf_offset = 0;

		while (!m_incoming_queue.empty())
		{
			aux::packet& p = m_incoming_queue.front();

			if (p.type == aux::packet::type_t::error)
			{
				// if we have received bytes also, first deliver those. In the next
				// read, deliver the error
				if (total_received > 0) break;

				assert(p.ec);
				ec = p.ec;
				m_incoming_queue.erase(m_incoming_queue.begin());
				m_channel.reset();
				return 0;
			}
			else if (p.type == aux::packet::type_t::payload)
			{
				// copy bytes from the incoming queue into the receive buffer.
				// both are vectors of buffer, so it can get a bit hairy
				while (recv_iter != m_recv_buffer.end())
				{
					int const buf_size = int(recv_iter->size());
					int const copy_size = (std::min)(int(p.buffer.size())
						, buf_size - buf_offset);

					memcpy(static_cast<char*>(recv_iter->data()) + buf_offset
						, p.buffer.data(), copy_size);

					p.buffer.erase(p.buffer.begin(), p.buffer.begin() + copy_size);
					m_queue_size -= copy_size;

					buf_offset += copy_size;
					assert(buf_offset <= buf_size);
					total_received += copy_size;
					if (buf_offset == buf_size)
					{
						++recv_iter;
						buf_offset = 0;
					}

					if (p.buffer.empty())
					{
						m_incoming_queue.erase(m_incoming_queue.begin());
						break;
					}
				}
			}
			else
			{
				assert(false);
			}

			if (recv_iter == m_recv_buffer.end())
				break;
		}

		assert(total_received > 0);

		ec.clear();
		return total_received;
	}

	void tcp::socket::async_read_some_impl(std::vector<boost::asio::mutable_buffer> const& bufs
		, aux::function<void(boost::system::error_code const&, std::size_t)> handler)
	{
		assert(!bufs.empty());
		assert(bufs[0].size());

		boost::system::error_code ec;
		std::size_t bytes_transferred = read_some_impl(bufs, ec);
		if (ec == boost::system::error_code(error::would_block))
		{
			assert(m_incoming_queue.empty());

			m_recv_buffer = bufs;
			m_recv_handler = std::move(handler);
			m_recv_null_buffers = false;
			return;
		}

		if (ec)
		{
			post(m_io_service, aux::make_malloc(std::bind(std::move(handler), ec, std::size_t(0))));
			m_recv_handler = nullptr;
			m_recv_buffer.clear();
			return;
		}

		post(m_io_service, aux::make_malloc(std::bind(std::move(handler), ec, bytes_transferred)));
		m_recv_handler = nullptr;
		m_recv_buffer.clear();
	}

	void tcp::socket::async_wait_read_impl(
		aux::function<void(boost::system::error_code const&)> handler)
	{
		boost::system::error_code ec;
		// null_buffers notifies the handler when data is available, without
		// reading any
		int const bytes = int(available(ec));
		if (ec)
		{
			post(m_io_service, aux::make_malloc(std::bind(std::move(handler), ec)));
			m_recv_handler = nullptr;
			m_recv_buffer.clear();
			return;
		}

		if (bytes > 0)
		{
			post(m_io_service, aux::make_malloc(std::bind(std::move(handler), ec)));
			m_recv_handler = nullptr;
			m_recv_buffer.clear();
			return;
		}

		m_wait_recv_handler = std::move(handler);
		m_recv_null_buffers = true;
	}

	// if there is an outstanding read operation, and this was the first incoming
	// operation since we last drained, wake up the reader
	void tcp::socket::maybe_wakeup_reader()
	{
		if (m_incoming_queue.size() != 1 || (!m_recv_handler && !m_wait_recv_handler)) return;

		if (m_recv_null_buffers)
		{
			async_wait_read_impl(std::move(m_wait_recv_handler));
		}
		else
		{
			// we have an async. read operation outstanding, and we just put one
			// packet in our incoming queue.

			// try to read from it and potentially fire the handler
			async_read_some_impl(m_recv_buffer, std::move(m_recv_handler));
		}
	}

	void tcp::socket::maybe_wakeup_writer()
	{
		if (!m_send_handler) return;

		if (m_send_null_buffers)
		{
			assert(false && "not supported yet");
//			async_wait_write_impl(m_recv_handler);
		}
		else
		{
			// we have an async. write operation outstanding
			async_write_some_impl(m_send_buffer, std::move(m_send_handler));
		}
	}

	bool tcp::socket::internal_is_listening() { return false; }

	void tcp::socket::send_packet(aux::packet p)
	{
		m_bytes_in_flight += int(p.buffer.size());
		m_outstanding_packet_sizes[p.seq_nr] = int(p.buffer.size());

		int const idx = m_channel->self_idx(m_bound_to);
		p.byte_counter = m_channel->bytes_sent[idx];
		m_channel->bytes_sent[idx] += std::uint32_t(p.buffer.size());

		auto* log = m_io_service.sim().get_pcap();
		if (log)
		{
			int const remote = m_channel->remote_idx(m_bound_to);
			log->log_tcp(p, m_bound_to, m_channel->ep[remote]);
		}

		forward_packet(std::move(p));
	}

	void tcp::socket::packet_dropped(aux::packet p)
	{
		int remote = m_channel->remote_idx(m_bound_to);
		p.hops = m_channel->hops[remote];
		m_outgoing_packets.push_back(std::move(p));

		const int packets_in_cwnd = m_cwnd / m_mss;

		// we just recently dropped a packet and cut the cwnd in half,
		// don't do it again already
		if (m_last_drop_seq > 0 && p.seq_nr < m_last_drop_seq + packets_in_cwnd) return;

		m_cwnd /= 2;
		m_last_drop_seq = p.seq_nr;

		// TODO: this should really happen one second later to be accurate
		if (m_cwnd < m_mss) m_cwnd = m_mss;
	}

	void tcp::socket::incoming_packet(aux::packet p)
	{
		switch (p.type)
		{
			case aux::packet::type_t::uninitialized:
			{
				assert(false && "uninitialized packet");
				return;
			}
			case aux::packet::type_t::ack:
			{
				// if the socket just became writeable, we need to notify the
				// client. First we want to know whether it was not writeable.
				const bool was_writeable = m_bytes_in_flight + m_mss > m_cwnd;

				auto it = m_outstanding_packet_sizes.find(p.seq_nr);
				assert(it != m_outstanding_packet_sizes.end());
				const int acked_bytes = it->second;
				m_outstanding_packet_sizes.erase(it);
				assert(m_bytes_in_flight >= acked_bytes);
				m_bytes_in_flight -= acked_bytes;

				// potentially resend packets
				while (!m_outgoing_packets.empty()
					&& m_bytes_in_flight
						+ int(m_outgoing_packets.front().buffer.size()) <= m_cwnd)
				{
					aux::packet pkt = std::move(m_outgoing_packets.front());
					m_outgoing_packets.erase(m_outgoing_packets.begin());
					send_packet(std::move(pkt));
				}

				// update cwnd based on the number of bytes ACKed.
				// every round-trip, increase the window size by one packet
				// (MSS)
				m_cwnd += m_mss * acked_bytes / m_cwnd;

				// TODO: implement slow-start

				const bool is_writeable = m_bytes_in_flight + m_mss <= m_cwnd;

				if (!was_writeable && is_writeable)
					maybe_wakeup_writer();

				return;
			}
			case aux::packet::type_t::syn:
			{
				// TODO: return connection refused
				return;
			}
			case aux::packet::type_t::syn_ack:
			{
				assert(m_connect_handler);
				boost::system::error_code ec;
				post(m_io_service, aux::make_malloc(std::bind(std::move(m_connect_handler), ec)));
				m_connect_handler = nullptr;
				if (ec) m_channel.reset();
				else maybe_wakeup_writer();
				return;
			}
			case aux::packet::type_t::error:
			case aux::packet::type_t::payload:
			{
				aux::packet ack;
				ack.type = aux::packet::type_t::ack;
				ack.seq_nr = p.seq_nr;

				int remote = m_channel->remote_idx(m_bound_to);
				ack.hops = m_channel->hops[remote];
				forward_packet(std::move(ack));

				// if the sequence number is out-of-order, put it in the
				// m_incoming_packets queue
				if (p.seq_nr != m_next_incoming_seq)
				{
					if (p.seq_nr < m_next_incoming_seq)
					{
						std::printf("TCP: incoming sequence number lower (%" PRId64 ") "
							"than expected: %" PRId64 "\n", p.seq_nr, m_next_incoming_seq);
					}

					m_reorder_buffer.emplace(p.seq_nr, std::move(p));
					return;
				}

				// this packet was in-order. increment the expected next sequence
				// number.
				++m_next_incoming_seq;
				m_incoming_queue.push_back(std::move(p));

				// also, perhaps there are some packets that arrived out-of-order,
				// check to see
				auto it = m_reorder_buffer.find(m_next_incoming_seq);
				while (it != m_reorder_buffer.end())
				{
					aux::packet pkt = std::move(it->second);
					m_reorder_buffer.erase(it);
					m_incoming_queue.push_back(std::move(pkt));
					++m_next_incoming_seq;
					it = m_reorder_buffer.find(m_next_incoming_seq);
				}

				maybe_wakeup_reader();
				return;
			}
		}
	}
}
}
}

