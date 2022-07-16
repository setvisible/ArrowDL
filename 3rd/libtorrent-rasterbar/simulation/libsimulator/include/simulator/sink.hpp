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

#ifndef SINK_HPP_INCLUDED
#define SINK_HPP_INCLUDED

#include "simulator/config.hpp"
#include <string>

namespace sim {

namespace aux {
	struct packet;
}

	// this is an interface for somthing that can accept incoming packets,
	// such as queues, sockets, NATs and TCP congestion windows
	struct SIMULATOR_DECL sink
	{
		virtual void incoming_packet(aux::packet p) = 0;

		// used for visualization
		virtual std::string label() const = 0;

		virtual std::string attributes() const { return "shape=box"; }
		virtual ~sink() = default;
	};

} // sim

#endif

