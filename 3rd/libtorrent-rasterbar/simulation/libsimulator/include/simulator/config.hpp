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

#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#include "simulator/push_warnings.hpp"
#include <boost/config.hpp>
#include "simulator/pop_warnings.hpp"

#ifdef SIMULATOR_BUILDING_SHARED
#define SIMULATOR_DECL BOOST_SYMBOL_EXPORT
#elif defined SIMULATOR_LINKING_SHARED
#define SIMULATOR_DECL BOOST_SYMBOL_IMPORT
#else
#define SIMULATOR_DECL
#endif

#if defined __clang__ || defined __GNUC__
#define LIBSIMULATOR_NO_RETURN __attribute((noreturn))
#elif _MSC_VER
#define LIBSIMULATOR_NO_RETURN __declspec(noreturn)
#else
#define LIBSIMULATOR_NO_RETURN
#endif

#ifdef _MSC_VER
#pragma warning(push)
// warning C4251: X: class Y needs to have dll-interface to be used by clients of struct
#pragma warning( disable : 4251)
// warning C4661: X: no suitable definition provided for explicit template instantiation request
#pragma warning( disable : 4661)
#endif

#endif

