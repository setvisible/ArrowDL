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

#ifndef SINK_FORWARDER_HPP_INCLUDED
#define SINK_FORWARDER_HPP_INCLUDED

#include "simulator/config.hpp"
#include "simulator/sink.hpp"

namespace sim { namespace aux {

	struct packet;

	struct SIMULATOR_DECL sink_forwarder final : sink
	{
		sink_forwarder(sink* dst);
		void incoming_packet(packet p) override;
		std::string label() const override;
		void reset(sink* s = nullptr);

	private:
		sink* m_dst;
	};

}} // sim

#endif

