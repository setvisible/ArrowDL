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

#include "simulator/pcap.hpp"
#include "simulator/chrono.hpp"
#include "simulator/packet.hpp"

using sim::chrono::duration_cast;
using sim::chrono::seconds;

using sim::asio::ip::address_v4;
using sim::asio::ip::tcp;
using sim::asio::ip::udp;

namespace sim { namespace aux {

namespace {

	template <typename T, typename cond = typename std::enable_if<std::is_pod<T>::value>::type>
	void write(std::fstream& o, T const& value)
	{ o.write(reinterpret_cast<char const*>(&value), sizeof(value)); }

	// documented here:
	// http://www.tcpdump.org/linktypes.html
	std::uint32_t const LINKTYPE_RAW = 101;

	struct ip_header
	{
		std::uint8_t version_len;
		std::uint8_t dscp;
		std::uint16_t length;
		std::uint16_t identification;
		std::uint16_t fragment;
		std::uint8_t ttl;
		std::uint8_t protocol;
		std::uint16_t checksum;
		std::uint32_t source_ip;
		std::uint32_t destination_ip;
	};

	struct udp_header
	{
		std::uint16_t src_port;
		std::uint16_t dst_port;
		std::uint16_t length;
		std::uint16_t checksum;
	};

	struct tcp_header
	{
		std::uint16_t src_port;
		std::uint16_t dst_port;
		std::uint32_t seq_nr;
		std::uint32_t ack_nr;
		std::uint8_t offset;
		std::uint8_t flags;
		std::uint16_t window_size;
		std::uint16_t checksum;
		std::uint16_t urgent;
	};
}

	pcap::pcap(char const* filename)
		: m_file(filename, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary)
	{
		// file format documented here:
		// https://wiki.wireshark.org/Development/LibpcapFileFormat
		// write pcap file header
		write(m_file, std::uint32_t(0xa1b2c3d4)); // magic number
		write(m_file, std::uint16_t(2)); // versopm major
		write(m_file, std::uint16_t(4)); // versopm minor
		write(m_file, std::int32_t(0)); // thiszone
		write(m_file, std::uint32_t(0)); // sigfigs
		write(m_file, std::uint32_t(0xffff)); // snaplen
		write(m_file, LINKTYPE_RAW); // network
	}

	void write_ip_header(std::fstream& file, int const size, int const protocol
		, address_v4 const source, address_v4 const dest)
	{
		ip_header const header = {
			(4 << 4) | 5, // version_len
			0, // dscp
			htons(std::uint16_t(size)), // length
			0, // identification
			0, // fragment
			200, // ttl
			std::uint8_t(protocol), // protocol
			0, // checksum
			htonl(std::uint32_t(source.to_uint())), // source ip
			htonl(std::uint32_t(dest.to_uint())) // destination ip
		};

		write(file, header);
	}

	void write_udp_header(std::fstream& file, int const size, int const src_port
		, int const dst_port)
	{
		assert(src_port != 0);
		assert(dst_port != 0);
		udp_header const header = {
			htons(std::uint16_t(src_port)), // source port
			htons(std::uint16_t(dst_port)), // destination port
			htons(std::uint16_t(sizeof(udp_header) + size)), // length
			0 // checksum
		};

		write(file, header);
	}

	void write_tcp_header(std::fstream& file, int const src_port
		, int const dst_port, std::uint32_t const seq_nr)
	{
		assert(src_port != 0);
		assert(dst_port != 0);
		tcp_header const header = {
			htons(std::uint16_t(src_port)), // source port
			htons(std::uint16_t(dst_port)), // destination port
			htonl(seq_nr), // sequence number
			std::uint32_t(0), // acknowledgement number
			std::uint8_t(5 << 4), // header size (data offset)
			std::uint8_t(0), // flags
			htons(std::uint16_t(65535)), // window size
			std::uint16_t(0), // checksum
			std::uint16_t(0) // urgent offset
		};

		write(file, header);
	}

	void pcap::log_tcp(packet const& p, tcp::endpoint const src
		, tcp::endpoint const dst)
	{
		// synthesize IP/TCP header and write packet
		auto const now = chrono::high_resolution_clock::now();

		// just an arbitrary posix time used as starting point
		std::uint32_t const sim_start_time = 441794304;
		std::uint32_t const secs = static_cast<std::uint32_t>(duration_cast<chrono::seconds>(now.time_since_epoch()).count());
		std::uint32_t const usecs = static_cast<std::uint32_t>(duration_cast<chrono::microseconds>(now.time_since_epoch() - seconds(secs)).count());

		std::uint32_t const packet_size = static_cast<std::uint32_t>(sizeof(ip_header) + sizeof(tcp_header) + p.buffer.size());

		write(m_file, sim_start_time + secs);
		write(m_file, usecs);
		write(m_file, packet_size);
		write(m_file, packet_size);

		// 6 is the protocol number for TCP
		write_ip_header(m_file, packet_size, 6
			, src.address().to_v4(), dst.address().to_v4());

		// TODO: if the packet has error set with asio::error::eof
		// set the FIN flag
		write_tcp_header(m_file, p.from.port(), dst.port(), p.byte_counter);

		m_file.write(reinterpret_cast<char const*>(p.buffer.data()), p.buffer.size());
	}

	void pcap::log_udp(packet const& p, udp::endpoint const src
		, udp::endpoint const dst)
	{
		// synthesize IP/UDP header and write packet
		auto const now = chrono::high_resolution_clock::now();

		// just an arbitrary posix time used as starting point
		std::uint32_t const sim_start_time = 441794304;
		std::uint32_t const secs = static_cast<std::uint32_t>(duration_cast<chrono::seconds>(now.time_since_epoch()).count());
		std::uint32_t const usecs = static_cast<std::uint32_t>(duration_cast<chrono::microseconds>(now.time_since_epoch() - seconds(secs)).count());

		std::uint32_t const packet_size = static_cast<std::uint32_t>(sizeof(ip_header) + sizeof(udp_header) + p.buffer.size());

		write(m_file, sim_start_time + secs);
		write(m_file, usecs);
		write(m_file, packet_size);
		write(m_file, packet_size);

		// 17 is the protocol number for UDP
		write_ip_header(m_file, packet_size, 17
			, src.address().to_v4(), dst.address().to_v4());

		write_udp_header(m_file, static_cast<int>(p.buffer.size()), p.from.port(), static_cast<int>(dst.port()));

		m_file.write(reinterpret_cast<char const*>(p.buffer.data()), p.buffer.size());
	}

}}

