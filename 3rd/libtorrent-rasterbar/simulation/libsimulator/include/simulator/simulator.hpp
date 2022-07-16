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

#ifndef SIMULATOR_HPP_INCLUDED
#define SIMULATOR_HPP_INCLUDED

#include "simulator/push_warnings.hpp"

#include <boost/config.hpp>
#include <boost/asio/detail/config.hpp>
#include <boost/asio/basic_deadline_timer.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/ip/network_v4.hpp>
#include <boost/asio/buffers_iterator.hpp>

#include <boost/asio/post.hpp>
#include <boost/asio/defer.hpp>
#include <boost/asio/dispatch.hpp>

#include <boost/optional.hpp>

#include "simulator/pop_warnings.hpp"

#include "simulator/chrono.hpp"
#include "simulator/sink_forwarder.hpp"
#include "simulator/function.hpp"
#include "simulator/noexcept_movable.hpp"
#include "simulator/mallocator.hpp"

#include <deque>
#include <mutex>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <list>
#include <functional>

#ifndef IP_DONTFRAGMENT
#define IP_DONTFRAGMENT 1
#endif

namespace sim
{
	namespace aux
	{
		struct channel;
		struct packet;
		struct pcap;
	}

	// this represents a network route (a series of sinks to pass a packet
	// through)
	struct SIMULATOR_DECL route
	{
		friend route operator+(route lhs, route rhs)
		{ return std::move(lhs.append(std::move(rhs))); }

		std::shared_ptr<sink> next_hop() const { return hops.front(); }
		std::shared_ptr<sink> pop_front()
		{
			if (hops.empty()) return std::shared_ptr<sink>();
			std::shared_ptr<sink> ret(std::move(hops.front()));
			hops.erase(hops.begin());
			return ret;
		}
		void replace_last(std::shared_ptr<sink> s) { hops.back() = std::move(s); }
		void prepend(route const& r)
		{ hops.insert(hops.begin(), r.hops.begin(), r.hops.end()); }
		void prepend(std::shared_ptr<sink> s) { hops.insert(hops.begin(), std::move(s)); }
		route& append(route const& r)
		{ hops.insert(hops.end(), r.hops.begin(), r.hops.end()); return *this; }
		route& append(std::shared_ptr<sink> s) { hops.push_back(std::move(s)); return *this; }
		bool empty() const { return hops.empty(); }
		std::shared_ptr<sink> last() const
		{ return hops.back(); }

	private:
		std::deque<std::shared_ptr<sink>, aux::mallocator<std::shared_ptr<sink>>> hops;
	};

	void forward_packet(aux::packet p);

	struct simulation;
	struct configuration;
	struct queue;

	namespace asio
	{

	using boost::asio::buffer_size;
	using boost::asio::const_buffer;
	using boost::asio::mutable_buffer;
	using boost::asio::buffer;

	using boost::asio::post;
	using boost::asio::dispatch;
	using boost::asio::defer;

	struct io_context;

	struct io_executor
	{
		io_executor(io_context& ctx) : m_ctx(&ctx) {}
		io_context& context() const { return *m_ctx; }

		template <typename Handler, typename Allocator>
		void dispatch(Handler handler, Allocator const& a) const;

		template <typename Handler, typename Allocator>
		void post(Handler handler, Allocator const& a) const;

		template <typename Handler, typename Allocator>
		void defer(Handler handler, Allocator const& a) const;

		void on_work_finished() const {}
		void on_work_started() const {}

		bool running_in_this_thread() const { return true; }

		friend bool operator==(io_executor const& lhs, io_executor const& rhs)
		{ return lhs.m_ctx == rhs.m_ctx; }

		friend bool operator!=(io_executor const& lhs, io_executor const& rhs)
		{ return lhs.m_ctx != rhs.m_ctx; }

	private:
		io_context* m_ctx;
	};

	struct SIMULATOR_DECL high_resolution_timer
	{
		friend struct sim::simulation;

		using time_type = chrono::high_resolution_clock::time_point;
		using duration_type = chrono::high_resolution_clock::duration;

		using executor_type = io_executor;
		executor_type get_executor();

		explicit high_resolution_timer(io_context& io_context);
		high_resolution_timer(io_context& io_context,
			const time_type& expiry_time);
		high_resolution_timer(io_context& io_context,
			const duration_type& expiry_time);
		high_resolution_timer(high_resolution_timer&&) noexcept = default;
		high_resolution_timer& operator=(high_resolution_timer&&) noexcept = default;
		~high_resolution_timer();

		std::size_t cancel();
		std::size_t cancel_one();

		time_type expiry() const;
		std::size_t expires_at(const time_type& expiry_time);
		std::size_t expires_after(const duration_type& expiry_time);

		void wait();
		void wait(boost::system::error_code& ec);

		void async_wait(aux::function<void(boost::system::error_code const&)> handler);

	private:

		void fire(boost::system::error_code ec);

		time_type m_expiration_time;
		aux::function<void(boost::system::error_code const&)> m_handler;
		io_context* m_io_service;
		bool m_expired;
	};

	using waitable_timer = high_resolution_timer;

	namespace error = boost::asio::error;

	template <typename Protocol>
	struct socket_base
	{
		socket_base(io_context& ios) : m_io_service(ios) {}
		socket_base(socket_base&& s) = default;

		enum wait_type_t
		{
			wait_read, wait_write, wait_error
		};

