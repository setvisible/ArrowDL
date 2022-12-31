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

#include <boost/make_shared.hpp>
#include <boost/system/error_code.hpp>

namespace sim { namespace asio {

	io_context::io_context(sim::simulation& sim)
		: io_context(sim, std::vector<asio::ip::address>())
	{}

	io_context::io_context(sim::simulation& sim, asio::ip::address const& ip)
		: io_context(sim, std::vector<asio::ip::address>{ip})
	{}

	io_context::io_context(sim::simulation& sim, std::vector<asio::ip::address> const& ips)
		: m_sim(sim)
		, m_ips(ips)
		, m_stopped(false)
	{
		for (auto const& ip : m_ips)
		{
			m_outgoing_route[ip] = m_sim.config().outgoing_route(ip);
			m_incoming_route[ip] = m_sim.config().incoming_route(ip);
		}
		m_sim.add_io_service(this);
	}

	io_context::~io_context()
	{
		m_sim.remove_io_service(this);
	}

	io_context::io_context(std::size_t)
		: m_sim(*reinterpret_cast<sim::simulation*>(0))
	{
		assert(false);
	}

	int io_context::get_path_mtu(const asio::ip::address& source, const asio::ip::address& dest) const
	{
		// TODO: it would be nice to actually traverse the virtual network nodes
		// and ask for their MTU instead
		assert(std::count(m_ips.begin(), m_ips.end(), source) > 0 && "source address must be a local address to this node/io_context");
		return m_sim.config().path_mtu(source, dest);
	}

	void io_context::stop()
	{
		// TODO: cancel all outstanding handler associated with this io_context
		m_stopped = true;
	}

	bool io_context::stopped() const
	{
		return m_stopped;
	}

	void io_context::restart()
	{
		m_stopped = false;
	}

	std::size_t io_context::run()
	{
		assert(false);
		return 0;
	}

	std::size_t io_context::run(boost::system::error_code&)
	{
		assert(false);
		return 0;
	}

	std::size_t io_context::poll()
	{
		assert(false);
		return 0;
	}

	std::size_t io_context::poll(boost::system::error_code&)
	{
		assert(false);
		return 0;
	}

	std::size_t io_context::poll_one()
	{
		assert(false);
		return 0;
	}

	std::size_t io_context::poll_one(boost::system::error_code&)
	{
		assert(false);
		return 0;
	}

	// private interface

	void io_context::add_timer(high_resolution_timer* t)
	{
		m_sim.add_timer(t);
	}

	void io_context::remove_timer(high_resolution_timer* t)
	{
		m_sim.remove_timer(t);
	}

	boost::asio::io_context& io_context::get_internal_service()
	{ return m_sim.get_internal_service(); }

	ip::tcp::endpoint io_context::bind_socket(ip::tcp::socket* socket
		, ip::tcp::endpoint ep, boost::system::error_code& ec)
	{
		assert(!m_ips.empty() && "you cannot use an internal io_context (one without an IP address) for creating and binding sockets");
		if (ep.address() == ip::address_v4::any())
		{
			auto it = std::find_if(m_ips.begin(), m_ips.end()
				, [](ip::address const& ip) { return ip.is_v4(); } );
			if (it == m_ips.end())
			{
				ec.assign(boost::system::errc::address_not_available
					, boost::system::generic_category());
				return ip::tcp::endpoint();
			}
			// TODO: pick the first local endpoint for now. In the future we may
			// want have a bias toward
			ep.address(*it);
		}
		else if (ep.address() == ip::address_v6::any())
		{
			auto it = std::find_if(m_ips.begin(), m_ips.end()
				, [](ip::address const& ip) { return ip.is_v6(); } );
			if (it == m_ips.end())
			{
				ec.assign(boost::system::errc::address_not_available
					, boost::system::generic_category());
				return ip::tcp::endpoint();
			}
			// TODO: pick the first local endpoint for now. In the future we may
			// want have a bias toward
			ep.address(*it);
		}
		else if (std::count(m_ips.begin(), m_ips.end(), ep.address()) == 0)
		{
			// you can only bind to the IP assigned to this node.
			// TODO: support loopback
			ec.assign(boost::system::errc::address_not_available
				, boost::system::generic_category());
			return ip::tcp::endpoint();
		}

		return m_sim.bind_socket(socket, ep, ec);
	}

	void io_context::unbind_socket(ip::tcp::socket* socket
		, const ip::tcp::endpoint& ep)
	{
		m_sim.unbind_socket(socket, ep);
	}

	void io_context::rebind_socket(asio::ip::tcp::socket* s, asio::ip::tcp::endpoint ep)
	{
		m_sim.rebind_socket(s, ep);
	}

	ip::udp::endpoint io_context::bind_udp_socket(ip::udp::socket* socket
		, ip::udp::endpoint ep, boost::system::error_code& ec)
	{
		assert(!m_ips.empty() && "you cannot use an internal io_context (one without an IP address) for creating and binding sockets");
		if (ep.address() == ip::address_v4::any())
		{
			auto it = std::find_if(m_ips.begin(), m_ips.end()
				, [](ip::address const& ip) { return ip.is_v4(); });
			if (it == m_ips.end())
			{
				ec.assign(boost::system::errc::address_not_available
					, boost::system::generic_category());
				return ip::udp::endpoint();
			}
			// TODO: pick the first local endpoint for now. In the future we may
			// want have a bias toward
			ep.address(*it);
		}
		else if (ep.address() == ip::address_v6::any())
		{
			auto it = std::find_if(m_ips.begin(), m_ips.end()
				, [](ip::address const& ip) { return ip.is_v6(); });
			if (it == m_ips.end())
			{
				ec.assign(boost::system::errc::address_not_available
						  , boost::system::generic_category());
				return ip::udp::endpoint();
			}
			// TODO: pick the first local endpoint for now. In the future we may
			// want have a bias toward
			ep.address(*it);
		}
		else if (std::count(m_ips.begin(), m_ips.end(), ep.address()) == 0)
		{
			// you can only bind to the IP assigned to this node.
			// TODO: support loopback
			ec.assign(boost::system::errc::address_not_available
				, boost::system::generic_category());
			return ip::udp::endpoint();
		}

		return m_sim.bind_udp_socket(socket, ep, ec);
	}

	void io_context::unbind_udp_socket(ip::udp::socket* socket
		, const ip::udp::endpoint& ep)
	{
		m_sim.unbind_udp_socket(socket, ep);
	}

	void io_context::rebind_udp_socket(asio::ip::udp::socket* socket, asio::ip::udp::endpoint ep)
	{
		m_sim.rebind_udp_socket(socket, ep);
	}

	std::shared_ptr<aux::channel> io_context::internal_connect(ip::tcp::socket* s
		, ip::tcp::endpoint const& target, boost::system::error_code& ec)
	{
		return m_sim.internal_connect(s, target, ec);
	}

	route io_context::find_udp_socket(asio::ip::udp::socket const& socket
		, ip::udp::endpoint const& ep)
	{
		return m_sim.find_udp_socket(socket, ep);
	}

} // asio
} // sim

