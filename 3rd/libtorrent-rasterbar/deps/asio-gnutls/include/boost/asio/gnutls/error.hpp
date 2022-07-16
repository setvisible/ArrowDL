//
// gnutls/error.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2020 Paul-Louis Ageneau (paul-louis at ageneau dot org)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_GNUTLS_ERROR_HPP
#define BOOST_ASIO_GNUTLS_ERROR_HPP

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <gnutls/gnutls.h>

#include <string>

namespace boost {
namespace asio {
namespace gnutls {

class error_category : public boost::system::error_category
{
public:
    char const* name() const BOOST_ASIO_ERROR_CATEGORY_NOEXCEPT { return "GnuTLS"; }

    std::string message(int value) const
    {
        char const* s = gnutls_strerror(value);
        return s ? s : "GnuTLS error";
    }
};

namespace error {

enum stream_errors
{
  stream_truncated = 1,
  unspecified_system_error = 2,
  unexpected_result = 3
};

static const boost::system::error_category& get_ssl_category()
{
    static error_category instance;
    return instance;
}

static const boost::system::error_category& get_stream_category() { return get_ssl_category(); }

static const auto& ssl_category BOOST_ASIO_UNUSED_VARIABLE = get_ssl_category();
static const auto& stream_category BOOST_ASIO_UNUSED_VARIABLE = get_stream_category();

} // namespace error
} // namespace gnutls
} // namespace asio
} // namespace boost

namespace boost {
namespace system {

template<> struct is_error_code_enum<boost::asio::gnutls::error::stream_errors>
{
  static const bool value = true;
};

} // namespace system
} // namespace boost

namespace boost {
namespace asio {
namespace gnutls {
namespace error {
inline boost::system::error_code make_error_code(stream_errors e)
{
  return boost::system::error_code(
      static_cast<int>(e), get_stream_category());
}

} // namespace error
} // namespace gnutls
} // namespace asio
} // namespace boost

#endif
