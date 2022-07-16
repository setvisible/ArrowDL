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

#ifndef HTTP_PROXY_HPP_INCLUDED
#define HTTP_PROXY_HPP_INCLUDED

#include "simulator/simulator.hpp"

#ifdef _MSC_VER
#pragma warning(push)
// warning C4251: X: class Y needs to have dll-interface to be used by clients of struct
#pragma warning( disable : 4251)
#endif

namespace sim
{
	struct http_request;

// This is a very simple http proxy that only supports a single
// concurrent connection
struct SIMULATOR_DECL http_proxy
{
	http_proxy(asio::io_context& ios, unsigned short listen_port);

	void stop();

private:

	void on_accept(boost::system::error_code const& ec);
	void on_read_request(boost::system::error_code const& ec, size_t bytes_transferred);

	void forward_request(http_request const& req);
	void open_forward_connection(const asio::ip::tcp::endpoint& target);
	void on_connected(boost::system::error_code const& ec);

	void on_domain_lookup(boost::system::error_code const& ec
		, const asio::ip::tcp::resolver::results_type ips);

	void write_server_send_buffer();
	void on_server_write(boost::system::error_code const& ec, size_t bytes_transferred);

	void on_server_receive(boost::system::error_code const& ec
		, std::size_t bytes_transferred);
	void on_server_forward(boost::system::error_code const& ec, size_t bytes_transferred);

	void error(int code, char const* message);
	void close_connection();

	asio::ip::tcp::resolver m_resolver;
	asio::ip::tcp::acceptor m_listen_socket;

	// this is the client connection, i.e. the client connecting to us, sending
	// HTTP requests that we forward
	asio::ip::tcp::socket m_client_connection;
	// client endpoint
	asio::ip::tcp::endpoint m_ep;

	// this is the connection to the server the client's requests are forwarded
	// to
	asio::ip::tcp::socket m_server_connection;
	// true while there is an outstanding write operation to the server
	bool m_writing_to_server;

	// receive buffer for requests from the client. i.e. client -> proxy (us) -> server
	char m_client_in_buffer[65536];
	// buffer size
	int m_num_client_in_bytes;

	char m_server_out_buffer[65536];
	int m_num_server_out_bytes;

	// receive buffer for incoming responses, i.e. server -> proxy (us) -> client
	char m_in_buffer[65536];
	// buffer size
	int m_num_in_bytes;

	// set to true when shutting down
	bool m_close;
};

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

