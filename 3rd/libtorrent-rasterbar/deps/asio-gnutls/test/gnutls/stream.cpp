//
// stream.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2020 Paul-Louis Ageneau (paul-louis at ageneau dot org)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <boost/asio/gnutls/stream.hpp>

#include "../unit_test.hpp"
#include <boost/asio.hpp>
#include <boost/asio/gnutls.hpp>

//------------------------------------------------------------------------------

// gnutls_stream_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that all public member functions on the class
// gnutls::stream::socket compile and link correctly. Runtime failures are ignored.

namespace gnutls_stream_compile {

bool verify_callback(bool, boost::asio::gnutls::verify_context&) { return false; }

void handshake_handler(const boost::system::error_code&) {}

void buffered_handshake_handler(const boost::system::error_code&, std::size_t) {}

void shutdown_handler(const boost::system::error_code&) {}

void write_some_handler(const boost::system::error_code&, std::size_t) {}

void read_some_handler(const boost::system::error_code&, std::size_t) {}

void test()
{
  using namespace boost::asio;
  namespace ip = boost::asio::ip;

  try
  {
    io_context ioc;
    char mutable_char_buffer[128] = "";
    const char const_char_buffer[128] = "";
    std::string hostname = "hostname";
    boost::asio::gnutls::context context(boost::asio::gnutls::context::tls);
    boost::system::error_code ec;

    // gnutls::stream constructors.

    gnutls::stream<ip::tcp::socket> stream1(ioc, context);
    ip::tcp::socket socket1(ioc, ip::tcp::v4());
    gnutls::stream<ip::tcp::socket&> stream2(socket1, context);

    // basic_io_object functions.

    gnutls::stream<ip::tcp::socket>::executor_type ex = stream1.get_executor();
    (void)ex;

    // gnutls::stream functions.

    stream1.set_verify_mode(gnutls::verify_none);
    stream1.set_verify_mode(gnutls::verify_none, ec);

    stream1.set_verify_depth(1);
    stream1.set_verify_depth(1, ec);

    stream1.set_verify_callback(verify_callback);
    stream1.set_verify_callback(verify_callback, ec);

    gnutls_session_t session1 = stream1.native_handle();
    (void)session1;

    gnutls::stream<ip::tcp::socket>::lowest_layer_type& lowest_layer = stream1.lowest_layer();
    (void)lowest_layer;

    const gnutls::stream<ip::tcp::socket>& stream3 = stream1;
    const gnutls::stream<ip::tcp::socket>::lowest_layer_type& lowest_layer2 =
        stream3.lowest_layer();
    (void)lowest_layer2;

    stream1.handshake(gnutls::stream_base::client);
    stream1.handshake(gnutls::stream_base::server);
    stream1.handshake(gnutls::stream_base::client, ec);
    stream1.handshake(gnutls::stream_base::server, ec);

    stream1.handshake(gnutls::stream_base::client, buffer(mutable_char_buffer));
    stream1.handshake(gnutls::stream_base::server, buffer(mutable_char_buffer));
    stream1.handshake(gnutls::stream_base::client, buffer(const_char_buffer));
    stream1.handshake(gnutls::stream_base::server, buffer(const_char_buffer));
    stream1.handshake(gnutls::stream_base::client, buffer(mutable_char_buffer), ec);
    stream1.handshake(gnutls::stream_base::server, buffer(mutable_char_buffer), ec);
    stream1.handshake(gnutls::stream_base::client, buffer(const_char_buffer), ec);
    stream1.handshake(gnutls::stream_base::server, buffer(const_char_buffer), ec);

    stream1.async_handshake(gnutls::stream_base::client, handshake_handler);
    stream1.async_handshake(gnutls::stream_base::server, handshake_handler);

    stream1.async_handshake(
        gnutls::stream_base::client, buffer(mutable_char_buffer), buffered_handshake_handler);
    stream1.async_handshake(
        gnutls::stream_base::server, buffer(mutable_char_buffer), buffered_handshake_handler);
    stream1.async_handshake(
        gnutls::stream_base::client, buffer(const_char_buffer), buffered_handshake_handler);
    stream1.async_handshake(
        gnutls::stream_base::server, buffer(const_char_buffer), buffered_handshake_handler);

    stream1.shutdown();
    stream1.shutdown(ec);

    stream1.async_shutdown(shutdown_handler);

    stream1.write_some(buffer(mutable_char_buffer));
    stream1.write_some(buffer(const_char_buffer));
    stream1.write_some(buffer(mutable_char_buffer), ec);
    stream1.write_some(buffer(const_char_buffer), ec);

    stream1.async_write_some(buffer(mutable_char_buffer), write_some_handler);
    stream1.async_write_some(buffer(const_char_buffer), write_some_handler);

    stream1.read_some(buffer(mutable_char_buffer));
    stream1.read_some(buffer(mutable_char_buffer), ec);

    stream1.async_read_some(buffer(mutable_char_buffer), read_some_handler);

    // SNI extension

    stream1.set_host_name(hostname);
    stream1.set_host_name(hostname, ec);
  }
  catch (std::exception&)
  {
  }
}

} // namespace gnutls_stream_compile

//------------------------------------------------------------------------------

BOOST_ASIO_TEST_SUITE("gnutls/stream", BOOST_ASIO_TEST_CASE(gnutls_stream_compile::test))
