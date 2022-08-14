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

#ifndef SIMULATOR_FUNCTION_HPP_INCLUDED
#define SIMULATOR_FUNCTION_HPP_INCLUDED

#include <utility>
#include <memory> // for allocator_traits

#include "simulator/mallocator.hpp"

namespace sim {
namespace aux {

	template <typename T, typename U>
	T exchange_(T& var, U&& new_val)
	{
		T temp = std::move(var);
		var = std::forward<U>(new_val);
		return temp;
	}

	template <typename T, typename Fun>
	T* allocate_handler(Fun h)
	{
		using alloc = typename boost::asio::associated_allocator<typename std::remove_reference<Fun>::type>::type;
		using our_alloc = typename std::allocator_traits<alloc>::template rebind_alloc<T>;
		our_alloc al(boost::asio::get_associated_allocator(h));
		void* ptr = al.allocate(1);
		if (ptr == nullptr) throw std::bad_alloc();
		try {
			return new (ptr) T(std::move(h));
		}
		catch (...) {
			al.deallocate(reinterpret_cast<T*>(ptr), 1);
			throw;
		}
	}

	// this is a std::function-like class that supports move-only function
	// objects
	template <typename R, typename... A>
	struct callable
	{
		using call_fun_t = R (*)(void*, A&&...);
		using deallocate_fun_t = void (*)(void*);
		call_fun_t call_fun;
		deallocate_fun_t deallocate_fun;
	};

	template <typename Handler, typename R, typename... A>
	R call_impl(void* mem, A&&... a);

	template <typename Handler, typename R, typename... A>
	void dealloc_impl(void* mem);

	template <typename Handler, typename R, typename... A>
	struct function_impl : callable<R, A...>
	{
		function_impl(Handler h)
			: handler(std::move(h))
		{
			this->call_fun = call_impl<Handler, R, A&&...>;
			this->deallocate_fun = dealloc_impl<Handler, R, A&&...>;
		}
		Handler handler;
	};

	template <typename Handler, typename R, typename... A>
	R call_impl(void* mem, A&&... a)
	{
		auto* obj = static_cast<function_impl<Handler, R, A...>*>(mem);
		Handler handler = std::move(obj->handler);

		obj->~function_impl();
		using alloc = typename boost::asio::associated_allocator<Handler>::type;
		using our_alloc = typename std::allocator_traits<alloc>::
			template rebind_alloc<typename std::remove_reference<decltype(*obj)>::type>;
		our_alloc al(boost::asio::get_associated_allocator(handler));
		al.deallocate(obj, 1);

		return handler(std::forward<A>(a)...);
	}

	template <typename Handler, typename R, typename... A>
	void dealloc_impl(void* mem)
	{
		auto* obj = static_cast<function_impl<Handler, R, A...>*>(mem);
		Handler h = std::move(obj->handler);

		obj->~function_impl();
		using alloc = typename boost::asio::associated_allocator<Handler>::type;
		using our_alloc = typename std::allocator_traits<alloc>::
			template rebind_alloc<typename std::remove_reference<decltype(*obj)>::type>;
		our_alloc al(boost::asio::get_associated_allocator(h));
		al.deallocate(obj, 1);
	}

	template <typename Fun>
	struct function;

	template <typename R, typename... A>
	struct function<R(A...)>
	{
		using result_type = R;

		using allocator_type = aux::mallocator<R(A...)>;
		allocator_type get_allocator() const { return allocator_type{}; }

		template <typename C>
		function(C c)
			: m_callable(allocate_handler<function_impl<C, R, A...>>(std::move(c)))
		{}
		function(function&& other) noexcept
			: m_callable(exchange_(other.m_callable, nullptr))
		{}
		function& operator=(function&& other) noexcept
		{
			if (&other == this) return *this;
			clear();
			m_callable = exchange_(other.m_callable, nullptr);
			return *this;
		}

		~function() { clear(); }

		// boost.asio requires handlers to be copy-constructible, but it will move
		// them, if they're movable. So we trick asio into accepting this handler.
		// If it attempts to copy, it will cause a link error
		function(function const&) { assert(false && "functions should not be copied"); }
		function& operator=(function const&) = delete;

		function() = default;
		explicit operator bool() const { return m_callable != nullptr; }
		function& operator=(std::nullptr_t) { clear(); return *this; }
		void clear()
		{
			if (m_callable == nullptr) return;
			auto fun = m_callable->deallocate_fun;
			fun(m_callable);
			m_callable = nullptr;
		}
		template <typename... Args>
		R operator()(Args&&... a)
		{
			assert(m_callable);
			auto fun = m_callable->call_fun;
			return fun(exchange_(m_callable, nullptr), std::forward<A>(a)...);
		}
	private:
		callable<R, A...>* m_callable = nullptr;
	};

	// index sequence, to unpack tuple
	template<std::size_t...> struct seq {};
	template<std::size_t N, std::size_t... S> struct gens : gens<N-1, N-1, S...> {};
	template<std::size_t... S> struct gens<0, S...> { using type = seq<S...>; };

	// a binder for move-only types, and movable arguments. It's not a general
	// binder as it doesn't support partial application, it just binds all
	// arguments and ignores any arguments passed to the call
	template <typename Callable, typename R, typename... A>
	struct move_binder
	{
		move_binder(Callable c, A&&... a)
			: m_args(std::move(a)...)
			, m_callable(std::move(c))
		{}

		move_binder(move_binder const&) = delete;
		move_binder& operator=(move_binder const&) = delete;

		move_binder(move_binder&&) = default;
		move_binder& operator=(move_binder&&) = default;

		// ignore any arguments passed in. This is used to ignore an error_code
		// argument for instance
		template <typename... Args>
		R operator()(Args...)
		{
			return call(typename gens<sizeof...(A)>::type());
		}

	private:

		template<std::size_t... I>
		R call(seq<I...>)
		{
			return m_callable(std::move(std::get<I>(m_args))...);
		}
		std::tuple<A...> m_args;
		Callable m_callable;
	};

	template <typename R, typename C, typename... A>
	move_binder<C, R, A...> move_bind(C c, A&&... a)
	{
		return move_binder<C, R, A...>(std::move(c), std::forward<A>(a)...);
	}

}
}

#endif

