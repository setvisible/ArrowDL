/*

Copyright (c) 2016, Arvid Norberg
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

#ifndef UTILS_HPP_INCLUDED
#define UTILS_HPP_INCLUDED

#include "simulator/simulator.hpp"

namespace sim
{

// shortcut for creating a timer with a timeout and action
struct timer
{
	timer(simulation& sim, chrono::high_resolution_clock::duration timeout
		, aux::function<void(boost::system::error_code const&)>&& f)
		: m_ios(sim, asio::ip::address_v4())
		, m_timer(m_ios)
	{
		m_timer.expires_after(timeout);
		m_timer.async_wait(std::move(f));
	}

private:
	sim::asio::io_context m_ios;
	sim::asio::high_resolution_timer m_timer;
};

} // sim

#endif

