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
#include "simulator/handler_allocator.hpp"
#include <functional>

typedef sim::chrono::high_resolution_clock::time_point time_point;
typedef sim::chrono::high_resolution_clock::duration duration;

using namespace std::placeholders;

namespace sim {
namespace asio {
namespace ip {

	template<typename Protocol>
	basic_resolver<Protocol>::basic_resolver(io_context& ios)
		: m_ios(&ios)
		, m_timer(ios)
	{}

	template<typename Protocol>
	basic_resolver<Protocol>::basic_resolver(basic_resolver<Protocol>&&) noexcept = default;

	template<typename Protocol>
	basic_resolver<Protocol>& basic_resolver<Protocol>::operator=(basic_resolver<Protocol>&&) noexcept = default;

	template<typename Protocol>
	void basic_resolver<Protocol>::async_resolve(std::string hostname, char const* service
		, aux::function<void(boost::system::error_code const&, results_type)> handler)
	{
		std::vector<asio::ip::address> result;
		boost::system::error_code ec;

		const chrono::high_resolution_clock::time_point start_time =
			m_queue.empty() ? chrono::high_resolution_clock::now() :
			m_queue.front().completion_time;

		assert(!m_ios->get_ips().empty() && "internal io service objects can only "
			"be used for timers");

		// if the hostname is an IP address, resolve it immediately
		asio::ip::address addr = make_address_v4(hostname, ec);
		if (ec) addr = make_address_v6(hostname, ec);
		if (!ec)
		{
			const chrono::high_resolution_clock::time_point t = chrono::high_resolution_clock::now()
				+ chrono::microseconds(1);
			results_type ips;
			int const port = atoi(service);
			assert(port >= 0 && port <= 0xffff);
			ips.emplace_back(
				typename Protocol::endpoint(addr, static_cast<unsigned short>(port))
				, hostname, service);
			result_t res{t, ec, std::move(ips), std::move(handler) };
			m_queue.insert(m_queue.begin(), std::move(res));
			m_timer.expires_at(m_queue.front().completion_time);
			m_timer.async_wait(aux::make_malloc(std::bind(&basic_resolver::on_lookup, this, _1)));
			return;
		}
		ec.clear();

		const chrono::high_resolution_clock::time_point completion_time =
			start_time
			+ m_ios->sim().config().hostname_lookup(m_ios->get_ips().front(), hostname
				, result, ec);

		results_type ips;

		int const port = atoi(service);
		assert(port >= 0 && port <= 0xffff);

		for (auto const& ip : result)
		{
			ips.emplace_back(
				typename Protocol::endpoint(ip, static_cast<unsigned short>(port))
				, hostname, service);
		}

		result_t res{ completion_time, ec, std::move(ips), std::move(handler)};
		m_queue.emplace_back(std::move(res));

		m_timer.expires_at(m_queue.front().completion_time);
		m_timer.async_wait(aux::make_malloc(std::bind(&basic_resolver::on_lookup, this, _1)));
	}

	template<typename Protocol>
	void basic_resolver<Protocol>::on_lookup(boost::system::error_code const& ec)
	{
		if (ec == asio::error::operation_aborted) return;

		if (m_queue.empty()) return;

		typename queue_t::value_type v = std::move(m_queue.front());
		m_queue.erase(m_queue.begin());

		// once the handler is called, it's possible the last reference keeping
		// this object (basic_resolver) alive is released and we're deleted. Make
		// sure to not touch any members after the handler in that case.
		bool const empty = m_queue.empty();
		v.handler(v.err, std::move(v.ips));
		if (empty) return;

		m_timer.expires_at(m_queue.front().completion_time);
		m_timer.async_wait(aux::make_malloc(std::bind(&basic_resolver::on_lookup, this, _1)));
	}

	template<typename Protocol>
	void basic_resolver<Protocol>::cancel()
	{
		queue_t q;
		m_queue.swap(q);
		for (auto& r : q)
		{
			r.err = asio::error::operation_aborted;
			post(m_timer.get_executor(), aux::make_malloc(std::bind(std::move(r.handler)
				, r.err, std::move(r.ips))));
		}
	}

	// explicitly instantiate the functions
	template struct basic_resolver<udp>;
	template struct basic_resolver<tcp>;
}
}
}

