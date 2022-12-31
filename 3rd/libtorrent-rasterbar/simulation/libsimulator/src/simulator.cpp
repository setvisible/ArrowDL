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
#include "simulator/packet.hpp"

#include <functional>
#include <unordered_set>
#include <set>
#include <cstdio> // for printf

#include "simulator/push_warnings.hpp"
#include <boost/system/error_code.hpp>
#include "simulator/pop_warnings.hpp"

typedef sim::chrono::high_resolution_clock::time_point time_point;
typedef sim::chrono::high_resolution_clock::duration duration;

namespace sim {
namespace asio {
namespace ip {

	default_config default_cfg;

} // ip
} // asio

void forward_packet(aux::packet p)
{
	std::shared_ptr<sink> next_hop = p.hops.pop_front();
	if (!next_hop)
	{
		std::fprintf(stderr, "packet lost\n");
		return;
	}
	next_hop->incoming_packet(std::move(p));
}

namespace
{
	// this is a dummy sink for endpoints, wrapping an io_context
	struct endpoint : sink
	{
		endpoint(asio::io_context& ioc)
			: m_ioc(ioc)
		{}

		virtual void incoming_packet(aux::packet p) override final { assert(false); }

		virtual std::string label() const override final
		{
			std::string ret;
			for (auto const& ip : m_ioc.get_ips())
			{
				ret += ip.to_string();
				ret += " ";
			}
			return ret;
		}

		virtual std::string attributes() const override final
		{
			return "shape=ellipse";
		}

	private:
		asio::io_context& m_ioc;
	};
}

namespace
{
	std::string escape_label(std::string n)
	{
		std::string ret;
		for (auto c : n)
		{
			if (c == '\n')
			{
				ret += "\\n";
				continue;
			}
			if (c == '\"')
			{
				ret += "\\\"";
				continue;
			}
			ret += c;
		}
		return ret;
	}
}

void dump_network_graph(simulation const& s, const std::string& filename)
{
	// all edges (directed).
	std::set<std::pair<std::shared_ptr<sink>, std::shared_ptr<sink>>> edges;

	// all network nodes
	std::unordered_set<std::shared_ptr<sink>> nodes;

	// local nodes (subgrapgs)
	std::vector<std::unordered_set<std::shared_ptr<sink>>> local_nodes;

	const std::vector<asio::io_context*> io_services = s.get_all_io_services();

	for (auto ioc : io_services)
	{
		std::shared_ptr<sink> ep = std::make_shared<endpoint>(*ioc);
		local_nodes.push_back(std::unordered_set<std::shared_ptr<sink>>());
		local_nodes.back().insert(ep);

		for (auto const& ip : ioc->get_ips())
		{
			route in = ioc->get_incoming_route(ip);
			route out = ioc->get_outgoing_route(ip);

			// this is the outgoing node for this endpoint. This is
			// how it connects to the network.
			const std::shared_ptr<sink> egress = out.empty() ? ep : out.last();

			// first add both the incoming and outgoing chains
			std::shared_ptr<sink> prev;
			while (!in.empty())
			{
				auto node = in.pop_front();
				local_nodes.back().insert(node);
				if (prev) edges.insert({prev, node});
				prev = node;
			}
			if (prev) edges.insert({prev, ep});

			prev = ep;
			while (!out.empty())
			{
				auto node = out.pop_front();
				local_nodes.back().insert(node);
				edges.insert({prev, node});
				prev = node;
			}

			// then connect the endpoint of those chains to the rest of the network.
			// Since the network may be arbitrarily complex, we actually have to
			// completely iterate over all other endpoints

			for (auto ios2 : io_services)
			{
				for (auto const& ip2 : ios2->get_ips())
				{
					route network = s.config().channel_route(
						ip, ip2);

					std::shared_ptr<sink> last = ios2->get_incoming_route(ip2).next_hop();

					prev = egress;
					while (!network.empty())
					{
						auto node = network.pop_front();
						nodes.insert(node);
						edges.insert({prev, node});
						prev = node;
					}
					edges.insert({prev, last});
				}
			}
		}
	}

	// by now, the nodes and edges should represent the complete graph. Render it
	// into dot.

	FILE* f = fopen(filename.c_str(), "w+");

	std::fprintf(f, "digraph network {\n"
		"concentrate=true;\n"
		"overlap=scale;\n"
		"splines=true;\n");

	std::fprintf(f, "\n// nodes\n\n");

	for (const auto& n : nodes)
	{
		std::string attributes = n->attributes();
		std::fprintf(f, " \"%p\" [label=\"%s\",style=\"filled\",color=\"red\"%s%s];\n"
			, static_cast<void*>(n.get())
			, escape_label(n->label()).c_str()
			, attributes.empty() ? "" : ", "
			, attributes.c_str());
	}

	std::fprintf(f, "\n// local networks\n\n");

	int idx = 0;
	for (auto ln : local_nodes)
	{
		std::fprintf(f, "subgraph cluster_%d {\n", idx++);

		for (const auto& n : ln)
		{
			std::string attributes = n->attributes();
			std::fprintf(f, " \"%p\" [label=\"%s\",style=\"filled\",color=\"green\"%s%s];\n"
				, static_cast<void*>(n.get())
				, escape_label(n->label()).c_str()
				, attributes.empty() ? "" : ", "
				, attributes.c_str());
		}

		std::fprintf(f, "}\n");
	}

	std::fprintf(f, "\n// edges\n\n");

	while (!edges.empty())
	{
		auto edge = *edges.begin();
		edges.erase(edges.begin());

		std::fprintf(f, "\"%p\" -> \"%p\"\n"
			, static_cast<void*>(edge.first.get())
			, static_cast<void*>(edge.second.get()));
	}

	std::fprintf(f, "}\n");
	fclose(f);
}

namespace aux {

	int channel::remote_idx(const asio::ip::tcp::endpoint& self) const
	{
		if (ep[0] == self) return 1;
		if (ep[1] == self) return 0;
		assert(false && "invalid socket");
		return -1;
	}

	int channel::self_idx(const asio::ip::tcp::endpoint& self) const
	{
		if (ep[0] == self) return 0;
		if (ep[1] == self) return 1;
		assert(false && "invalid socket");
		return -1;
	}

} // aux
} // sim

