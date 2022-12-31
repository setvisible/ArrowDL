//
// gnutls/stream_base.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2020 Paul-Louis Ageneau (paul-louis at ageneau dot org)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_GNUTLS_STREAM_BASE_HPP
#define BOOST_ASIO_GNUTLS_STREAM_BASE_HPP

#include "context.hpp"

#include <boost/system/error_code.hpp>

#include <gnutls/gnutls.h>

#include <exception>
#include <memory>
#include <string>

namespace boost {
namespace asio {
namespace gnutls {

class stream_base
{
public:
    using error_code = boost::system::error_code;
    using native_handle_type = gnutls_session_t;

    enum handshake_type
    {
        client,
        server
    };

    stream_base(context& ctx) { set_context(ctx); }
    stream_base(stream_base&& other)
        : m_context_impl(std::move(other.m_context_impl))
    {}
    stream_base(stream_base const& other) = delete;
    virtual ~stream_base() = default;

    context& get_context() const
    {
        if (!m_context_impl->parent) throw std::logic_error("access to destroyed ssl context");

        return *m_context_impl->parent;
    }

    void set_context(context& ctx) { m_context_impl = ctx.m_impl; }

    virtual native_handle_type native_handle() = 0;

#ifndef BOOST_NO_EXCEPTIONS
    virtual void set_host_name(std::string const& name) = 0;
#endif
    virtual error_code set_host_name(std::string const& name, error_code& ec) = 0;

protected:
    std::shared_ptr<context::impl> m_context_impl;
};

} // namespace gnutls
} // namespace asio
} // namespace boost

#endif
