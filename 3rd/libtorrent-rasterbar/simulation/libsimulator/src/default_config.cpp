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
#include "simulator/queue.hpp"
#include <memory>

typedef sim::chrono::high_resolution_clock::time_point time_point;
typedef sim::chrono::high_resolution_clock::duration duration;
using sim::asio::ip::address_v4;
using sim::asio::ip::address_v6;
using sim::asio::ip::address;
using sim::chrono::milliseconds;
using sim::chrono::duration_cast;

namespace sim {

	void default_config::build(simulation& sim)
	{
		// 0 bandwidth and 0 queue means infinite. The network itself only adds
		// 50 ms latency
		m_network = std::make_shared<queue>(std::ref(sim.get_io_context())
			, 0, duration_cast<duration>(milliseconds(30)), 0, "network");
		m_sim = &sim;
	}

	void default_config::clear()
	{
		m_network.reset();
		m_outgoing.clear();
		m_incoming.clear();
	}

	route default_config::channel_route(
		asio::ip::address /* src */
		, asio::ip::address /* dst */)
	{
		return route().append(m_network);
	}

	route default_config::incoming_route(asio::ip::address ip)
	{
		// incoming download rate is 800kB/s with a 200 kB queue
		// and 1 ms forwarding delay
		auto it = m_incoming.find(ip);
		if (it != m_incoming.end()) return route().append(it->second);
		it = m_incoming.insert(it, std::make_pair(ip, std::make_shared<queue>(
			std::ref(m_sim->get_io_context()), 800 * 1000
			, duration_cast<duration>(milliseconds(1)), 200 * 1000, "DSL modem in")));
		return route().append(it->second);
	}

	int default_config::path_mtu(
		asio::ip::address /* ip1 */
		, asio::ip::address /* ip2 */)
	{
		return 1475;
	}

	// return the hops an outgoing packet from ep need to traverse before
	// reaching the network (for instance a DSL modem)
	route default_config::outgoing_route(asio::ip::address ip)
	{
		// outgoing upload rate is 200kB/s with a 200 kB queue
		// and 1 ms forwarding delay
		auto it = m_outgoing.find(ip);
		if (it != m_outgoing.end()) return route().append(it->second);
		it = m_outgoing.insert(it, std::make_pair(ip, std::make_shared<queue>(
			std::ref(m_sim->get_io_context()), 200 * 1000
			, duration_cast<duration>(milliseconds(1)), 200 * 1000, "DSL modem out")));
		return route().append(it->second);
	}

	duration default_config::hostname_lookup(
		asio::ip::address const& /* requestor */
		, std::string hostname
		, std::vector<asio::ip::address>& result
		, boost::system::error_code& ec)
	{
		if (hostname == "localhost")
		{
			result = { asio::ip::make_address_v6("::1")
				, asio::ip::make_address_v4("127.0.0.1") };
			return duration_cast<duration>(chrono::microseconds(1));
		}

		ec = boost::system::error_code(asio::error::host_not_found);
		return duration_cast<duration>(chrono::milliseconds(100));
	}
}

