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
#include "simulator/handler_allocator.hpp"

#include <functional>
#include <boost/system/error_code.hpp>

namespace sim
{

	namespace asio {

	high_resolution_timer::high_resolution_timer(io_context& ioc)
		: m_expiration_time(time_type())
		, m_io_service(&ioc)
		, m_expired(true)
	{
	}

	high_resolution_timer::high_resolution_timer(io_context& ioc,
		const time_type& expiry_time)
		: m_expiration_time(time_type())
		, m_io_service(&ioc)
		, m_expired(true)
	{
		expires_at(expiry_time);
	}

	high_resolution_timer::high_resolution_timer(io_context& ioc,
		const duration_type& expiry_time)
		: m_expiration_time(time_type())
		, m_io_service(&ioc)
		, m_expired(true)
	{
		expires_after(expiry_time);
	}

	high_resolution_timer::~high_resolution_timer()
	{
		cancel();
	}

	std::size_t high_resolution_timer::cancel()
	{
		if (m_expired) return 0;
		m_expired = true;
		m_io_service->remove_timer(this);
		if (!m_handler) return 0;
		fire(boost::asio::error::operation_aborted);
		return 1;
	}

	std::size_t high_resolution_timer::cancel_one()
	{
		// TODO: support multiple handlers
		return cancel();
	}

	high_resolution_timer::time_type high_resolution_timer::expiry() const
	{ return m_expiration_time; }

	std::size_t high_resolution_timer::expires_at(high_resolution_timer::time_type const& expiry_time)
	{
		std::size_t ret = cancel();
		m_expiration_time = expiry_time;
		m_expired = false;
		m_io_service->add_timer(this);
		return ret;
	}

	std::size_t high_resolution_timer::expires_after(const duration_type& expiry_time)
	{
		std::size_t ret = cancel();
		m_expiration_time = chrono::high_resolution_clock::now() + expiry_time;
		m_expired = false;
		m_io_service->add_timer(this);
		return ret;
	}

	void high_resolution_timer::wait()
	{
		assert(false && "can't use synchcronous calls in simulator");
		boost::system::error_code ec;
		wait(ec);
	}

	void high_resolution_timer::wait(boost::system::error_code&)
	{
		assert(false && "can't use synchcronous calls in simulator");
		time_type now = chrono::high_resolution_clock::now();
		if (now >= m_expiration_time) return;
		chrono::high_resolution_clock::fast_forward(m_expiration_time - now);
	}

	void high_resolution_timer::async_wait(aux::function<void(boost::system::error_code const&)> handler)
	{
		// TODO: support multiple handlers
		assert(!m_handler);
		m_handler = std::move(handler);
		if (m_expired)
		{
			fire(boost::system::error_code());
			return;
		}
	}

	void high_resolution_timer::fire(boost::system::error_code ec)
	{
		m_expired = true;
		if (!m_handler) return;
		auto h = std::move(m_handler);
		m_handler = nullptr;
		post(*m_io_service, make_malloc(std::bind(std::move(h), ec)));
	}

	high_resolution_timer::executor_type high_resolution_timer::get_executor()
	{
		return (*m_io_service).get_executor();
	}

	} // asio

} // sim

