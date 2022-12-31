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

#ifndef HANDLER_ALLOCATOR_HPP_INCLUDED
#define HANDLER_ALLOCATOR_HPP_INCLUDED

namespace sim
{
namespace aux
{

template <typename T>
struct malloc_allocator
{
	using value_type = T;
	using size_type = std::size_t;

	friend bool operator==(malloc_allocator, malloc_allocator) { return true; }
	friend bool operator!=(malloc_allocator, malloc_allocator) { return false; }

	template <class U>
	struct rebind { using other = malloc_allocator<U>; };

	malloc_allocator() = default;
	template <typename U>
	malloc_allocator(malloc_allocator<U> const&) {}

	T* allocate(std::size_t size) { return static_cast<T*>(std::malloc(size * sizeof(T))); }
	void deallocate(T* pointer, std::size_t) { std::free(pointer); }
	using is_always_equal = std::true_type;
};

// this is a handler wrapper that customizes the asio handler allocator to use
// malloc instead of new. The purpose is to distinguish allocations that are
// internal to the simulator and allocations part of the program under test.
template <typename Handler>
struct malloc_wrapper
{
	malloc_wrapper(Handler h) : m_handler(std::move(h)) {}

	template <typename... Args>
	void operator()(Args&&... a)
	{
		m_handler(std::forward<Args>(a)...);
	}

	using allocator_type = malloc_allocator<malloc_wrapper<Handler>>;

	allocator_type get_allocator() const noexcept
	{ return allocator_type{}; }

private:
	Handler m_handler;
};

template <typename T>
malloc_wrapper<T> make_malloc(T h)
{
	return malloc_wrapper<T>(std::move(h));
}

}
}

#endif

