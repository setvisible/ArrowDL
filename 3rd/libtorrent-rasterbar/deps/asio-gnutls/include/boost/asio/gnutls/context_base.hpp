//
// gnutls/context_base.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2020 Paul-Louis Ageneau (paul-louis at ageneau dot org)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_GNUTLS_CONTEXT_BASE_HPP
#define BOOST_ASIO_GNUTLS_CONTEXT_BASE_HPP

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <gnutls/gnutls.h>

namespace boost {
namespace asio {
namespace gnutls {

class stream_base;
template <typename next_layer_type> class stream;

typedef int verify_mode;

BOOST_ASIO_STATIC_CONSTANT(int, verify_none = 0x00);
BOOST_ASIO_STATIC_CONSTANT(int, verify_peer = 0x01);
BOOST_ASIO_STATIC_CONSTANT(int, verify_fail_if_no_peer_cert = 0x02);
BOOST_ASIO_STATIC_CONSTANT(int, verify_client_once = 0x04); // Ignored

class context_base
{
public:
    using error_code = boost::system::error_code;
    using native_handle_type = gnutls_certificate_credentials_t;

    enum method : int
    {
        // Any TLS version
        tls = 0x0000,
        tls_client = 0x0001,
        tls_server = 0x0002,

        // Force specific TLS version
        tlsv1 = 0x1000, // 0xXY.. => TLS X.Y
        tlsv1_client = 0x1001,
        tlsv1_server = 0x1002,
        tlsv11 = 0x1100,
        tlsv11_client = 0x1101,
        tlsv11_server = 0x1102,
        tlsv12 = 0x1200,
        tlsv12_client = 0x1201,
        tlsv12_server = 0x1202,
        tlsv13 = 0x1300,
        tlsv13_client = 0x1301,
        tlsv13_server = 0x1302,

        // SSLv3 + TLS (for compatibility only)
        sslv23 = 0x0300,
        sslv23_client = 0x0301,
        sslv23_server = 0x0302,
    };

    enum file_format
    {
        pem,
        der
    };

    typedef long options;

    BOOST_ASIO_STATIC_CONSTANT(long, default_workarounds = 0x01);
    BOOST_ASIO_STATIC_CONSTANT(long, single_dh_use = 0x02);       // Ignored
    BOOST_ASIO_STATIC_CONSTANT(long, no_sslv2 = 0x04);            // Ignored, always disabled
    BOOST_ASIO_STATIC_CONSTANT(long, no_sslv3 = 0x08);

    using verify_mode = gnutls::verify_mode;

    BOOST_ASIO_STATIC_CONSTANT(int, verify_none = gnutls::verify_none);
    BOOST_ASIO_STATIC_CONSTANT(int, verify_peer = gnutls::verify_peer);
    BOOST_ASIO_STATIC_CONSTANT(int,
                               verify_fail_if_no_peer_cert = gnutls::verify_fail_if_no_peer_cert);
    BOOST_ASIO_STATIC_CONSTANT(int, verify_client_once = gnutls::verify_client_once);
};

} // namespace gnutls
} // namespace asio
} // namespace boost

#endif
