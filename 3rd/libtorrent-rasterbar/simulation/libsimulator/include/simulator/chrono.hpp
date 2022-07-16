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

#ifndef CHRONO_HPP_INCLUDED
#define CHRONO_HPP_INCLUDED

#include <boost/config.hpp>
#include "simulator/config.hpp"

#if defined BOOST_ASIO_HAS_STD_CHRONO
#include <chrono>
#else
#include "simulator/push_warnings.hpp"

#include <boost/chrono/duration.hpp>
#include <boost/chrono/time_point.hpp>
#include <boost/ratio.hpp>

#include "simulator/pop_warnings.hpp"
#endif

namespace sim { namespace chrono
{
#if defined BOOST_ASIO_HAS_STD_CHRONO
	using std::chrono::seconds;
	using std::chrono::milliseconds;
	using std::chrono::microseconds;
	using std::chrono::nanoseconds;
	using std::chrono::minutes;
	using std::chrono::hours;
	using std::chrono::duration_cast;
	using std::chrono::time_point;
	using std::chrono::duration;
#else
	using boost::chrono::seconds;
	using boost::chrono::milliseconds;
	using boost::chrono::microseconds;
	using boost::chrono::nanoseconds;
	using boost::chrono::minutes;
	using boost::chrono::hours;
	using boost::chrono::duration_cast;
	using boost::chrono::time_point;
	using boost::chrono::duration;
#endif

	// std.chrono / boost.chrono compatible high_resolution_clock using a simulated time
	struct SIMULATOR_DECL high_resolution_clock
	{
		using rep = std::int64_t;
#if defined BOOST_ASIO_HAS_STD_CHRONO
		using period = std::nano;
		using time_point = std::chrono::time_point<high_resolution_clock, nanoseconds>;
		using duration = std::chrono::duration<boost::int64_t, std::nano>;
#else
		using period = boost::nano;
		using time_point = time_point<high_resolution_clock, nanoseconds>;
		using duration = duration<boost::int64_t, boost::nano>;
#endif
		static const bool is_steady = true;
		static time_point now();

		// private interface
		static void fast_forward(high_resolution_clock::duration d);
	};

	// private interface
	void reset_clock();

} // chrono
} // sim

#endif

