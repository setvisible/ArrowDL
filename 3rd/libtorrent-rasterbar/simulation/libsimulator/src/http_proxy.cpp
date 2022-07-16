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
#include "simulator/http_proxy.hpp"
#include "simulator/http_server.hpp" // for helper functions

#include <functional>
#include <cstdio> // for printf

using namespace sim::asio;
using namespace sim::asio::ip;
using namespace std::placeholders;

using boost::system::error_code;

namespace sim
{
	using namespace aux;

	http_proxy::http_proxy(io_context& ios, unsigned short const listen_port)
		: m_resolver(ios)
		, m_listen_socket(ios)
		, m_client_connection(ios)
		, m_server_connection(ios)
		, m_writing_to_server(false)
		, m_num_client_in_bytes(0)
		, m_num_server_out_bytes(0)
		, m_num_in_bytes(0)
		, m_close(false)
	{
		address local_ip = ios.get_ips().front();
		if (local_ip.is_v4())
		{
			m_listen_socket.open(tcp::v4());
			m_listen_socket.bind(tcp::endpoint(address_v4::any(), listen_port));
		}
		else
		{
			m_listen_socket.open(tcp::v6());
			m_listen_socket.bind(tcp::endpoint(address_v6::any(), listen_port));
		}
		m_listen_socket.listen();

		m_listen_socket.async_accept(m_client_connection, m_ep
			, std::bind(&http_proxy::on_accept, this, _1));
	}

	void http_proxy::on_accept(error_code const& ec)
	{
		if (ec == asio::error::operation_aborted)
			return;

		if (ec)
		{
			std::printf("http_proxy::on_accept: (%d) %s\n"
				, ec.value(), ec.message().c_str());
			close_connection();
			return;
		}

		std::printf("http_proxy accepted connection from: %s : %d\n",
			m_ep.address().to_string().c_str(), m_ep.port());

		// read http request
		m_client_connection.async_read_some(asio::buffer(
			&m_client_in_buffer[0], sizeof(m_client_in_buffer))
			, std::bind(&http_proxy::on_read_request, this, _1, _2));
	}

	void http_proxy::on_read_request(error_code const& ec, size_t bytes_transferred) try
	{
		if (ec)
		{
			std::printf("http_proxy::on_read_request: (%d) %s\n"
				, ec.value(), ec.message().c_str());
			close_connection();
			return;
		}

		m_num_client_in_bytes += int(bytes_transferred);

		// scan for end of HTTP request
		int req_len = find_request_len(m_client_in_buffer, m_num_client_in_bytes);
		while (req_len >= 0)
		{
			// parse request from [0, eor), connect to target server and forward
			// the request.
			http_request const req = parse_request(m_client_in_buffer, req_len);
			forward_request(req);

			// pop this request from the receive buffer
			memmove(m_client_in_buffer, m_client_in_buffer + req_len
				, m_num_client_in_bytes - req_len);
			m_num_client_in_bytes -= req_len;

			// is there another request in the buffer?
			req_len = find_request_len(m_client_in_buffer, m_num_client_in_bytes);
		}

		// read more from the client
		m_client_connection.async_read_some(asio::buffer(
			&m_client_in_buffer[m_num_client_in_bytes]
			, sizeof(m_client_in_buffer) - m_num_client_in_bytes)
			, std::bind(&http_proxy::on_read_request, this, _1, _2));
	}
	catch (std::runtime_error& e)
	{
		std::printf("http_proxy::on_read_request() failed: %s\n"
			, e.what());
		close_connection();
	}

	void http_proxy::forward_request(http_request const& req)
	{
		std::string out_request;
		out_request = req.method;
		out_request += ' ';
		if (req.req.compare(0, 7, "http://") != 0)
		{
			std::printf("http_proxy::forward_request: expected full URL in request, got: %s\n"
				, req.req.c_str());
			throw std::runtime_error("invalid request");
		}

		std::string::size_type const path_start = req.req.find_first_of('/', 7);
		if (path_start == std::string::npos) out_request += '/';
		else out_request.append(req.req, path_start, std::string::npos);
		out_request += " HTTP/1.1\r\n";

		std::string::size_type const host_end = req.req.substr(0, path_start).find_last_of(':');

		std::string host = req.req.substr(7, (host_end != std::string::npos && host_end > 7)
			? host_end - 7 : path_start - 7);

		// if the hostname is an IPv6 address, strip the brackets around it to
		// make it parse correctly
		if (host.size() >= 2 && host.front() == '[' && host.back() == ']')
			host = host.substr(1, host.size() - 2);

		int const port = host_end == std::string::npos || host_end <= 7 ? 80
			: atoi(req.req.substr(host_end + 1, path_start).c_str());
		assert(port >= 0 && port < 0xffff);

		bool found_host = false;
		for (auto const& h : req.headers)
		{
			if (h.first == "host") found_host = true;

			out_request += h.first;
			out_request += ": ";
			out_request += h.second;
			out_request += "\r\n";
		}
		if (!found_host)
		{
			out_request += "host: ";
			out_request += host;
			out_request += "\r\n";
		}
		out_request += "\r\n";

		if (m_num_server_out_bytes + out_request.size() > sizeof(m_server_out_buffer))
		{
			std::printf("http_proxy: Too many queued server requests: %d bytes\n"
				, int(m_num_server_out_bytes + out_request.size()));
			throw std::runtime_error("pipeline too deep");
		}
		memmove(&m_server_out_buffer[m_num_server_out_bytes]
			, out_request.data(), out_request.size());
		m_num_server_out_bytes += int(out_request.size());

		if (!m_server_connection.is_open())
		{
			boost::system::error_code err;
			tcp::endpoint target(make_address(host.c_str(), err)
				, static_cast<unsigned short>(port));
			if (err)
			{
				char port_str[10];
				std::snprintf(port_str, sizeof(port_str), "%d", port);
				m_resolver.async_resolve(host, port_str
					, std::bind(&http_proxy::on_domain_lookup, this, _1, _2));
				return;
			}
			open_forward_connection(target);
			return;
		}

		// TODO: make sure we're connecting/connected to the same (host, port)
		// that this request is for. Don't support multiple servers

		write_server_send_buffer();
	}

