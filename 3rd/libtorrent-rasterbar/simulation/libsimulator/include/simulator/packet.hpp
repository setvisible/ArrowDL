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

#ifndef PACKET_HPP_INCLUDED
#define PACKET_HPP_INCLUDED

#include "simulator/config.hpp"
#include "simulator/simulator.hpp" // for route, endpoint

namespace sim { namespace aux {

	struct channel;

	struct packet
	{
		packet() = default;

		// this is move-only
		packet(packet const&) = delete;
		packet& operator=(packet const&) = delete;
		packet(packet&&) = default;
		packet& operator=(packet&&) = default;

		// to keep things simple, don't drop ACKs or errors
		bool ok_to_drop() const
		{
			return type != type_t::syn_ack
				&& type != type_t::ack
				&& type != type_t::error;
		}

		enum class type_t
		{
			uninitialized, // invalid type (used for debugging)
			syn, // TCP connect
			syn_ack, // TCP connection accepted
			ack, // the seq_nr is interpreted as "we received this"
			error, // the error_code (ec) is set
			payload // the buffer is filled
		};

		type_t type = type_t::uninitialized;

		boost::system::error_code ec;

		// actual payload
		std::vector<boost::uint8_t> buffer;

		// used for UDP packets
		asio::ip::udp::endpoint from;

		// the number of bytes of overhead for this packet. The total packet
		// size is the number of bytes in the buffer + this number
		int overhead = 20;

		// each hop in the route will pop itself off and forward the packet to
		// the next hop
		route hops;

		// for SYN packets, this is set to the channel we're trying to
		// establish
		std::shared_ptr<aux::channel> channel;

		// sequence number of this packet (used for debugging)
		std::uint64_t seq_nr = 0;

		// the number of (payload) bytes sent over this channel so far. This is
		// meant to map to the TCP sequence number
		std::uint32_t byte_counter = 0;

		// this function must be called with this packet in case the packet is
		// dropped.
		aux::function<void(aux::packet)> drop_fun;
	};

}} // sim

#endif