		// io_control
		using reuse_address = boost::asio::socket_base::reuse_address;
		using executor_type = io_executor;
		executor_type get_executor();

		// socket options
		using send_buffer_size = boost::asio::socket_base::send_buffer_size;
		using receive_buffer_size = boost::asio::socket_base::receive_buffer_size;

		template <class Option>
		void set_option(Option const& opt, boost::system::error_code&)
		{
			Protocol const p = Protocol::v4();
			(void)p;
#ifdef IP_DONTFRAG
			if (opt.name(p) == IP_DONTFRAG)
				m_dont_fragment = *reinterpret_cast<int const*>(opt.data(p)) != 0;
#endif
#ifdef IP_DONTFRAGMENT
			if (opt.name(p) == IP_DONTFRAGMENT)
				m_dont_fragment = *reinterpret_cast<int const*>(opt.data(p)) != 0;
#endif
#ifdef IP_MTU_DISCOVER
			if (opt.name(p) == IP_MTU_DISCOVER)
				m_dont_fragment = *reinterpret_cast<int const*>(opt.data(p)) == IP_PMTUDISC_DO;
#endif
		}

		void set_option(receive_buffer_size const& op, boost::system::error_code&)
		{
			m_max_receive_queue_size = op.value();
		}

		void set_option(send_buffer_size const&, boost::system::error_code&)
		{
			// TODO: implement
		}

		void set_option(reuse_address const&, boost::system::error_code&)
		{
			// TODO: implement
		}

		typename Protocol::endpoint local_endpoint(boost::system::error_code& ec) const
		{
			if (!m_open)
			{
				ec = error::bad_descriptor;
				return typename Protocol::endpoint{};
			}

			return m_user_bound_to;
		}

		typename Protocol::endpoint local_endpoint() const
		{
			boost::system::error_code ec;
			auto const ret = local_endpoint(ec);
			if (ec) throw boost::system::system_error(ec);
			return ret;
		}

		typename Protocol::endpoint local_bound_to(boost::system::error_code& ec) const
		{
			if (!m_open)
			{
				ec = error::bad_descriptor;
				return typename Protocol::endpoint{};
			}

			return m_bound_to;
		}

		typename Protocol::endpoint local_bound_to() const
		{
			boost::system::error_code ec;
			auto const ret = local_bound_to(ec);
			if (ec) throw boost::system::system_error(ec);
			return ret;
		}

		template <class Option>
		void get_option(Option&, boost::system::error_code&) { }

		void get_option(receive_buffer_size& op, boost::system::error_code&)
		{
			op = m_max_receive_queue_size;
		}

		template <class IoControl>
		void io_control(IoControl const&, boost::system::error_code&) { }

		template <class IoControl>
		void io_control(IoControl const&) {}

		void non_blocking(bool b, boost::system::error_code&)
		{ m_non_blocking = b; }

		void non_blocking(bool b)
		{ m_non_blocking = b; }

		bool is_open() const
		{
			return m_open;
		}

		using message_flags = int;

		// internal interface

		route get_incoming_route();
		route get_outgoing_route();

	protected:

		io_context& m_io_service;

		typename Protocol::endpoint m_bound_to;

		// this is the interface the user requested to bind to (in order to
		// distinguish the concrete interface it was bound to and INADDR_ANY if
		// that was requested). We keep this separately to return it as the local
		// endpoint
		typename Protocol::endpoint m_user_bound_to;

		// this is an object implementing the sink interface, forwarding
		// packets to this socket. If this socket is destructed, this forwarder
		// is redirected to just drop packets. This is necessary since sinks
		// must be held by shared_ptr, and socket objects aren't.
		std::shared_ptr<aux::sink_forwarder> m_forwarder;

		// whether the socket is open or not
		bool m_open = false;

		// true if the socket is set to non-blocking mode
		bool m_non_blocking = false;

		// when true, the MTU limit is in effect
		bool m_dont_fragment = false;

		// the max size of the incoming queue. This is to emulate the send and
		// receive buffer. This should also depend on the bandwidth, to not
		// make the queue size not grow too long in time.
		int m_max_receive_queue_size = 64 * 1024;
	};

	namespace ip {

	using boost::asio::ip::address;
	using boost::asio::ip::address_v4;
	using boost::asio::ip::address_v6;

	using boost::asio::ip::make_address_v4;
	using boost::asio::ip::make_address_v6;
	using boost::asio::ip::make_address;

	using boost::asio::ip::make_network_v4;

	template<typename Protocol>
	struct basic_endpoint : boost::asio::ip::basic_endpoint<Protocol>
	{
		basic_endpoint(ip::address const& addr, unsigned short port)
			: boost::asio::ip::basic_endpoint<Protocol>(addr, port) {}
		basic_endpoint() : boost::asio::ip::basic_endpoint<Protocol>() {}
	};

	template <typename Protocol>
	struct basic_resolver_entry
	{
		using endpoint_type = typename Protocol::endpoint;
		using protocol_type = Protocol;

		basic_resolver_entry() {}
		basic_resolver_entry(
			endpoint_type const& ep
			, std::string const& host
			, std::string const& service)
			: m_endpoint(ep)
			, m_host_name(host)
			, m_service(service)
		{}

		endpoint_type endpoint() const { return m_endpoint; }
		std::string host_name() const { return m_host_name; }
		operator endpoint_type() const { return m_endpoint; }
		std::string service_name() const { return m_service; }