	void http_proxy::on_domain_lookup(boost::system::error_code const& ec
		, const asio::ip::tcp::resolver::results_type ips)
	{
		if (ec || ips.empty())
		{
			if (ec)
			{
				std::printf("http_proxy::on_domain_lookup: (%d) %s\n"
					, ec.value(), ec.message().c_str());
			}
			else
			{
				std::printf("http_proxy::on_request_domain_lookup: empty response\n");
			}
			error(503, "Resource Temporarily Unavailable");
			return;
		}
		open_forward_connection(ips.front().endpoint());
	}

	void http_proxy::open_forward_connection(const asio::ip::tcp::endpoint& target)
	{
		m_server_connection.open(target.protocol());

		std::printf("http_proxy: async_connect: %s:%d\n"
			, target.address().to_string().c_str(), target.port());
		m_server_connection.async_connect(target
			, std::bind(&http_proxy::on_connected, this, _1));
	}

	void http_proxy::error(int code, char const* message)
	{
		std::string send_buffer = send_response(code, message);
		memcpy(m_in_buffer, send_buffer.data(), send_buffer.size());
		asio::async_write(m_client_connection, asio::buffer(
			&m_in_buffer[0], send_buffer.size())
			, std::bind(&http_proxy::close_connection, this));
	}

	void http_proxy::on_connected(boost::system::error_code const& ec)
	{
		if (ec)
		{
			std::printf("http_proxy::on_connected() connection failed: %s\n", ec.message().c_str());
			m_server_connection.close();
			error(503, "Service Temporarily Unavailable");
			return;
		}

		std::printf("http_proxy: connected\n");

		write_server_send_buffer();

		m_server_connection.async_read_some(
			asio::buffer(m_in_buffer, sizeof(m_in_buffer))
			, std::bind(&http_proxy::on_server_receive, this, _1, _2));
	}

	void http_proxy::write_server_send_buffer()
	{
		if (m_writing_to_server) return;
		m_writing_to_server = true;
		m_server_connection.async_write_some(asio::buffer(
			&m_server_out_buffer[0], m_num_server_out_bytes)
			, std::bind(&http_proxy::on_server_write, this, _1, _2));
	}

	void http_proxy::on_server_write(error_code const& ec, size_t bytes_transferred)
	{
		m_writing_to_server = false;
		if (ec)
		{
			std::printf("http_proxy::on_server_write: (%d) %s\n"
				, ec.value(), ec.message().c_str());
			close_connection();
			return;
		}

		memmove(&m_server_out_buffer[0], &m_server_out_buffer[bytes_transferred]
			, m_num_server_out_bytes - bytes_transferred);
		m_num_server_out_bytes -= int(bytes_transferred);

		if (m_num_server_out_bytes > 0)
			write_server_send_buffer();
	}

	// we received some data from the server, forward it to the server
	void http_proxy::on_server_receive(boost::system::error_code const& ec
		, std::size_t bytes_transferred)
	{
		if (ec)
		{
			std::printf("http_proxy: error reading from server: (%d) %s\n"
				, ec.value(), ec.message().c_str());
			close_connection();
			return;
		}

		asio::async_write(m_client_connection, asio::buffer(&m_in_buffer[0], bytes_transferred)
			, std::bind(&http_proxy::on_server_forward, this, _1, _2));
	}

	void http_proxy::on_server_forward(error_code const& ec
		, size_t)
	{
		if (ec)
		{
			std::printf("http_proxy: error writing to client: (%d) %s\n"
				, ec.value(), ec.message().c_str());
			close_connection();
			return;
		}

		m_server_connection.async_read_some(
			sim::asio::buffer(m_in_buffer, sizeof(m_in_buffer))
			, std::bind(&http_proxy::on_server_receive, this, _1, _2));
	}

	void http_proxy::stop()
	{
		m_close = true;
		m_listen_socket.close();
	}

	void http_proxy::close_connection()
	{
		m_num_client_in_bytes = 0;
		m_num_server_out_bytes = 0;
		m_num_in_bytes = 0;

		error_code err;
		m_client_connection.close(err);
		if (err)
		{
			std::printf("http_proxy::close: failed to close client connection (%d) %s\n"
				, err.value(), err.message().c_str());
		}
		m_server_connection.close(err);
		if (err)
		{
			std::printf("http_proxy::close: failed to close server connection (%d) %s\n"
				, err.value(), err.message().c_str());
		}

		if (m_close) return;

		// now we can accept another connection
		m_listen_socket.async_accept(m_client_connection, m_ep
			, std::bind(&http_proxy::on_accept, this, _1));
	}
}

