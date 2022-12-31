libsimulator
============

.. image:: https://travis-ci.org/arvidn/libsimulator.svg?branch=master
    :target: https://travis-ci.org/arvidn/libsimulator

.. image:: https://ci.appveyor.com/api/projects/status/0857n4g3f6mui90i/branch/master
    :target: https://ci.appveyor.com/project/arvidn/libsimulator/branch/master

*This is still in initial development, some of this README represents ambitions
rather than the current state*

libsimulator is a library for running discrete event simulations, implementing
the ``boost.asio`` API (or a somewhat faithful emulation of a subset of it,
patches are welcome). This makes it practical to be used as a testing tool of
real implementations of network software as well as for writing simulators that
later turn into live production applications.

The simulation has to have a single time-line to be deterministic, meaning it
must be single threaded and use a single ``io_service`` as the message queue.
These requirements may affect how the program to be tested is written. It may
for instance require that an external io_service can be provided rather than one
being wrapped in an internal thread.

However, ``boost.asio`` programs may generally benefit from being transformed to
this form, as the become *composable*, i.e. agnostic to which io_service they
run on or how many threads are running it.

features
--------

The currently (partially) supported classes are:

* chrono::high_resolution_clock
* asio::high_resolution_timer
* asio::ip::tcp::acceptor
* asio::ip::tcp::endpoint
* asio::ip::address (v4 and v6 variants, these just defer to the actual
  boost.asio types)
* asio::ip::tcp::socket
* asio::ip::udp::socket
* asio::io_service
* asio::ip::udp::resolver
* asio::ip::tcp::resolver

The ``high_resolution_clock`` in the ``chrono`` namespace implements the timer
concept from the chrono library.

usage
-----

The ``io_service`` object is significantly different from the one in boost.asio.
This is because one simulation may only have a single message loop and a single
ordering of events. This single message loop is provided by the ``simulation``
class. Each simulation should have only one such object. An ``io_service``
object represents a single node on the network. When creating an io_service, you
have to pass in the simulation it belongs to as well as the IP address it should
have. It is also possible to pass in multiple addresses to form a multi-homed
node. For instance, one with both an IPv4 and IPv6 interface.

When creating sockets, binding and connecting them, the io_service object
determines what ``INADDR_ANY`` resolves to (the first IP assigned to that node).

The only aspects of the io_service interface that's preserved are ``post()``,
``dispatch()`` and constructing timers and sockets. In short, the ``run()`` and
``poll()`` family of functions do not exist. Every io_service object is assumed
to be run, and all of their events are handled by the simulation object.

None of the synchronous APIs are supported, because that would require
integration with OS threads and scheduler.

example
-------

Here's a simple example illustrating the asio timer::

	#include "simulator/simulator.hpp"
	#include <functional>
	#include <boost/system.hpp>

	void print_time(sim::asio::high_resolution_timer& timer
		, boost::system::error_code const& ec)
	{
		using namespace sim::chrono;
		static int counter = 0;

		printf("[%d] timer fired at: %d milliseconds. error: %s\n"
			, counter
			, int(duration_cast<milliseconds>(high_resolution_clock::now()
					.time_since_epoch()).count())
			, ec.message().c_str());

		++counter;
		if (counter < 5)
		{
			timer.expires_from_now(seconds(counter));
			timer.async_wait(std::bind(&print_time, std::ref(timer), _1));
		}
	}

	int main()
	{
		using namespace sim::chrono;

		default_config cfg;
		simulation sim(cfg);
		io_service ios(sim, ip::address_v4::from_string("1.2.3.4"));
		sim::asio::high_resolution_timer timer(ios);

		timer.expires_from_now(seconds(1));
		timer.async_wait(std::bind(&print_time, std::ref(timer), _1));

		boost::system::error_code ec;
		sim.run(ec);

		printf("sim::run() returned: %s at: %d\n"
			, ec.message().c_str()
			, int(duration_cast<milliseconds>(high_resolution_clock::now()
					.time_since_epoch()).count()));
	}

The output from this program is::

	[0] timer fired at: 1000 milliseconds. error: Undefined error: 0
	[1] timer fired at: 2000 milliseconds. error: Undefined error: 0
	[2] timer fired at: 4000 milliseconds. error: Undefined error: 0
	[3] timer fired at: 7000 milliseconds. error: Undefined error: 0
	[4] timer fired at: 11000 milliseconds. error: Undefined error: 0
	io_service::run() returned: Undefined error: 0 at: 11000

And obviously it doesn't take 11 wall-clock seconds to run (it returns
instantly).

configuration
-------------

The simulated network can be configured with per-node pair bandwidth, round-trip
latency and queue sizes. This is controlled via a callback interface that
libsimulator will ask for these properties when nodes get connected.

The resolution of hostnames is also configurable by providing a callback on the
configuration object along with the latency of individual lookups.

To configure the network for the simulation, pass in a reference to an object
implementing the ``sim::configuration`` interface::

	struct configuration
	{
		// build the network
		virtual void build(simulation& sim) = 0;

		// return the hops on the network packets from src to dst need to traverse
		virtual route channel_route(asio::ip::address src
			, asio::ip::address dst) = 0;

		// return the hops an incoming packet to ep need to traverse before
		// reaching the socket (for instance a NAT)
		virtual route incoming_route(asio::ip::address ip) = 0;

		// return the hops an outgoing packet from ep need to traverse before
		// reaching the network (for instance a DSL modem)
		virtual route outgoing_route(asio::ip::address ip) = 0;

		// return the path MTU between the two IP addresses
		// For TCP sockets, this will be called once when the connection is
		// established. For UDP sockets it's called for every burst of packets
		// that are sent
		virtual int path_mtu(asio::ip::address ip1, asio::ip::address ip2) = 0;

		// called for every hostname lookup made by the client. ``reqyestor`` is
		// the node performing the lookup, ``hostname`` is the name being looked
		// up. Resolve the name into addresses and fill in ``result`` or set
		// ``ec`` if the hostname is not found or some other error occurs. The
		// return value is the latency of the lookup. The client's callback won't
		// be called until after waiting this long.
		virtual chrono::high_resolution_clock::duration hostname_lookup(
			asio::ip::address const& requestor
			, std::string hostname
			, std::vector<asio::ip::address>& result
			, boost::system::error_code& ec) = 0;
	};

``build()`` is called right after the simulation is constructed. It gives the
configuration object an opportunity to construct the core queues, since they
need access to the simulator.

``channel_route()`` is expected to return a *route* of network hops from the
source IP to the destination IP. A route is a series of ``sink`` objects. The
typical sink is a ``sim::queue``, which is a network node with a specific rate
limit, propagation delay and queue size.

*TODO: finish document configuration interface*

history
-------

libsimulator grew out of libtorrent's unit tests, as a tool to make them reliable
and deterministic (i.e. not depend on external systems like sockets and timers)
and also easier to debug. The subset of the asio API initially supported by this
library is the subset used by libtorrent. Patches are welcome to improve
fidelity and support.