	private:
		endpoint_type m_endpoint;
		std::string m_host_name;
		std::string m_service;
	};

	template<typename Protocol>
	struct SIMULATOR_DECL basic_resolver
	{
		basic_resolver(io_context& ios);

		using protocol_type = Protocol;
		using results_type = std::vector<basic_resolver_entry<Protocol>, aux::mallocator<basic_resolver_entry<Protocol>>>;

		void cancel();

		void async_resolve(std::string hostname, char const* service
			, aux::function<void(boost::system::error_code const&,
				results_type)> handler);

		basic_resolver(basic_resolver&&) noexcept;
		basic_resolver& operator=(basic_resolver&&) noexcept;
		basic_resolver(basic_resolver const&) = delete;
		basic_resolver& operator=(basic_resolver const&) = delete;

		//TODO: add remaining members

	private:

		void on_lookup(boost::system::error_code const& ec);

		struct result_t
		{
			chrono::high_resolution_clock::time_point completion_time;
			boost::system::error_code err;
			results_type ips;
			aux::function<void(boost::system::error_code const&,
				results_type)> handler;

			result_t(
				chrono::high_resolution_clock::time_point ct
				, boost::system::error_code e
				, results_type ips_
				, aux::function<void(boost::system::error_code const&
					, results_type)> h)
				: completion_time(ct)
				, err(e)
				, ips(std::move(ips_))
				, handler(std::move(h))
			{}

			result_t(result_t&&) noexcept = default;
			result_t& operator=(result_t&&) = default;
			result_t(result_t const&) = delete;
			result_t& operator=(result_t const&) = delete;
		};

		io_context* m_ios;
		asio::high_resolution_timer m_timer;
		using queue_t = aux::noexcept_movable<std::vector<result_t>>;

		queue_t m_queue;
	};

	struct SIMULATOR_DECL udp
	{
		static udp v4() { return udp(AF_INET); }
		static udp v6() { return udp(AF_INET6); }

		using endpoint = basic_endpoint<udp>;

		struct SIMULATOR_DECL socket : socket_base<udp>, sink
		{
			using endpoint_type = ip::udp::endpoint;
			using protocol_type = ip::udp;
			using lowest_layer_type = socket;

			socket(io_context& ios);
			~socket() override;

			socket(socket const&) = delete;
			socket& operator=(socket const&) = delete;
			socket(socket&&);

			lowest_layer_type& lowest_layer() { return *this; }

			void bind(ip::udp::endpoint const& ep
				, boost::system::error_code& ec);
			void bind(ip::udp::endpoint const& ep);

			void close();
			void close(boost::system::error_code& ec);

			void cancel(boost::system::error_code& ec);
			void cancel();

			void open(udp protocol, boost::system::error_code& ec);
			void open(udp protocol);

			template<typename ConstBufferSequence>
			std::size_t send_to(ConstBufferSequence const& bufs
				, udp::endpoint const& destination
				, socket_base::message_flags flags
				, boost::system::error_code& ec)
			{
				std::vector<asio::const_buffer> b(buffer_sequence_begin(bufs)
					, buffer_sequence_end(bufs));
				abort_send_handlers();
				return send_to_impl(b, destination, flags, ec);
			}

			template<typename ConstBufferSequence>
			std::size_t send_to(ConstBufferSequence const& bufs
				, udp::endpoint const& destination)
			{
				std::vector<asio::const_buffer> b(buffer_sequence_begin(bufs), buffer_sequence_end(bufs));
				abort_send_handlers();
				boost::system::error_code ec;
				std::size_t ret = send_to_impl(b, destination, 0, ec);
				if (ec) throw boost::system::system_error(ec);
				return ret;
			}

			void async_wait(socket_base::wait_type_t w
				, aux::function<void(boost::system::error_code const&)> handler);

			template <class BufferSequence>
			void async_receive(BufferSequence const& bufs
				, aux::function<void(boost::system::error_code const&
					, std::size_t)> handler)
			{
				std::vector<asio::mutable_buffer> b(buffer_sequence_begin(bufs), buffer_sequence_end(bufs));
				abort_recv_handlers();

				async_receive_from_impl(b, nullptr, 0, std::move(handler));
			}

			template <class BufferSequence>
			void async_receive_from(BufferSequence const& bufs
				, udp::endpoint& sender
				, aux::function<void(boost::system::error_code const&
					, std::size_t)> handler)
			{
				std::vector<asio::mutable_buffer> b(buffer_sequence_begin(bufs), buffer_sequence_end(bufs));
				abort_recv_handlers();

				async_receive_from_impl(b, &sender, 0, std::move(handler));
			}

			template <class BufferSequence>
			void async_receive_from(BufferSequence const& bufs
				, udp::endpoint& sender
				, socket_base::message_flags flags
				, aux::function<void(boost::system::error_code const&
					, std::size_t)> handler)
			{
				std::vector<asio::mutable_buffer> b(buffer_sequence_begin(bufs), buffer_sequence_end(bufs));
				abort_recv_handlers();

				async_receive_from_impl(b, &sender, flags, std::move(handler));
			}
/*
			void async_read_from(null_buffers const&
				, aux::function<void(boost::system::error_code const&
					, std::size_t)> handler)
			{
				abort_recv_handlers();
				async_read_some_null_buffers_impl(std::move(handler));
			}
*/

