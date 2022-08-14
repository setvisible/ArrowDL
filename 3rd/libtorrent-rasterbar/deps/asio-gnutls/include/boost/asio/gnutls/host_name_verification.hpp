//
// gnutls/host_name_verification.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2020 Paul-Louis Ageneau (paul-louis at ageneau dot org)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_GNUTLS_HOST_NAME_VERIFICATION_HPP
#define BOOST_ASIO_GNUTLS_HOST_NAME_VERIFICATION_HPP

#include <boost/asio/gnutls/verify_context.hpp>

#include <gnutls/x509.h>

#include <string>

namespace boost {
namespace asio {
namespace gnutls {

// Verifies a certificate against a host_name according to the rules described in RFC 6125.
class host_name_verification
{
public:
  typedef bool result_type;

  explicit host_name_verification(std::string host)
      : m_host(std::move(host))
  {}

  // Perform certificate verification.
  bool operator()(bool preverified, verify_context& ctx) const
  {
      // Don't bother looking at certificates that have failed pre-verification.
      if (!preverified) return false;

      // Return non-zero for a successful match
      return gnutls_x509_crt_check_hostname(ctx.native_handle(), m_host.c_str()) != 0;
  }

private:
    // The host name to be checked.
    std::string m_host;
};

} // namespace gnutls
} // namespace asio
} // namespace boost

#endif // BOOST_ASIO_GNUTLS_HOST_NAME_VERIFICATION_HPP
