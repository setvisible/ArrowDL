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

#include <functional>

#include "simulator/push_warnings.hpp"
#include <boost/system/error_code.hpp>
#include "simulator/pop_warnings.hpp"

namespace sim { namespace chrono {
	namespace {

		// this is the global simulation timer
		high_resolution_clock::time_point g_simulation_time;
	}

	high_resolution_clock::time_point high_resolution_clock::now()
	{
		return g_simulation_time;
	}

	void high_resolution_clock::fast_forward(high_resolution_clock::duration d)
	{
		g_simulation_time += d;
	}

	void reset_clock()
	{
		g_simulation_time = high_resolution_clock::time_point{};
	}

} // chrono
} // sim