			template <class BufferSequence>
			std::size_t receive_from(BufferSequence const& bufs
				, udp::endpoint& sender)
			{
				std::vector<asio::mutable_buffer> b(buffer_sequence_begin(bufs), buffer_sequence_end(bufs));
				assert(!b.empty());
				abort_recv_handlers();
				boost::system::error_code ec;
				std::size_t ret = receive_from_impl(b, &sender, 0, ec);
				if (ec) throw boost::system::system_error(ec);
				return ret;
			}

			template <class BufferSequence>
			std::size_t receive_from(BufferSequence const& bufs
				, udp::endpoint& sender
				, socket_base::message_flags)
			{
				std::vector<asio::mutable_buffer> b(buffer_sequence_begin(bufs), buffer_sequence_end(bufs));
				assert(!b.empty());
				abort_recv_handlers();
				boost::system::error_code ec;
				std::size_t ret = receive_from_impl(b, &sender, 0, ec);
				if (ec) throw boost::system::system_error(ec);
				return ret;
			}

			template <class BufferSequence>
			std::size_t receive_from(BufferSequence const& bufs
				, udp::endpoint& sender
				, socket_base::message_flags
				, boost::system::error_code& ec)
			{
				std::vector<asio::mutable_buffer> b(buffer_sequence_begin(bufs), buffer_sequence_end(bufs));
				assert(!b.empty());
				abort_recv_handlers();
				return receive_from_impl(b, &sender, 0, ec);
			}

			// TODO: support connect and remote_endpoint

			// internal interface

			// implements sink
			virtual void incoming_packet(aux::packet p) override final;
			virtual std::string label() const override final
			{ return m_bound_to.address().to_string(); }

			void async_receive_from_impl(std::vector<asio::mutable_buffer> const& bufs
				, udp::endpoint* sender
				, socket_base::message_flags flags
				, aux::function<void(boost::system::error_code const&
					, std::size_t)> handler);

			std::size_t receive_from_impl(
				std::vector<asio::mutable_buffer> const& bufs
				, udp::endpoint* sender
				, socket_base::message_flags flags
				, boost::system::error_code& ec);

			void async_wait_receive_impl(
				udp::endpoint* sender
				, aux::function<void(boost::system::error_code const&)> handler);

		private:

			void maybe_wakeup_reader();
			void abort_send_handlers();
			void abort_recv_handlers();

			std::size_t send_to_impl(std::vector<asio::const_buffer> const& b
				, udp::endpoint const& dst, message_flags flags
				, boost::system::error_code& ec);

			// this is the next time we'll have an opportunity to send another
			// outgoing packet. This is used to implement the bandwidth constraints
			// of channels. This may be in the past, in which case it's OK to send
			// a packet immediately.
			chrono::high_resolution_clock::time_point m_next_send;

			// while we're blocked in an async_write_some operation, this is the
			// handler that should be called once we're done sending
			aux::function<void(boost::system::error_code const&, std::size_t)>
				m_send_handler;
			aux::function<void(boost::system::error_code const&)>
				m_wait_send_handler;

			// if we have an outstanding read on this socket, this is set to the
			// handler.
			aux::function<void(boost::system::error_code const&, std::size_t)>
				m_recv_handler;
			aux::function<void(boost::system::error_code const&)>
				m_wait_recv_handler;

			// if we have an outstanding read operation, this is the buffer to
			// receive into
			std::vector<asio::mutable_buffer> m_recv_buffer;

			// if we have an outstanding receive operation, this may point to an
			// endpoint to fill in the senders IP in
			udp::endpoint* m_recv_sender;

			asio::high_resolution_timer m_recv_timer;
			asio::high_resolution_timer m_send_timer;

			// this is the incoming queue of packets for each socket
			std::list<aux::packet, aux::mallocator<aux::packet>> m_incoming_queue;

			bool m_recv_null_buffers;

			// the number of bytes in the incoming packet queue
			int m_queue_size;

			// our address family
			bool m_is_v4;
		};

		using resolver = basic_resolver<udp>;

		int family() const { return m_family; }

		friend bool operator==(udp const& lhs, udp const& rhs)
		{ return lhs.m_family == rhs.m_family; }

		friend bool operator!=(udp const& lhs, udp const& rhs)
		{ return lhs.m_family != rhs.m_family; }

	private:
		// Construct with a specific family.
		explicit udp(int protocol_family)
			: m_family(protocol_family)
		{}

		int m_family;

	}; // udp

	struct SIMULATOR_DECL tcp
	{
		// temporary fix until the resolvers are implemented using our endpoint
		tcp(boost::asio::ip::tcp p) : m_family(p.family()) {}

		static tcp v4() { return tcp(AF_INET); }
		static tcp v6() { return tcp(AF_INET6); }

		int family() const { return m_family; }

		using endpoint = basic_endpoint<tcp>;

		struct SIMULATOR_DECL socket : socket_base<tcp>, sink
		{
			using endpoint_type = ip::tcp::endpoint;
			using protocol_type = ip::tcp;
			using lowest_layer_type = socket;

			explicit socket(io_context& ios);
			socket(socket const&) = delete;
			socket& operator=(socket const&) = delete;
			socket(socket&&);
			socket& operator=(socket&&);

			~socket() override;

			void close();
			void close(boost::system::error_code& ec);
			void open(tcp protocol, boost::system::error_code& ec);
			void open(tcp protocol);
			void bind(ip::tcp::endpoint const& ep
				, boost::system::error_code& ec);
			void bind(ip::tcp::endpoint const& ep);
			tcp::endpoint remote_endpoint(boost::system::error_code& ec) const;
			tcp::endpoint remote_endpoint() const;

