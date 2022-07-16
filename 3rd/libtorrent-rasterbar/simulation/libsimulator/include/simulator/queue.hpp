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

#ifndef QUEUE_HPP_INCLUDED
#define QUEUE_HPP_INCLUDED

#include "simulator/simulator.hpp"
#include "simulator/packet.hpp"
#include "simulator/mallocator.hpp"

#ifdef _MSC_VER
#pragma warning(push)
// warning C4251: X: class Y needs to have dll-interface to be used by clients of struct
#pragma warning( disable : 4251)
#endif

namespace sim {

	struct timed_packet
	{
		timed_packet(chrono::high_resolution_clock::time_point t, aux::packet p)
			: ts(t), pkt(std::move(p))
		{}
		timed_packet(timed_packet&&) = default;
		timed_packet& operator=(timed_packet&&) = default;
		timed_packet(timed_packet const&) = delete;
		timed_packet& operator=(timed_packet const&) = delete;
		chrono::high_resolution_clock::time_point ts;
		aux::packet pkt;
	};

	// this is a queue. It can be configured to contrain
	struct SIMULATOR_DECL queue : sink
	{
		queue(asio::io_context& ios, int bandwidth
			, chrono::high_resolution_clock::duration propagation_delay
			, int max_queue_size, std::string name = "queue");

		virtual void incoming_packet(aux::packet p) override final;

		virtual std::string label() const override final;

		queue(queue const&) = delete;
		queue& operator=(queue const&) = delete;

		queue(queue&&) = default;
		queue& operator=(queue&&) = delete;

	private:

		void begin_send_next_packet();
		void next_packet_sent();

		// the queue can't hold more than this number of bytes. Once it's full,
		// any new packets arriving will be dropped (tail drop)
		int m_max_queue_size;

		// the amount of time it takes to forward a packet. Every packet is
		// delayed by at least this much before being forwarded
		chrono::high_resolution_clock::duration m_forwarding_latency;

		// the number of bytes per second that can be sent. This includes the
		// packet overhead
		int m_bandwidth;

		// the number of bytes currently in the packet queue
		int m_queue_size;

		std::string m_node_name;

		// this is the queue of packets and the time each packet was enqueued
		std::deque<timed_packet, aux::mallocator<timed_packet>> m_queue;
		asio::high_resolution_timer m_forward_timer;

		chrono::high_resolution_clock::time_point m_last_forward;
	};

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

