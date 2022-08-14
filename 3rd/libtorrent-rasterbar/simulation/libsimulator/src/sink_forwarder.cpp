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

#include "simulator/sink_forwarder.hpp"
#include "simulator/packet.hpp"

namespace sim { namespace aux {

	sink_forwarder::sink_forwarder(sink* dst)
		: m_dst(dst)
	{}

	void sink_forwarder::incoming_packet(packet p)
	{
		if (m_dst == nullptr) return;
		m_dst->incoming_packet(std::move(p));
	}

	std::string sink_forwarder::label() const
	{
		return m_dst ? m_dst->label() : "";
	}

	void sink_forwarder::reset(sink* s)
	{
		m_dst = s;
	}

}}