			lowest_layer_type& lowest_layer() { return *this; }

			void async_connect(tcp::endpoint const& target
				, aux::function<void(boost::system::error_code const&)> h);

			template <class ConstBufferSequence>
			void async_write_some(ConstBufferSequence const& bufs
				, aux::function<void(boost::system::error_code const&
					, std::size_t)> handler)
			{
				std::vector<asio::const_buffer> b(buffer_sequence_begin(bufs), buffer_sequence_end(bufs));
				abort_send_handlers();
				async_write_some_impl(b, std::move(handler));
			}

			void async_wait(socket_base::wait_type_t const w
				, aux::function<void(boost::system::error_code const&)> handler)
			{
				if (w == socket_base::wait_type_t::wait_write)
				{
					abort_send_handlers();
					assert(false && "not supported yet");
				}
				else if (w == socket_base::wait_type_t::wait_read)
				{
					abort_recv_handlers();
					async_wait_read_impl(std::move(handler));
				}
			}

			template <class BufferSequence>
			std::size_t read_some(BufferSequence const& bufs
				, boost::system::error_code& ec)
			{
				assert(m_non_blocking && "blocking operations not supported");
				std::vector<asio::mutable_buffer> b(buffer_sequence_begin(bufs), buffer_sequence_end(bufs));
				return read_some_impl(b, ec);
			}

			template <class ConstBufferSequence>
			std::size_t write_some(ConstBufferSequence const& bufs
				, boost::system::error_code& ec)
			{
				assert(m_non_blocking && "blocking operations not supported");
				std::vector<asio::const_buffer> b(buffer_sequence_begin(bufs), buffer_sequence_end(bufs));
				return write_some_impl(b, ec);
			}

			template <class BufferSequence>
			void async_read_some(BufferSequence const& bufs
				, aux::function<void(boost::system::error_code const&
					, std::size_t)> handler)
			{
				std::vector<asio::mutable_buffer> b(buffer_sequence_begin(bufs), buffer_sequence_end(bufs));
				abort_recv_handlers();

				async_read_some_impl(b, std::move(handler));
			}

			std::size_t available(boost::system::error_code & ec) const;
			std::size_t available() const;

			void cancel(boost::system::error_code& ec);
			void cancel();

			using socket_base::set_option;
			using socket_base::get_option;
			using socket_base::io_control;

			// private interface

			// implements sink
			virtual void incoming_packet(aux::packet p) override;
			virtual std::string label() const override final
			{ return m_bound_to.address().to_string(); }

			void internal_connect(tcp::endpoint const& bind_ip
				, std::shared_ptr<aux::channel> const& c
				, boost::system::error_code& ec);

			void abort_send_handlers();
			void abort_recv_handlers();

			virtual bool internal_is_listening();
		protected:

			void maybe_wakeup_reader();
			void maybe_wakeup_writer();

			void async_write_some_impl(std::vector<asio::const_buffer> const& bufs
				, aux::function<void(boost::system::error_code const&, std::size_t)> handler);
			void async_read_some_impl(std::vector<asio::mutable_buffer> const& bufs
				, aux::function<void(boost::system::error_code const&, std::size_t)> handler);
			void async_wait_read_impl(
				aux::function<void(boost::system::error_code const&)> handler);
			std::size_t write_some_impl(std::vector<asio::const_buffer> const& bufs
				, boost::system::error_code& ec);
			std::size_t read_some_impl(std::vector<asio::mutable_buffer> const& bufs
				, boost::system::error_code& ec);

			void send_packet(aux::packet p);

			// called when a packet is dropped
			void packet_dropped(aux::packet p);

			aux::function<void(boost::system::error_code const&)> m_connect_handler;

			asio::high_resolution_timer m_connect_timer;

			// the tcp "packet size" (segment size)
			// TODO: name this constant!
			int m_mss = 1475;

			// while we're blocked in an async_write_some operation, this is the
			// handler that should be called once we're done sending
			aux::function<void(boost::system::error_code const&, std::size_t)> m_send_handler;
			aux::function<void(boost::system::error_code const&)> m_wait_send_handler;

			std::vector<asio::const_buffer> m_send_buffer;

			// this is the incoming queue of packets for each socket
			std::list<aux::packet, aux::mallocator<aux::packet>> m_incoming_queue;

			// the number of bytes in the incoming packet queue
			int m_queue_size = 0;

			// if we have an outstanding read on this socket, this is set to the
			// handler.
			aux::function<void(boost::system::error_code const&, std::size_t)> m_recv_handler;
			aux::function<void(boost::system::error_code const&)> m_wait_recv_handler;

			// if we have an outstanding buffer to receive into, these are them
			std::vector<asio::mutable_buffer> m_recv_buffer;

			asio::high_resolution_timer m_recv_timer;

			// our address family
			bool m_is_v4 = true;

			// true if the currently outstanding read operation is for null_buffers
			bool m_recv_null_buffers = false;

			// true if the currenly outstanding write operation is for null_buffers
			bool m_send_null_buffers = false;

			// if this socket is connected to another endpoint, this object is
			// shared between both sockets and contain information and state about
			// the channel.
			std::shared_ptr<aux::channel> m_channel;

			std::uint64_t m_next_outgoing_seq = 0;
			std::uint64_t m_next_incoming_seq = 0;

