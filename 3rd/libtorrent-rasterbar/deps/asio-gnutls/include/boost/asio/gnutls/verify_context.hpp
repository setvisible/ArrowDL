//
// gnutls/context.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2020 Paul-Louis Ageneau (paul-louis at ageneau dot org)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_GNUTLS_VERIFY_CONTEXT_HPP
#define BOOST_ASIO_GNUTLS_VERIFY_CONTEXT_HPP

#include <gnutls/x509.h>

namespace boost {
namespace asio {
namespace gnutls {

class verify_context
{
public:
    using native_handle_type = gnutls_x509_crt_t;

    explicit verify_context(native_handle_type cert)
        : m_cert(cert)
    {}

    native_handle_type native_handle() { return m_cert; }

private:
    gnutls_x509_crt_t m_cert;
};

} // namespace gnutls
} // namespace asio
} // namespace boost

#endif

