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

#ifndef NOEXCEPT_MOVABLE_HPP_INCLUDED
#define NOEXCEPT_MOVABLE_HPP_INCLUDED

namespace sim {
namespace aux {

	template <typename T>
	struct noexcept_movable : T
	{
		noexcept_movable() noexcept {}
		noexcept_movable(noexcept_movable<T>&& rhs) noexcept
			: T(std::forward<T>(rhs))
		{}
		noexcept_movable(noexcept_movable<T> const& rhs)
			: T(static_cast<T const&>(rhs))
		{}
		noexcept_movable(T&& rhs) noexcept : T(std::forward<T>(rhs)) {} // NOLINT
		noexcept_movable(T const& rhs) : T(rhs) {} // NOLINT
		noexcept_movable& operator=(noexcept_movable&& rhs) noexcept
		{
			this->T::operator=(std::forward<T>(rhs));
			return *this;
		}
		noexcept_movable& operator=(noexcept_movable const& rhs)
		{
			this->T::operator=(rhs);
			return *this;
		}

		using T::T;
		using T::operator=;
	};

}
}

#endif