			// the sequence number of the last dropped packet. We should only cut
			// the cwnd in half once per round-trip. If a whole window is lost, we
			// need to only halve it once
			std::uint64_t m_last_drop_seq = 0;

			// the current congestion window size (in bytes)
			int m_cwnd = m_mss * 2;

			// the number of bytes that have been sent but not ACKed yet
			int m_bytes_in_flight = 0;

			// reorder buffer for when packets are dropped
			std::map<std::uint64_t, aux::packet, std::less<std::uint64_t>, aux::mallocator<std::pair<std::uint64_t const, aux::packet>>> m_reorder_buffer;

			// the sizes of packets given their sequence number
			std::unordered_map<std::uint64_t, int, std::hash<std::uint64_t>, std::equal_to<std::uint64_t>, aux::mallocator<std::pair<std::uint64_t const, int>>> m_outstanding_packet_sizes;

			// packets to re-send (because they were dropped)
			std::list<aux::packet, aux::mallocator<aux::packet>> m_outgoing_packets;
		};

		struct SIMULATOR_DECL acceptor : socket
		{
			explicit acceptor(io_context& ios);
			acceptor(acceptor&&);
			~acceptor() override;

			void cancel(boost::system::error_code& ec);
			void cancel();

			void listen(int qs = -1);
			void listen(int qs, boost::system::error_code& ec);

			void async_accept(ip::tcp::socket& peer
				, aux::function<void(boost::system::error_code const&)> h);
			void async_accept(ip::tcp::socket& peer
				, ip::tcp::endpoint& peer_endpoint
				, aux::function<void(boost::system::error_code const&)> h);
			void async_accept(aux::function<void(boost::system::error_code const&, ip::tcp::socket peer)> h);

			void close(boost::system::error_code& ec);
			void close();

			// private interface

			// implements sink
			virtual void incoming_packet(aux::packet p) override final;
			virtual bool internal_is_listening() override final;

		private:
			// check the incoming connection queue to see if any connection in
			// there is ready to be accepted and delivered to the user
			void check_accept_queue();
			void do_check_accept_queue(boost::system::error_code const& ec);

			aux::function<void(boost::system::error_code const&)> m_accept_handler;
			aux::function<void(boost::system::error_code const&, ip::tcp::socket)> m_accept_handler2;

			// the number of half-open incoming connections this listen socket can
			// hold. If this is -1, this socket is not yet listening and incoming
			// connection attempts should be rejected.
			int m_queue_size_limit;

			// these are incoming connection attempts. Both half-open and
			// completely connected. When accepting a connection, this queue is
			// checked first before waiting for a connection attempt.
			using incoming_conns_t = std::vector<std::shared_ptr<aux::channel>, aux::mallocator<std::shared_ptr<aux::channel>>>;
			incoming_conns_t m_incoming_conns;

			// for new-style accept, allocate socket in here just to fail early
			boost::optional<ip::tcp::socket> m_new_socket;

			// the socket to accept a connection into
			tcp::socket* m_accept_into;

			// the endpoint to write the remote endpoint into when accepting
			tcp::endpoint* m_remote_endpoint;

			// non copyable
			acceptor(acceptor const&);
			acceptor& operator=(acceptor const&);
		};

		using resolver = basic_resolver<tcp>;

		friend bool operator==(tcp const& lhs, tcp const& rhs)
		{ return lhs.m_family == rhs.m_family; }

		friend bool operator!=(tcp const& lhs, tcp const& rhs)
		{ return lhs.m_family != rhs.m_family; }

	private:
		// Construct with a specific family.
		explicit tcp(int protocol_family)
			: m_family(protocol_family)
		{}

		int m_family;
	};

	extern template struct basic_resolver<udp>;
	extern template struct basic_resolver<tcp>;

	} // ip

	} // asio

	struct SIMULATOR_DECL simulation
	{
		// it calls fire() when a timer fires
		friend struct high_resolution_timer;

		simulation(configuration& config);
		~simulation();

		std::size_t run();

		std::size_t poll(boost::system::error_code& ec);
		std::size_t poll();

		std::size_t poll_one(boost::system::error_code& ec);
		std::size_t poll_one();

		void stop();
		bool stopped() const;
		void restart();
		// private interface

		void add_timer(asio::high_resolution_timer* t);
		void remove_timer(asio::high_resolution_timer* t);

		boost::asio::io_context& get_internal_service()
		{ return m_service; }

		asio::io_context& get_io_context()
		{ return *m_internal_ios; }

		asio::ip::tcp::endpoint bind_socket(asio::ip::tcp::socket* socket
			, asio::ip::tcp::endpoint ep
			, boost::system::error_code& ec);
		void unbind_socket(asio::ip::tcp::socket* socket
			, asio::ip::tcp::endpoint const& ep);
		void rebind_socket(asio::ip::tcp::socket* s, asio::ip::tcp::endpoint ep);

		asio::ip::udp::endpoint bind_udp_socket(asio::ip::udp::socket* socket
			, asio::ip::udp::endpoint ep
			, boost::system::error_code& ec);
		void unbind_udp_socket(asio::ip::udp::socket* socket
			, asio::ip::udp::endpoint const& ep);
		void rebind_udp_socket(asio::ip::udp::socket* socket, asio::ip::udp::endpoint ep);

		std::shared_ptr<aux::channel> internal_connect(asio::ip::tcp::socket* s
			, asio::ip::tcp::endpoint const& target, boost::system::error_code& ec);

