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

#include "simulator/queue.hpp"
#include "simulator/handler_allocator.hpp"
#include <functional>
#include <cstdio> // for printf

typedef sim::chrono::high_resolution_clock::time_point time_point;
typedef sim::chrono::high_resolution_clock::duration duration;

namespace sim
{
	using namespace aux;

	queue::queue(asio::io_context& ios
		, int bandwidth
		, chrono::high_resolution_clock::duration propagation_delay
		, int max_queue_size
		, std::string name)
		: m_max_queue_size(max_queue_size)
		, m_forwarding_latency(propagation_delay)
		, m_bandwidth(bandwidth)
		, m_queue_size(0)
		, m_node_name(name)
		, m_forward_timer(ios)
		, m_last_forward(chrono::high_resolution_clock::now())
	{}

	std::string queue::label() const
	{
		char ret[400];
		int p = std::snprintf(ret, sizeof(ret), "%s\n", m_node_name.c_str());

		if (m_bandwidth != 0)
		{
			p += std::snprintf(ret + p, sizeof(ret) - p, "rate: %d kB/s\n"
				, m_bandwidth / 1000);
		}

		if (m_queue_size != 0)
		{
			p += std::snprintf(ret + p, sizeof(ret) - p, "queue: %d kB\n"
				, m_queue_size / 1000);
		}

		if (m_forwarding_latency.count() != 0)
		{
			p += std::snprintf(ret + p, sizeof(ret) - p, "latency: %d ms\n"
				, int(chrono::duration_cast<chrono::milliseconds>(m_forwarding_latency).count()));
		}

		return ret;
	}

	void queue::incoming_packet(aux::packet p)
	{
		const int packet_size = int(p.buffer.size() + p.overhead);

		// tail-drop
		if (p.ok_to_drop()
			&& m_max_queue_size > 0
			&& m_queue_size + packet_size > m_max_queue_size)
		{
			// if any hop on the network drops a packet, it has to return it to the
			// sender.
			auto drop_fun = std::move(p.drop_fun);
			if (drop_fun) drop_fun(std::move(p));
			return;
		}

		time_point const now = chrono::high_resolution_clock::now();

		m_queue.emplace_back(now + m_forwarding_latency, std::move(p));
		m_queue_size += packet_size;
		if (m_queue.size() > 1) return;

		begin_send_next_packet();
	}

	void queue::begin_send_next_packet()
	{
		time_point now = chrono::high_resolution_clock::now();

		if (m_queue.front().ts > now)
		{
			m_forward_timer.expires_at(m_queue.front().ts);
			m_forward_timer.async_wait(make_malloc(std::bind(&queue::begin_send_next_packet
				, this)));
			return;
		}

		m_last_forward = now;
		if (m_bandwidth == 0)
		{
			post(m_forward_timer.get_executor(), make_malloc(std::bind(&queue::next_packet_sent
				, this)));
			return;
		}
		const double nanoseconds_per_byte = 1000000000.0
			/ double(m_bandwidth);

		aux::packet const& p = m_queue.front().pkt;
		const int packet_size = int(p.buffer.size() + p.overhead);

		m_last_forward += chrono::duration_cast<duration>(chrono::nanoseconds(
			boost::int64_t(nanoseconds_per_byte * packet_size)));

		m_forward_timer.expires_at(m_last_forward);
		m_forward_timer.async_wait(make_malloc(std::bind(&queue::next_packet_sent
			, this)));
	}

	void queue::next_packet_sent()
	{
		aux::packet p = std::move(m_queue.front().pkt);
		m_queue.erase(m_queue.begin());
		const int packet_size = int(p.buffer.size() + p.overhead);
		m_queue_size -= packet_size;

		forward_packet(std::move(p));

		if (m_queue.size())
			begin_send_next_packet();
	}
}

