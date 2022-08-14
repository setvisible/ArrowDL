/*

Copyright (c) 2017, Arvid Norberg
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

#ifndef PCAP_HPP_INCLUDED
#define PCAP_HPP_INCLUDED

#include <fstream>
#include "simulator/simulator.hpp" // for endpoint

namespace sim { namespace aux
{
	struct packet;

	struct pcap
	{
		pcap(char const* filename);
		void log_tcp(packet const& p, asio::ip::tcp::endpoint src
			, asio::ip::tcp::endpoint dst);
		void log_udp(packet const& p, asio::ip::udp::endpoint src
			, asio::ip::udp::endpoint dst);
	private:
		std::fstream m_file;
	};
}}

#endif