		route find_udp_socket(
			asio::ip::udp::socket const& socket
			, asio::ip::udp::endpoint const& ep);

		configuration& config() const { return m_config; }

		void add_io_service(asio::io_context* ios);
		void remove_io_service(asio::io_context* ios);
		std::vector<asio::io_context*> get_all_io_services() const;

		aux::pcap* get_pcap() const { return m_pcap.get(); }
		void log_pcap(char const* filename);

	private:
		struct timer_compare
		{
			bool operator()(asio::high_resolution_timer const* lhs
				, asio::high_resolution_timer const* rhs) const
			{ return lhs->expiry() < rhs->expiry(); }
		};

		configuration& m_config;

		std::unique_ptr<aux::pcap> m_pcap;

		// all non-expired timers
		std::mutex m_timer_queue_mutex;
		using timer_queue_t = std::multiset<asio::high_resolution_timer*, timer_compare, aux::mallocator<asio::high_resolution_timer*>>;
		timer_queue_t m_timer_queue;

		// these are the io services that represent nodes on the network
		std::unordered_set<asio::io_context*, std::hash<asio::io_context*>, std::equal_to<asio::io_context*>, aux::mallocator<asio::io_context*>> m_nodes;

		using listen_sockets_t = std::map<asio::ip::tcp::endpoint, asio::ip::tcp::socket*>;
		using listen_socket_iter_t = listen_sockets_t::iterator;
		listen_sockets_t m_listen_sockets;

		using udp_sockets_t = std::map<asio::ip::udp::endpoint, asio::ip::udp::socket*>;
		using udp_socket_iter_t = udp_sockets_t::iterator;
		udp_sockets_t m_udp_sockets;

		// used for internal timers. this is a pimpl sine our io_context is
		// incomplete at this point
		std::unique_ptr<asio::io_context> m_internal_ios;

		// the next port to use for an outgoing connection, where the port is not
		// specified. We want this to be as unique as possible, to distinguish the
		// TCP streams.
		std::uint16_t m_next_bind_port = 2000;

		bool m_stopped = false;

		// underlying message queue
		boost::asio::io_context m_service;
	};

	namespace asio {

	using boost::asio::async_write;
	using boost::asio::async_read;

	// boost.asio compatible io_context class that simulates the network
	// and time.
	struct SIMULATOR_DECL io_context : boost::asio::execution_context
	{
		io_context(sim::simulation& sim);
		io_context(sim::simulation& sim, ip::address const& ip);
		io_context(sim::simulation& sim, std::vector<ip::address> const& ips);
		io_context(std::size_t threads_hint = 0);
		~io_context();

		// not copyable and non movable (it's not movable because we currently
		// keep pointers to the io_context instances in the simulator object)
		io_context(io_context const&) = delete;
		io_context(io_context&&) = delete;
		io_context& operator=(io_context const&) = delete;
		io_context& operator=(io_context&&) = delete;

		std::size_t run(boost::system::error_code& ec);
		std::size_t run();

		std::size_t poll(boost::system::error_code& ec);
		std::size_t poll();

		std::size_t poll_one(boost::system::error_code& ec);
		std::size_t poll_one();

		void stop();
		bool stopped() const;
		void restart();

		template <typename Handler, typename Allocator>
		void dispatch_impl(Handler handler, Allocator const& a)
		{ get_internal_service().get_executor().dispatch(std::move(handler), a); }

		template <typename Handler, typename Allocator>
		void post_impl(Handler handler, Allocator const& a)
		{ get_internal_service().get_executor().post(std::move(handler), a); }

		template <typename Handler, typename Allocator>
		void defer_impl(Handler handler, Allocator const& a)
		{ get_internal_service().get_executor().defer(std::move(handler), a); }

		// internal interface
		boost::asio::io_context& get_internal_service();

		void add_timer(high_resolution_timer* t);
		void remove_timer(high_resolution_timer* t);

		ip::tcp::endpoint bind_socket(ip::tcp::socket* socket, ip::tcp::endpoint ep
			, boost::system::error_code& ec);
		void unbind_socket(ip::tcp::socket* socket
			, ip::tcp::endpoint const& ep);
		void rebind_socket(asio::ip::tcp::socket* s, asio::ip::tcp::endpoint ep);

		ip::udp::endpoint bind_udp_socket(ip::udp::socket* socket, ip::udp::endpoint ep
			, boost::system::error_code& ec);
		void unbind_udp_socket(ip::udp::socket* socket
			, ip::udp::endpoint const& ep);
		void rebind_udp_socket(asio::ip::udp::socket* socket, asio::ip::udp::endpoint ep);

		std::shared_ptr<aux::channel> internal_connect(ip::tcp::socket* s
			, ip::tcp::endpoint const& target, boost::system::error_code& ec);

		route find_udp_socket(asio::ip::udp::socket const& socket
			, ip::udp::endpoint const& ep);

		route const& get_outgoing_route(ip::address ip) const
		{ return m_outgoing_route.find(ip)->second; }

		route const& get_incoming_route(ip::address ip) const
		{ return m_incoming_route.find(ip)->second; }

		int get_path_mtu(const asio::ip::address& source, const asio::ip::address& dest) const;
		std::vector<ip::address> const& get_ips() const { return m_ips; }

		sim::simulation& sim() { return m_sim; }

		using executor_type = io_executor;
		executor_type get_executor() { return executor_type(*this); }

	private:

		sim::simulation& m_sim;
		std::vector<ip::address> m_ips;

		// these are determined by the configuration. They may include NATs and
		// DSL modems (queues)
		std::map<ip::address, route> m_outgoing_route;
		std::map<ip::address, route> m_incoming_route;

		bool m_stopped = false;
	};

	template <typename Handler, typename Allocator>
	void io_executor::dispatch(Handler handler, Allocator const& a) const
	{ m_ctx->dispatch_impl(std::move(handler), a); }

	template <typename Handler, typename Allocator>
	void io_executor::post(Handler handler, Allocator const& a) const
	{ m_ctx->post_impl(std::move(handler), a); }

	template <typename Handler, typename Allocator>
	void io_executor::defer(Handler handler, Allocator const& a) const
	{ m_ctx->defer_impl(std::move(handler), a); }

	template <typename Protocol>
	io_executor
	socket_base<Protocol>::get_executor()
	{ return io_executor(m_io_service); }

	template <typename Protocol>
	route socket_base<Protocol>::get_incoming_route()
	{
		route ret = m_io_service.get_incoming_route(m_bound_to.address());
		assert(m_forwarder);
		ret.append(std::static_pointer_cast<sim::sink>(m_forwarder));
		return ret;
	}

	template <typename Protocol>
	route socket_base<Protocol>::get_outgoing_route()
	{
		return route(m_io_service.get_outgoing_route(m_bound_to.address()));
	}

	} // asio

	// user supplied configuration of the network to simulate
	struct SIMULATOR_DECL configuration
	{
		virtual ~configuration() {}

		// build the network
		virtual void build(simulation& sim) = 0;

		// return the hops on the network packets from src to dst need to traverse
		virtual route channel_route(asio::ip::address src
			, asio::ip::address dst) = 0;

		// return the hops an incoming packet to ep need to traverse before
		// reaching the socket (for instance a NAT)
		virtual route incoming_route(asio::ip::address ip) = 0;

		// return the hops an outgoing packet from ep need to traverse before
		// reaching the network (for instance a DSL modem)
		virtual route outgoing_route(asio::ip::address ip) = 0;

		// return the path MTU between the two IP addresses
		// For TCP sockets, this will be called once when the connection is
		// established. For UDP sockets it's called for every burst of packets
		// that are sent
		virtual int path_mtu(asio::ip::address ip1, asio::ip::address ip2) = 0;

		// called for every hostname lookup made by the client. ``reqyestor`` is
		// the node performing the lookup, ``hostname`` is the name being looked
		// up. Resolve the name into addresses and fill in ``result`` or set
		// ``ec`` if the hostname is not found or some other error occurs. The
		// return value is the latency of the lookup. The client's callback won't
		// be called until after waiting this long.
		virtual chrono::high_resolution_clock::duration hostname_lookup(
			asio::ip::address const& requestor
			, std::string hostname
			, std::vector<asio::ip::address>& result
			, boost::system::error_code& ec) = 0;

		virtual void clear() = 0;
	};

	struct SIMULATOR_DECL default_config : configuration
	{
		default_config() : m_sim(nullptr) {}

		void build(simulation& sim) override;
		route channel_route(asio::ip::address src, asio::ip::address dst) override;
		route incoming_route(asio::ip::address ip) override;
		route outgoing_route(asio::ip::address ip) override;
		int path_mtu(asio::ip::address ip1, asio::ip::address ip2) override;
		chrono::high_resolution_clock::duration hostname_lookup(
			asio::ip::address const& requestor
			, std::string hostname
			, std::vector<asio::ip::address>& result
			, boost::system::error_code& ec) override;

		void clear() override;
	protected:
		std::shared_ptr<queue> m_network;
		std::map<asio::ip::address, std::shared_ptr<queue>> m_incoming;
		std::map<asio::ip::address, std::shared_ptr<queue>> m_outgoing;
		simulation* m_sim;
	};

	namespace aux
	{
		/* the channel can be in the following states:
			1. handshake-1 - the initiating socket has sent SYN
			2. handshake-2 - the accepting connection has sent SYN+ACK
			3. handshake-3 - the initiating connection has received the SYN+ACK and
			                 considers the connection open, but the 3rd handshake
			                 message is still in flight.
			4. connected   - the accepting side has received the 3rd handshake
			                 packet and considers it open

			Whenever a connection attempt is made to a listening socket, as long as
			there is still space in the incoming socket queue, the accepting side
			will always respond immediately and complete the handshake, then wait
			until the user calls async_accept (which in this case would complete
			immediately).
		*/
		struct SIMULATOR_DECL channel
		{
			channel() {}
			// index 0 is the incoming route to the socket that initiated the connection.
			// index 1 may be empty while the connection is half-open
			route hops[2];

			// the actual endpoint of each end of the channel
			asio::ip::tcp::endpoint ep[2];

			// observable endpoint of each side of the channel. This is not how you
			// see yourself, just the other end
			asio::ip::tcp::endpoint visible_ep[2];

			// the number of bytes sent from respective direction
			// this is used to simulate the TCP sequence number, so it deliberately
			// is meant to wrap at 32 bits
			std::uint32_t bytes_sent[2];

			int remote_idx(asio::ip::tcp::endpoint const& self) const;
			int self_idx(asio::ip::tcp::endpoint const& self) const;
		};

	} // aux

	void SIMULATOR_DECL dump_network_graph(simulation const& s, const std::string& filename);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // SIMULATOR_HPP_INCLUDED

