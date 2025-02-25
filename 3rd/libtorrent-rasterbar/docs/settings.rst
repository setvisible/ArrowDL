.. _user_agent:

.. raw:: html

	<a name="user_agent"></a>

+------------+--------+-------------+
| name       | type   | default     |
+============+========+=============+
| user_agent | string | libtorrent/ |
+------------+--------+-------------+

this is the client identification to the tracker. The recommended
format of this string is: "client-name/client-version
libtorrent/libtorrent-version". This name will not only be used when
making HTTP requests, but also when sending extended headers to
peers that support that extension. It may not contain \r or \n

.. _announce_ip:

.. raw:: html

	<a name="announce_ip"></a>

+-------------+--------+---------+
| name        | type   | default |
+=============+========+=========+
| announce_ip | string | nullptr |
+-------------+--------+---------+

``announce_ip`` is the ip address passed along to trackers as the
``&ip=`` parameter. If left as the default, that parameter is
omitted.

.. note::
   This setting is only meant for very special cases where a seed is
   running on the same host as the tracker, and the tracker accepts
   the IP parameter (which normal trackers don't). Do not set this
   option unless you also control the tracker.

.. _handshake_client_version:

.. raw:: html

	<a name="handshake_client_version"></a>

+--------------------------+--------+---------+
| name                     | type   | default |
+==========================+========+=========+
| handshake_client_version | string | nullptr |
+--------------------------+--------+---------+

this is the client name and version identifier sent to peers in the
handshake message. If this is an empty string, the user_agent is
used instead. This string must be a UTF-8 encoded unicode string.

.. _outgoing_interfaces:

.. raw:: html

	<a name="outgoing_interfaces"></a>

+---------------------+--------+---------+
| name                | type   | default |
+=====================+========+=========+
| outgoing_interfaces | string |         |
+---------------------+--------+---------+

This controls which IP address outgoing TCP peer connections are bound
to, in addition to controlling whether such connections are also
bound to a specific network interface/adapter (*bind-to-device*).

This string is a comma-separated list of IP addresses and
interface names. An empty string will not bind TCP sockets to a
device, and let the network stack assign the local address.

A list of names will be used to bind outgoing TCP sockets in a
round-robin fashion. An IP address will simply be used to `bind()`
the socket. An interface name will attempt to bind the socket to
that interface. If that fails, or is unsupported, one of the IP
addresses configured for that interface is used to `bind()` the
socket to. If the interface or adapter doesn't exist, the
outgoing peer connection will fail with an error message suggesting
the device cannot be found. Adapter names on Unix systems are of
the form "eth0", "eth1", "tun0", etc. This may be useful for
clients that are multi-homed. Binding an outgoing connection to a
local IP does not necessarily make the connection via the
associated NIC/Adapter.

When outgoing interfaces are specified, incoming connections or
packets sent to a local interface or IP that's *not* in this list
will be rejected with a peer_blocked_alert with
``invalid_local_interface`` as the reason.

Note that these are just interface/adapter names or IP addresses.
There are no ports specified in this list. IPv6 addresses without
port should be specified without enclosing ``[``, ``]``.

.. _listen_interfaces:

.. raw:: html

	<a name="listen_interfaces"></a>

+-------------------+--------+------------------------+
| name              | type   | default                |
+===================+========+========================+
| listen_interfaces | string | 0.0.0.0:6881,[::]:6881 |
+-------------------+--------+------------------------+

a comma-separated list of (IP or device name, port) pairs. These are
the listen ports that will be opened for accepting incoming uTP and
TCP peer connections. These are also used for *outgoing* uTP and UDP
tracker connections and DHT nodes.

It is possible to listen on multiple interfaces and
multiple ports. Binding to port 0 will make the operating system
pick the port.

.. note::
   There are reasons to stick to the same port across sessions,
   which would mean only using port 0 on the first start, and
   recording the port that was picked for subsequent startups.
   Trackers, the DHT and other peers will remember the port they see
   you use and hand that port out to other peers trying to connect
   to you, as well as trying to connect to you themselves.

A port that has an "s" suffix will accept SSL peer connections. (note
that SSL sockets are only available in builds with SSL support)

A port that has an "l" suffix will be considered a local network.
i.e. it's assumed to only be able to reach hosts in the same local
network as the IP address (based on the netmask associated with the
IP, queried from the operating system).

if binding fails, the listen_failed_alert is posted. Once a
socket binding succeeds (if it does), the listen_succeeded_alert
is posted. There may be multiple failures before a success.

If a device name that does not exist is configured, no listen
socket will be opened for that interface. If this is the only
interface configured, it will be as if no listen ports are
configured.

If no listen ports are configured (e.g. listen_interfaces is an
empty string), networking will be disabled. No DHT will start, no
outgoing uTP or tracker connections will be made. No incoming TCP
or uTP connections will be accepted. (outgoing TCP connections
will still be possible, depending on
settings_pack::outgoing_interfaces).

For example:
``[::1]:8888`` - will only accept connections on the IPv6 loopback
address on port 8888.

``eth0:4444,eth1:4444`` - will accept connections on port 4444 on
any IP address bound to device ``eth0`` or ``eth1``.

``[::]:0s`` - will accept SSL connections on a port chosen by the
OS. And not accept non-SSL connections at all.

``0.0.0.0:6881,[::]:6881`` - binds to all interfaces on port 6881.

``10.0.1.13:6881l`` - binds to the local IP address, port 6881, but
only allow talking to peers on the same local network. The netmask
is queried from the operating system. Interfaces marked ``l`` are
not announced to trackers, unless the tracker is also on the same
local network.

Windows OS network adapter device name must be specified with GUID.
It can be obtained from "netsh lan show interfaces" command output.
GUID must be uppercased string embraced in curly brackets.
``{E4F0B674-0DFC-48BB-98A5-2AA730BDB6D6}:7777`` - will accept
connections on port 7777 on adapter with this GUID.

For more information, see the `Multi-homed hosts`_ section.

.. _`Multi-homed hosts`: manual-ref.html#multi-homed-hosts

.. _proxy_hostname:

.. raw:: html

	<a name="proxy_hostname"></a>

+----------------+--------+---------+
| name           | type   | default |
+================+========+=========+
| proxy_hostname | string |         |
+----------------+--------+---------+

when using a proxy, this is the hostname where the proxy is running
see proxy_type. Note that when using a proxy, the
settings_pack::listen_interfaces setting is overridden and only a
single interface is created, just to contact the proxy. This
means a proxy cannot be combined with SSL torrents or multiple
listen interfaces. This proxy listen interface will not accept
incoming TCP connections, will not map ports with any gateway and
will not enable local service discovery. All traffic is supposed
to be channeled through the proxy.

.. _proxy_username:

.. _proxy_password:

.. raw:: html

	<a name="proxy_username"></a>
	<a name="proxy_password"></a>

+----------------+--------+---------+
| name           | type   | default |
+================+========+=========+
| proxy_username | string |         |
+----------------+--------+---------+
| proxy_password | string |         |
+----------------+--------+---------+

when using a proxy, these are the credentials (if any) to use when
connecting to it. see proxy_type

.. _i2p_hostname:

.. raw:: html

	<a name="i2p_hostname"></a>

+--------------+--------+---------+
| name         | type   | default |
+==============+========+=========+
| i2p_hostname | string |         |
+--------------+--------+---------+

sets the i2p_ SAM bridge to connect to. set the port with the
``i2p_port`` setting. Unless this is set, i2p torrents are not
supported. This setting is separate from the other proxy settings
since i2p torrents and their peers are orthogonal. You can have
i2p peers as well as regular peers via a proxy.

.. _i2p: http://www.i2p2.de

.. _peer_fingerprint:

.. raw:: html

	<a name="peer_fingerprint"></a>

+------------------+--------+----------+
| name             | type   | default  |
+==================+========+==========+
| peer_fingerprint | string | -LT20B0- |
+------------------+--------+----------+

this is the fingerprint for the client. It will be used as the
prefix to the peer_id. If this is 20 bytes (or longer) it will be
truncated to 20 bytes and used as the entire peer-id

There is a utility function, generate_fingerprint() that can be used
to generate a standard client peer ID fingerprint prefix.

.. _dht_bootstrap_nodes:

.. raw:: html

	<a name="dht_bootstrap_nodes"></a>

+---------------------+--------+--------------------------+
| name                | type   | default                  |
+=====================+========+==========================+
| dht_bootstrap_nodes | string | dht.libtorrent.org:25401 |
+---------------------+--------+--------------------------+

This is a comma-separated list of IP port-pairs. They will be added
to the DHT node (if it's enabled) as back-up nodes in case we don't
know of any.

Changing these after the DHT has been started may not have any
effect until the DHT is restarted.
Here are some other bootstrap nodes that may work:
``router.bittorrent.com:6881``,
``dht.transmissionbt.com:6881``
``router.bt.ouinet.work:6881``,

.. _allow_multiple_connections_per_ip:

.. raw:: html

	<a name="allow_multiple_connections_per_ip"></a>

+-----------------------------------+------+---------+
| name                              | type | default |
+===================================+======+=========+
| allow_multiple_connections_per_ip | bool | false   |
+-----------------------------------+------+---------+

determines if connections from the same IP address as existing
connections should be rejected or not. Rejecting multiple connections
from the same IP address will prevent abusive
behavior by peers. The logic for determining whether connections are
to the same peer is more complicated with this enabled, and more
likely to fail in some edge cases. It is not recommended to enable
this feature.

.. _send_redundant_have:

.. raw:: html

	<a name="send_redundant_have"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| send_redundant_have | bool | true    |
+---------------------+------+---------+

``send_redundant_have`` controls if have messages will be sent to
peers that already have the piece. This is typically not necessary,
but it might be necessary for collecting statistics in some cases.

.. _use_dht_as_fallback:

.. raw:: html

	<a name="use_dht_as_fallback"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| use_dht_as_fallback | bool | false   |
+---------------------+------+---------+

``use_dht_as_fallback`` determines how the DHT is used. If this is
true, the DHT will only be used for torrents where all trackers in
its tracker list has failed. Either by an explicit error message or
a time out. If this is false, the DHT is used regardless of if the
trackers fail or not.

.. _upnp_ignore_nonrouters:

.. raw:: html

	<a name="upnp_ignore_nonrouters"></a>

+------------------------+------+---------+
| name                   | type | default |
+========================+======+=========+
| upnp_ignore_nonrouters | bool | false   |
+------------------------+------+---------+

``upnp_ignore_nonrouters`` indicates whether or not the UPnP
implementation should ignore any broadcast response from a device
whose address is not on our subnet. i.e.
it's a way to not talk to other people's routers by mistake.

.. _use_parole_mode:

.. raw:: html

	<a name="use_parole_mode"></a>

+-----------------+------+---------+
| name            | type | default |
+=================+======+=========+
| use_parole_mode | bool | true    |
+-----------------+------+---------+

``use_parole_mode`` specifies if parole mode should be used. Parole
mode means that peers that participate in pieces that fail the hash
check are put in a mode where they are only allowed to download
whole pieces. If the whole piece a peer in parole mode fails the
hash check, it is banned. If a peer participates in a piece that
passes the hash check, it is taken out of parole mode.

.. _auto_manage_prefer_seeds:

.. raw:: html

	<a name="auto_manage_prefer_seeds"></a>

+--------------------------+------+---------+
| name                     | type | default |
+==========================+======+=========+
| auto_manage_prefer_seeds | bool | false   |
+--------------------------+------+---------+

if true, prefer seeding torrents when determining which torrents to give
active slots to. If false, give preference to downloading torrents

.. _dont_count_slow_torrents:

.. raw:: html

	<a name="dont_count_slow_torrents"></a>

+--------------------------+------+---------+
| name                     | type | default |
+==========================+======+=========+
| dont_count_slow_torrents | bool | true    |
+--------------------------+------+---------+

if ``dont_count_slow_torrents`` is true, torrents without any
payload transfers are not subject to the ``active_seeds`` and
``active_downloads`` limits. This is intended to make it more
likely to utilize all available bandwidth, and avoid having
torrents that don't transfer anything block the active slots.

.. _close_redundant_connections:

.. raw:: html

	<a name="close_redundant_connections"></a>

+-----------------------------+------+---------+
| name                        | type | default |
+=============================+======+=========+
| close_redundant_connections | bool | true    |
+-----------------------------+------+---------+

``close_redundant_connections`` specifies whether libtorrent should
close connections where both ends have no utility in keeping the
connection open. For instance if both ends have completed their
downloads, there's no point in keeping it open.

.. _prioritize_partial_pieces:

.. raw:: html

	<a name="prioritize_partial_pieces"></a>

+---------------------------+------+---------+
| name                      | type | default |
+===========================+======+=========+
| prioritize_partial_pieces | bool | false   |
+---------------------------+------+---------+

If ``prioritize_partial_pieces`` is true, partial pieces are picked
before pieces that are more rare. If false, rare pieces are always
prioritized, unless the number of partial pieces is growing out of
proportion.

.. _rate_limit_ip_overhead:

.. raw:: html

	<a name="rate_limit_ip_overhead"></a>

+------------------------+------+---------+
| name                   | type | default |
+========================+======+=========+
| rate_limit_ip_overhead | bool | true    |
+------------------------+------+---------+

if set to true, the estimated TCP/IP overhead is drained from the
rate limiters, to avoid exceeding the limits with the total traffic

.. _announce_to_all_tiers:

.. _announce_to_all_trackers:

.. raw:: html

	<a name="announce_to_all_tiers"></a>
	<a name="announce_to_all_trackers"></a>

+--------------------------+------+---------+
| name                     | type | default |
+==========================+======+=========+
| announce_to_all_tiers    | bool | false   |
+--------------------------+------+---------+
| announce_to_all_trackers | bool | false   |
+--------------------------+------+---------+

``announce_to_all_trackers`` controls how multi tracker torrents
are treated. If this is set to true, all trackers in the same tier
are announced to in parallel. If all trackers in tier 0 fails, all
trackers in tier 1 are announced as well. If it's set to false, the
behavior is as defined by the multi tracker specification.

``announce_to_all_tiers`` also controls how multi tracker torrents
are treated. When this is set to true, one tracker from each tier
is announced to. This is the uTorrent behavior. To be compliant
with the Multi-tracker specification, set it to false.

.. _prefer_udp_trackers:

.. raw:: html

	<a name="prefer_udp_trackers"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| prefer_udp_trackers | bool | true    |
+---------------------+------+---------+

``prefer_udp_trackers``: true means that trackers
may be rearranged in a way that udp trackers are always tried
before http trackers for the same hostname. Setting this to false
means that the tracker's tier is respected and there's no
preference of one protocol over another.

.. _disable_hash_checks:

.. raw:: html

	<a name="disable_hash_checks"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| disable_hash_checks | bool | false   |
+---------------------+------+---------+

when set to true, all data downloaded from peers will be assumed to
be correct, and not tested to match the hashes in the torrent this
is only useful for simulation and testing purposes (typically
combined with disabled_storage)

.. _allow_i2p_mixed:

.. raw:: html

	<a name="allow_i2p_mixed"></a>

+-----------------+------+---------+
| name            | type | default |
+=================+======+=========+
| allow_i2p_mixed | bool | false   |
+-----------------+------+---------+

if this is true, i2p torrents are allowed to also get peers from
other sources than the tracker, and connect to regular IPs, not
providing any anonymization. This may be useful if the user is not
interested in the anonymization of i2p, but still wants to be able
to connect to i2p peers.

.. _no_atime_storage:

.. raw:: html

	<a name="no_atime_storage"></a>

+------------------+------+---------+
| name             | type | default |
+==================+======+=========+
| no_atime_storage | bool | true    |
+------------------+------+---------+

``no_atime_storage`` this is a Linux-only option and passes in the
``O_NOATIME`` to ``open()`` when opening files. This may lead to
some disk performance improvements.

.. _incoming_starts_queued_torrents:

.. raw:: html

	<a name="incoming_starts_queued_torrents"></a>

+---------------------------------+------+---------+
| name                            | type | default |
+=================================+======+=========+
| incoming_starts_queued_torrents | bool | false   |
+---------------------------------+------+---------+

``incoming_starts_queued_torrents``.  If a torrent
has been paused by the auto managed feature in libtorrent, i.e. the
torrent is paused and auto managed, this feature affects whether or
not it is automatically started on an incoming connection. The main
reason to queue torrents, is not to make them unavailable, but to
save on the overhead of announcing to the trackers, the DHT and to
avoid spreading one's unchoke slots too thin. If a peer managed to
find us, even though we're no in the torrent anymore, this setting
can make us start the torrent and serve it.

.. _report_true_downloaded:

.. raw:: html

	<a name="report_true_downloaded"></a>

+------------------------+------+---------+
| name                   | type | default |
+========================+======+=========+
| report_true_downloaded | bool | false   |
+------------------------+------+---------+

when set to true, the downloaded counter sent to trackers will
include the actual number of payload bytes downloaded including
redundant bytes. If set to false, it will not include any redundancy
bytes

.. _strict_end_game_mode:

.. raw:: html

	<a name="strict_end_game_mode"></a>

+----------------------+------+---------+
| name                 | type | default |
+======================+======+=========+
| strict_end_game_mode | bool | true    |
+----------------------+------+---------+

``strict_end_game_mode`` controls when a
block may be requested twice. If this is ``true``, a block may only
be requested twice when there's at least one request to every piece
that's left to download in the torrent. This may slow down progress
on some pieces sometimes, but it may also avoid downloading a lot
of redundant bytes. If this is ``false``, libtorrent attempts to
use each peer connection to its max, by always requesting
something, even if it means requesting something that has been
requested from another peer already.

.. _enable_outgoing_utp:

.. _enable_incoming_utp:

.. _enable_outgoing_tcp:

.. _enable_incoming_tcp:

.. raw:: html

	<a name="enable_outgoing_utp"></a>
	<a name="enable_incoming_utp"></a>
	<a name="enable_outgoing_tcp"></a>
	<a name="enable_incoming_tcp"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| enable_outgoing_utp | bool | true    |
+---------------------+------+---------+
| enable_incoming_utp | bool | true    |
+---------------------+------+---------+
| enable_outgoing_tcp | bool | true    |
+---------------------+------+---------+
| enable_incoming_tcp | bool | true    |
+---------------------+------+---------+

Enables incoming and outgoing, TCP and uTP peer connections.
``false`` is disabled and ``true`` is enabled. When outgoing
connections are disabled, libtorrent will simply not make
outgoing peer connections with the specific transport protocol.
Disabled incoming peer connections will simply be rejected.
These options only apply to peer connections, not tracker- or any
other kinds of connections.

.. _no_recheck_incomplete_resume:

.. raw:: html

	<a name="no_recheck_incomplete_resume"></a>

+------------------------------+------+---------+
| name                         | type | default |
+==============================+======+=========+
| no_recheck_incomplete_resume | bool | false   |
+------------------------------+------+---------+

``no_recheck_incomplete_resume`` determines if the storage should
check the whole files when resume data is incomplete or missing or
whether it should simply assume we don't have any of the data. If
false, any existing files will be checked.
By setting this setting to true, the files won't be checked, but
will go straight to download mode.

.. _anonymous_mode:

.. raw:: html

	<a name="anonymous_mode"></a>

+----------------+------+---------+
| name           | type | default |
+================+======+=========+
| anonymous_mode | bool | false   |
+----------------+------+---------+

``anonymous_mode``: When set to true, the client tries to hide
its identity to a certain degree.

* A generic user-agent will be
  used for trackers (except for private torrents).
* Your local IPv4 and IPv6 address won't be sent as query string
  parameters to private trackers.
* If announce_ip is configured, it will not be sent to trackers
* The client version will not be sent to peers in the extension
  handshake.

.. _report_web_seed_downloads:

.. raw:: html

	<a name="report_web_seed_downloads"></a>

+---------------------------+------+---------+
| name                      | type | default |
+===========================+======+=========+
| report_web_seed_downloads | bool | true    |
+---------------------------+------+---------+

specifies whether downloads from web seeds is reported to the
tracker or not. Turning it off also excludes web
seed traffic from other stats and download rate reporting via the
libtorrent API.

.. _seeding_outgoing_connections:

.. raw:: html

	<a name="seeding_outgoing_connections"></a>

+------------------------------+------+---------+
| name                         | type | default |
+==============================+======+=========+
| seeding_outgoing_connections | bool | true    |
+------------------------------+------+---------+

``seeding_outgoing_connections`` determines if seeding (and
finished) torrents should attempt to make outgoing connections or
not. It may be set to false in very
specific applications where the cost of making outgoing connections
is high, and there are no or small benefits of doing so. For
instance, if no nodes are behind a firewall or a NAT, seeds don't
need to make outgoing connections.

.. _no_connect_privileged_ports:

.. raw:: html

	<a name="no_connect_privileged_ports"></a>

+-----------------------------+------+---------+
| name                        | type | default |
+=============================+======+=========+
| no_connect_privileged_ports | bool | false   |
+-----------------------------+------+---------+

when this is true, libtorrent will not attempt to make outgoing
connections to peers whose port is < 1024. This is a safety
precaution to avoid being part of a DDoS attack

.. _smooth_connects:

.. raw:: html

	<a name="smooth_connects"></a>

+-----------------+------+---------+
| name            | type | default |
+=================+======+=========+
| smooth_connects | bool | true    |
+-----------------+------+---------+

``smooth_connects`` means the number of
connection attempts per second may be limited to below the
``connection_speed``, in case we're close to bump up against the
limit of number of connections. The intention of this setting is to
more evenly distribute our connection attempts over time, instead
of attempting to connect in batches, and timing them out in
batches.

.. _always_send_user_agent:

.. raw:: html

	<a name="always_send_user_agent"></a>

+------------------------+------+---------+
| name                   | type | default |
+========================+======+=========+
| always_send_user_agent | bool | false   |
+------------------------+------+---------+

always send user-agent in every web seed request. If false, only
the first request per http connection will include the user agent

.. _apply_ip_filter_to_trackers:

.. raw:: html

	<a name="apply_ip_filter_to_trackers"></a>

+-----------------------------+------+---------+
| name                        | type | default |
+=============================+======+=========+
| apply_ip_filter_to_trackers | bool | true    |
+-----------------------------+------+---------+

``apply_ip_filter_to_trackers`` determines
whether the IP filter applies to trackers as well as peers. If this
is set to false, trackers are exempt from the IP filter (if there
is one). If no IP filter is set, this setting is irrelevant.

.. _ban_web_seeds:

.. raw:: html

	<a name="ban_web_seeds"></a>

+---------------+------+---------+
| name          | type | default |
+===============+======+=========+
| ban_web_seeds | bool | true    |
+---------------+------+---------+

when true, web seeds sending bad data will be banned

.. _support_share_mode:

.. raw:: html

	<a name="support_share_mode"></a>

+--------------------+------+---------+
| name               | type | default |
+====================+======+=========+
| support_share_mode | bool | true    |
+--------------------+------+---------+

if false, prevents libtorrent to advertise share-mode support

.. _report_redundant_bytes:

.. raw:: html

	<a name="report_redundant_bytes"></a>

+------------------------+------+---------+
| name                   | type | default |
+========================+======+=========+
| report_redundant_bytes | bool | true    |
+------------------------+------+---------+

if this is true, the number of redundant bytes is sent to the
tracker

.. _listen_system_port_fallback:

.. raw:: html

	<a name="listen_system_port_fallback"></a>

+-----------------------------+------+---------+
| name                        | type | default |
+=============================+======+=========+
| listen_system_port_fallback | bool | true    |
+-----------------------------+------+---------+

if this is true, libtorrent will fall back to listening on a port
chosen by the operating system (i.e. binding to port 0). If a
failure is preferred, set this to false.

.. _announce_crypto_support:

.. raw:: html

	<a name="announce_crypto_support"></a>

+-------------------------+------+---------+
| name                    | type | default |
+=========================+======+=========+
| announce_crypto_support | bool | true    |
+-------------------------+------+---------+

when this is true, and incoming encrypted connections are enabled,
&supportcrypt=1 is included in http tracker announces

.. _enable_upnp:

.. raw:: html

	<a name="enable_upnp"></a>

+-------------+------+---------+
| name        | type | default |
+=============+======+=========+
| enable_upnp | bool | true    |
+-------------+------+---------+

Starts and stops the UPnP service. When started, the listen port
and the DHT port are attempted to be forwarded on local UPnP router
devices.

The upnp object returned by ``start_upnp()`` can be used to add and
remove arbitrary port mappings. Mapping status is returned through
the portmap_alert and the portmap_error_alert. The object will be
valid until ``stop_upnp()`` is called. See upnp-and-nat-pmp_.

.. _enable_natpmp:

.. raw:: html

	<a name="enable_natpmp"></a>

+---------------+------+---------+
| name          | type | default |
+===============+======+=========+
| enable_natpmp | bool | true    |
+---------------+------+---------+

Starts and stops the NAT-PMP service. When started, the listen port
and the DHT port are attempted to be forwarded on the router
through NAT-PMP.

The natpmp object returned by ``start_natpmp()`` can be used to add
and remove arbitrary port mappings. Mapping status is returned
through the portmap_alert and the portmap_error_alert. The object
will be valid until ``stop_natpmp()`` is called. See
upnp-and-nat-pmp_.

.. _enable_lsd:

.. raw:: html

	<a name="enable_lsd"></a>

+------------+------+---------+
| name       | type | default |
+============+======+=========+
| enable_lsd | bool | true    |
+------------+------+---------+

Starts and stops Local Service Discovery. This service will
broadcast the info-hashes of all the non-private torrents on the
local network to look for peers on the same swarm within multicast
reach.

.. _enable_dht:

.. raw:: html

	<a name="enable_dht"></a>

+------------+------+---------+
| name       | type | default |
+============+======+=========+
| enable_dht | bool | true    |
+------------+------+---------+

starts the dht node and makes the trackerless service available to
torrents.

.. _prefer_rc4:

.. raw:: html

	<a name="prefer_rc4"></a>

+------------+------+---------+
| name       | type | default |
+============+======+=========+
| prefer_rc4 | bool | false   |
+------------+------+---------+

if the allowed encryption level is both, setting this to true will
prefer RC4 if both methods are offered, plain text otherwise

.. _proxy_hostnames:

.. raw:: html

	<a name="proxy_hostnames"></a>

+-----------------+------+---------+
| name            | type | default |
+=================+======+=========+
| proxy_hostnames | bool | true    |
+-----------------+------+---------+

if true, hostname lookups are done via the configured proxy (if
any). This is only supported by SOCKS5 and HTTP.

.. _proxy_peer_connections:

.. raw:: html

	<a name="proxy_peer_connections"></a>

+------------------------+------+---------+
| name                   | type | default |
+========================+======+=========+
| proxy_peer_connections | bool | true    |
+------------------------+------+---------+

if true, peer connections are made (and accepted) over the
configured proxy, if any. Web seeds as well as regular bittorrent
peer connections are considered "peer connections". Anything
transporting actual torrent payload (trackers and DHT traffic are
not considered peer connections).

.. _auto_sequential:

.. raw:: html

	<a name="auto_sequential"></a>

+-----------------+------+---------+
| name            | type | default |
+=================+======+=========+
| auto_sequential | bool | true    |
+-----------------+------+---------+

if this setting is true, torrents with a very high availability of
pieces (and seeds) are downloaded sequentially. This is more
efficient for the disk I/O. With many seeds, the download order is
unlikely to matter anyway

.. _proxy_tracker_connections:

.. raw:: html

	<a name="proxy_tracker_connections"></a>

+---------------------------+------+---------+
| name                      | type | default |
+===========================+======+=========+
| proxy_tracker_connections | bool | true    |
+---------------------------+------+---------+

if true, tracker connections are made over the configured proxy, if
any.

.. _enable_ip_notifier:

.. raw:: html

	<a name="enable_ip_notifier"></a>

+--------------------+------+---------+
| name               | type | default |
+====================+======+=========+
| enable_ip_notifier | bool | true    |
+--------------------+------+---------+

Starts and stops the internal IP table route changes notifier.

The current implementation supports multiple platforms, and it is
recommended to have it enable, but you may want to disable it if
it's supported but unreliable, or if you have a better way to
detect the changes. In the later case, you should manually call
``session_handle::reopen_network_sockets`` to ensure network
changes are taken in consideration.

.. _dht_prefer_verified_node_ids:

.. raw:: html

	<a name="dht_prefer_verified_node_ids"></a>

+------------------------------+------+---------+
| name                         | type | default |
+==============================+======+=========+
| dht_prefer_verified_node_ids | bool | true    |
+------------------------------+------+---------+

when this is true, nodes whose IDs are derived from their source
IP according to `BEP 42`_ are preferred in the routing table.

.. _dht_restrict_routing_ips:

.. raw:: html

	<a name="dht_restrict_routing_ips"></a>

+--------------------------+------+---------+
| name                     | type | default |
+==========================+======+=========+
| dht_restrict_routing_ips | bool | true    |
+--------------------------+------+---------+

determines if the routing table entries should restrict entries to one
per IP. This defaults to true, which helps mitigate some attacks on
the DHT. It prevents adding multiple nodes with IPs with a very close
CIDR distance.

when set, nodes whose IP address that's in the same /24 (or /64 for
IPv6) range in the same routing table bucket. This is an attempt to
mitigate node ID spoofing attacks also restrict any IP to only have a
single entry in the whole routing table

.. _dht_restrict_search_ips:

.. raw:: html

	<a name="dht_restrict_search_ips"></a>

+-------------------------+------+---------+
| name                    | type | default |
+=========================+======+=========+
| dht_restrict_search_ips | bool | true    |
+-------------------------+------+---------+

determines if DHT searches should prevent adding nodes with IPs with
very close CIDR distance. This also defaults to true and helps
mitigate certain attacks on the DHT.

.. _dht_extended_routing_table:

.. raw:: html

	<a name="dht_extended_routing_table"></a>

+----------------------------+------+---------+
| name                       | type | default |
+============================+======+=========+
| dht_extended_routing_table | bool | true    |
+----------------------------+------+---------+

makes the first buckets in the DHT routing table fit 128, 64, 32 and
16 nodes respectively, as opposed to the standard size of 8. All other
buckets have size 8 still.

.. _dht_aggressive_lookups:

.. raw:: html

	<a name="dht_aggressive_lookups"></a>

+------------------------+------+---------+
| name                   | type | default |
+========================+======+=========+
| dht_aggressive_lookups | bool | true    |
+------------------------+------+---------+

slightly changes the lookup behavior in terms of how many outstanding
requests we keep. Instead of having branch factor be a hard limit, we
always keep *branch factor* outstanding requests to the closest nodes.
i.e. every time we get results back with closer nodes, we query them
right away. It lowers the lookup times at the cost of more outstanding
queries.

.. _dht_privacy_lookups:

.. raw:: html

	<a name="dht_privacy_lookups"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| dht_privacy_lookups | bool | false   |
+---------------------+------+---------+

when set, perform lookups in a way that is slightly more expensive,
but which minimizes the amount of information leaked about you.

.. _dht_enforce_node_id:

.. raw:: html

	<a name="dht_enforce_node_id"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| dht_enforce_node_id | bool | false   |
+---------------------+------+---------+

when set, node's whose IDs that are not correctly generated based on
its external IP are ignored. When a query arrives from such node, an
error message is returned with a message saying "invalid node ID".

.. _dht_ignore_dark_internet:

.. raw:: html

	<a name="dht_ignore_dark_internet"></a>

+--------------------------+------+---------+
| name                     | type | default |
+==========================+======+=========+
| dht_ignore_dark_internet | bool | true    |
+--------------------------+------+---------+

ignore DHT messages from parts of the internet we wouldn't expect to
see any traffic from

.. _dht_read_only:

.. raw:: html

	<a name="dht_read_only"></a>

+---------------+------+---------+
| name          | type | default |
+===============+======+=========+
| dht_read_only | bool | false   |
+---------------+------+---------+

when set, the other nodes won't keep this node in their routing
tables, it's meant for low-power and/or ephemeral devices that
cannot support the DHT, it is also useful for mobile devices which
are sensitive to network traffic and battery life.
this node no longer responds to 'query' messages, and will place a
'ro' key (value = 1) in the top-level message dictionary of outgoing
query messages.

.. _piece_extent_affinity:

.. raw:: html

	<a name="piece_extent_affinity"></a>

+-----------------------+------+---------+
| name                  | type | default |
+=======================+======+=========+
| piece_extent_affinity | bool | false   |
+-----------------------+------+---------+

when this is true, create an affinity for downloading 4 MiB extents
of adjacent pieces. This is an attempt to achieve better disk I/O
throughput by downloading larger extents of bytes, for torrents with
small piece sizes

.. _validate_https_trackers:

.. raw:: html

	<a name="validate_https_trackers"></a>

+-------------------------+------+---------+
| name                    | type | default |
+=========================+======+=========+
| validate_https_trackers | bool | true    |
+-------------------------+------+---------+

when set to true, the certificate of HTTPS trackers and HTTPS web
seeds will be validated against the system's certificate store
(as defined by OpenSSL). If the system does not have a
certificate store, this option may have to be disabled in order
to get trackers and web seeds to work).

.. _ssrf_mitigation:

.. raw:: html

	<a name="ssrf_mitigation"></a>

+-----------------+------+---------+
| name            | type | default |
+=================+======+=========+
| ssrf_mitigation | bool | true    |
+-----------------+------+---------+

when enabled, tracker and web seed requests are subject to
certain restrictions.

An HTTP(s) tracker requests to localhost (loopback)
must have the request path start with "/announce". This is the
conventional bittorrent tracker request. Any other HTTP(S)
tracker request to loopback will be rejected. This applies to
trackers that redirect to loopback as well.

Web seeds that end up on the client's local network (i.e. in a
private IP address range) may not include query string arguments.
This applies to web seeds redirecting to the local network as
well.

Web seeds on global IPs (i.e. not local network) may not redirect
to a local network address

.. _allow_idna:

.. raw:: html

	<a name="allow_idna"></a>

+------------+------+---------+
| name       | type | default |
+============+======+=========+
| allow_idna | bool | false   |
+------------+------+---------+

when disabled, any tracker or web seed with an IDNA hostname
(internationalized domain name) is ignored. This is a security
precaution to avoid various unicode encoding attacks that might
happen at the application level.

.. _enable_set_file_valid_data:

.. raw:: html

	<a name="enable_set_file_valid_data"></a>

+----------------------------+------+---------+
| name                       | type | default |
+============================+======+=========+
| enable_set_file_valid_data | bool | false   |
+----------------------------+------+---------+

when set to true, enables the attempt to use SetFileValidData()
to pre-allocate disk space. This system call will only work when
running with Administrator privileges on Windows, and so this
setting is only relevant in that scenario. Using
SetFileValidData() poses a security risk, as it may reveal
previously deleted information from the disk.

.. _socks5_udp_send_local_ep:

.. raw:: html

	<a name="socks5_udp_send_local_ep"></a>

+--------------------------+------+---------+
| name                     | type | default |
+==========================+======+=========+
| socks5_udp_send_local_ep | bool | false   |
+--------------------------+------+---------+

When using a SOCKS5 proxy, UDP traffic is routed through the
proxy by sending a UDP ASSOCIATE command. If this option is true,
the UDP ASSOCIATE command will include the IP address and
listen port to the local UDP socket. This indicates to the proxy
which source endpoint to expect our packets from. The benefit is
that incoming packets can be forwarded correctly, before any
outgoing packets are sent. The risk is that if there's a NAT
between the client and the proxy, the IP address specified in the
protocol may not be valid from the proxy's point of view.

.. _tracker_completion_timeout:

.. raw:: html

	<a name="tracker_completion_timeout"></a>

+----------------------------+------+---------+
| name                       | type | default |
+============================+======+=========+
| tracker_completion_timeout | int  | 30      |
+----------------------------+------+---------+

``tracker_completion_timeout`` is the number of seconds the tracker
connection will wait from when it sent the request until it
considers the tracker to have timed-out.

.. _tracker_receive_timeout:

.. raw:: html

	<a name="tracker_receive_timeout"></a>

+-------------------------+------+---------+
| name                    | type | default |
+=========================+======+=========+
| tracker_receive_timeout | int  | 10      |
+-------------------------+------+---------+

``tracker_receive_timeout`` is the number of seconds to wait to
receive any data from the tracker. If no data is received for this
number of seconds, the tracker will be considered as having timed
out. If a tracker is down, this is the kind of timeout that will
occur.

.. _stop_tracker_timeout:

.. raw:: html

	<a name="stop_tracker_timeout"></a>

+----------------------+------+---------+
| name                 | type | default |
+======================+======+=========+
| stop_tracker_timeout | int  | 5       |
+----------------------+------+---------+

``stop_tracker_timeout`` is the number of seconds to wait when
sending a stopped message before considering a tracker to have
timed out. This is usually shorter, to make the client quit faster.
If the value is set to 0, the connections to trackers with the
stopped event are suppressed.

.. _tracker_maximum_response_length:

.. raw:: html

	<a name="tracker_maximum_response_length"></a>

+---------------------------------+------+-----------+
| name                            | type | default   |
+=================================+======+===========+
| tracker_maximum_response_length | int  | 1024*1024 |
+---------------------------------+------+-----------+

this is the maximum number of bytes in a tracker response. If a
response size passes this number of bytes it will be rejected and
the connection will be closed. On gzipped responses this size is
measured on the uncompressed data. So, if you get 20 bytes of gzip
response that'll expand to 2 megabytes, it will be interrupted
before the entire response has been uncompressed (assuming the
limit is lower than 2 MiB).

.. _piece_timeout:

.. raw:: html

	<a name="piece_timeout"></a>

+---------------+------+---------+
| name          | type | default |
+===============+======+=========+
| piece_timeout | int  | 20      |
+---------------+------+---------+

the number of seconds from a request is sent until it times out if
no piece response is returned.

.. _request_timeout:

.. raw:: html

	<a name="request_timeout"></a>

+-----------------+------+---------+
| name            | type | default |
+=================+======+=========+
| request_timeout | int  | 60      |
+-----------------+------+---------+

the number of seconds one block (16 kiB) is expected to be received
within. If it's not, the block is requested from a different peer

.. _request_queue_time:

.. raw:: html

	<a name="request_queue_time"></a>

+--------------------+------+---------+
| name               | type | default |
+====================+======+=========+
| request_queue_time | int  | 3       |
+--------------------+------+---------+

the length of the request queue given in the number of seconds it
should take for the other end to send all the pieces. i.e. the
actual number of requests depends on the download rate and this
number.

.. _max_allowed_in_request_queue:

.. raw:: html

	<a name="max_allowed_in_request_queue"></a>

+------------------------------+------+---------+
| name                         | type | default |
+==============================+======+=========+
| max_allowed_in_request_queue | int  | 2000    |
+------------------------------+------+---------+

the number of outstanding block requests a peer is allowed to queue
up in the client. If a peer sends more requests than this (before
the first one has been sent) the last request will be dropped. the
higher this is, the faster upload speeds the client can get to a
single peer.

.. _max_out_request_queue:

.. raw:: html

	<a name="max_out_request_queue"></a>

+-----------------------+------+---------+
| name                  | type | default |
+=======================+======+=========+
| max_out_request_queue | int  | 500     |
+-----------------------+------+---------+

``max_out_request_queue`` is the maximum number of outstanding
requests to send to a peer. This limit takes precedence over
``request_queue_time``. i.e. no matter the download speed, the
number of outstanding requests will never exceed this limit.

.. _whole_pieces_threshold:

.. raw:: html

	<a name="whole_pieces_threshold"></a>

+------------------------+------+---------+
| name                   | type | default |
+========================+======+=========+
| whole_pieces_threshold | int  | 20      |
+------------------------+------+---------+

if a whole piece can be downloaded in this number of seconds, or
less, the peer_connection will prefer to request whole pieces at a
time from this peer. The benefit of this is to better utilize disk
caches by doing localized accesses and also to make it easier to
identify bad peers if a piece fails the hash check.

.. _peer_timeout:

.. raw:: html

	<a name="peer_timeout"></a>

+--------------+------+---------+
| name         | type | default |
+==============+======+=========+
| peer_timeout | int  | 120     |
+--------------+------+---------+

``peer_timeout`` is the number of seconds the peer connection
should wait (for any activity on the peer connection) before
closing it due to time out. 120 seconds is
specified in the protocol specification. After half
the time out, a keep alive message is sent.

.. _urlseed_timeout:

.. raw:: html

	<a name="urlseed_timeout"></a>

+-----------------+------+---------+
| name            | type | default |
+=================+======+=========+
| urlseed_timeout | int  | 20      |
+-----------------+------+---------+

same as peer_timeout, but only applies to url-seeds. this is
usually set lower, because web servers are expected to be more
reliable.

.. _urlseed_pipeline_size:

.. raw:: html

	<a name="urlseed_pipeline_size"></a>

+-----------------------+------+---------+
| name                  | type | default |
+=======================+======+=========+
| urlseed_pipeline_size | int  | 5       |
+-----------------------+------+---------+

controls the pipelining size of url and http seeds. i.e. the number of HTTP
request to keep outstanding before waiting for the first one to
complete. It's common for web servers to limit this to a relatively
low number, like 5

.. _urlseed_wait_retry:

.. raw:: html

	<a name="urlseed_wait_retry"></a>

+--------------------+------+---------+
| name               | type | default |
+====================+======+=========+
| urlseed_wait_retry | int  | 30      |
+--------------------+------+---------+

number of seconds until a new retry of a url-seed takes place.
Default retry value for http-seeds that don't provide
a valid ``retry-after`` header.

.. _file_pool_size:

.. raw:: html

	<a name="file_pool_size"></a>

+----------------+------+---------+
| name           | type | default |
+================+======+=========+
| file_pool_size | int  | 40      |
+----------------+------+---------+

sets the upper limit on the total number of files this session will
keep open. The reason why files are left open at all is that some
anti virus software hooks on every file close, and scans the file
for viruses. deferring the closing of the files will be the
difference between a usable system and a completely hogged down
system. Most operating systems also has a limit on the total number
of file descriptors a process may have open.

.. _max_failcount:

.. raw:: html

	<a name="max_failcount"></a>

+---------------+------+---------+
| name          | type | default |
+===============+======+=========+
| max_failcount | int  | 3       |
+---------------+------+---------+

``max_failcount`` is the maximum times we try to
connect to a peer before stop connecting again. If a
peer succeeds, the failure counter is reset. If a
peer is retrieved from a peer source (other than DHT)
the failcount is decremented by one, allowing another
try.

.. _min_reconnect_time:

.. raw:: html

	<a name="min_reconnect_time"></a>

+--------------------+------+---------+
| name               | type | default |
+====================+======+=========+
| min_reconnect_time | int  | 60      |
+--------------------+------+---------+

the number of seconds to wait to reconnect to a peer. this time is
multiplied with the failcount.

.. _peer_connect_timeout:

.. raw:: html

	<a name="peer_connect_timeout"></a>

+----------------------+------+---------+
| name                 | type | default |
+======================+======+=========+
| peer_connect_timeout | int  | 15      |
+----------------------+------+---------+

``peer_connect_timeout`` the number of seconds to wait after a
connection attempt is initiated to a peer until it is considered as
having timed out. This setting is especially important in case the
number of half-open connections are limited, since stale half-open
connection may delay the connection of other peers considerably.

.. _connection_speed:

.. raw:: html

	<a name="connection_speed"></a>

+------------------+------+---------+
| name             | type | default |
+==================+======+=========+
| connection_speed | int  | 30      |
+------------------+------+---------+

``connection_speed`` is the number of connection attempts that are
made per second. If a number < 0 is specified, it will default to
200 connections per second. If 0 is specified, it means don't make
outgoing connections at all.

.. _inactivity_timeout:

.. raw:: html

	<a name="inactivity_timeout"></a>

+--------------------+------+---------+
| name               | type | default |
+====================+======+=========+
| inactivity_timeout | int  | 600     |
+--------------------+------+---------+

if a peer is uninteresting and uninterested for longer than this
number of seconds, it will be disconnected.

.. _unchoke_interval:

.. raw:: html

	<a name="unchoke_interval"></a>

+------------------+------+---------+
| name             | type | default |
+==================+======+=========+
| unchoke_interval | int  | 15      |
+------------------+------+---------+

``unchoke_interval`` is the number of seconds between
chokes/unchokes. On this interval, peers are re-evaluated for being
choked/unchoked. This is defined as 30 seconds in the protocol, and
it should be significantly longer than what it takes for TCP to
ramp up to it's max rate.

.. _optimistic_unchoke_interval:

.. raw:: html

	<a name="optimistic_unchoke_interval"></a>

+-----------------------------+------+---------+
| name                        | type | default |
+=============================+======+=========+
| optimistic_unchoke_interval | int  | 30      |
+-----------------------------+------+---------+

``optimistic_unchoke_interval`` is the number of seconds between
each *optimistic* unchoke. On this timer, the currently
optimistically unchoked peer will change.

.. _num_want:

.. raw:: html

	<a name="num_want"></a>

+----------+------+---------+
| name     | type | default |
+==========+======+=========+
| num_want | int  | 200     |
+----------+------+---------+

``num_want`` is the number of peers we want from each tracker
request. It defines what is sent as the ``&num_want=`` parameter to
the tracker.

.. _initial_picker_threshold:

.. raw:: html

	<a name="initial_picker_threshold"></a>

+--------------------------+------+---------+
| name                     | type | default |
+==========================+======+=========+
| initial_picker_threshold | int  | 4       |
+--------------------------+------+---------+

``initial_picker_threshold`` specifies the number of pieces we need
before we switch to rarest first picking. The first
``initial_picker_threshold`` pieces in any torrent are picked at random
, the following pieces are picked in rarest first order.

.. _allowed_fast_set_size:

.. raw:: html

	<a name="allowed_fast_set_size"></a>

+-----------------------+------+---------+
| name                  | type | default |
+=======================+======+=========+
| allowed_fast_set_size | int  | 5       |
+-----------------------+------+---------+

the number of allowed pieces to send to peers that supports the
fast extensions

.. _suggest_mode:

.. raw:: html

	<a name="suggest_mode"></a>

+--------------+------+-------------------------------------+
| name         | type | default                             |
+==============+======+=====================================+
| suggest_mode | int  | settings_pack::no_piece_suggestions |
+--------------+------+-------------------------------------+

``suggest_mode`` controls whether or not libtorrent will send out
suggest messages to create a bias of its peers to request certain
pieces. The modes are:

* ``no_piece_suggestions`` which will not send out suggest messages.
* ``suggest_read_cache`` which will send out suggest messages for
  the most recent pieces that are in the read cache.

.. _max_queued_disk_bytes:

.. raw:: html

	<a name="max_queued_disk_bytes"></a>

+-----------------------+------+-------------+
| name                  | type | default     |
+=======================+======+=============+
| max_queued_disk_bytes | int  | 1024 * 1024 |
+-----------------------+------+-------------+

``max_queued_disk_bytes`` is the maximum number of bytes, to
be written to disk, that can wait in the disk I/O thread queue.
This queue is only for waiting for the disk I/O thread to receive
the job and either write it to disk or insert it in the write
cache. When this limit is reached, the peer connections will stop
reading data from their sockets, until the disk thread catches up.
Setting this too low will severely limit your download rate.

.. _handshake_timeout:

.. raw:: html

	<a name="handshake_timeout"></a>

+-------------------+------+---------+
| name              | type | default |
+===================+======+=========+
| handshake_timeout | int  | 10      |
+-------------------+------+---------+

the number of seconds to wait for a handshake response from a peer.
If no response is received within this time, the peer is
disconnected.

.. _send_buffer_low_watermark:

.. _send_buffer_watermark:

.. _send_buffer_watermark_factor:

.. raw:: html

	<a name="send_buffer_low_watermark"></a>
	<a name="send_buffer_watermark"></a>
	<a name="send_buffer_watermark_factor"></a>

+------------------------------+------+------------+
| name                         | type | default    |
+==============================+======+============+
| send_buffer_low_watermark    | int  | 10 * 1024  |
+------------------------------+------+------------+
| send_buffer_watermark        | int  | 500 * 1024 |
+------------------------------+------+------------+
| send_buffer_watermark_factor | int  | 50         |
+------------------------------+------+------------+

``send_buffer_low_watermark`` the minimum send buffer target size
(send buffer includes bytes pending being read from disk). For good
and snappy seeding performance, set this fairly high, to at least
fit a few blocks. This is essentially the initial window size which
will determine how fast we can ramp up the send rate

if the send buffer has fewer bytes than ``send_buffer_watermark``,
we'll read another 16 kiB block onto it. If set too small, upload
rate capacity will suffer. If set too high, memory will be wasted.
The actual watermark may be lower than this in case the upload rate
is low, this is the upper limit.

the current upload rate to a peer is multiplied by this factor to
get the send buffer watermark. The factor is specified as a
percentage. i.e. 50 -> 0.5 This product is clamped to the
``send_buffer_watermark`` setting to not exceed the max. For high
speed upload, this should be set to a greater value than 100. For
high capacity connections, setting this higher can improve upload
performance and disk throughput. Setting it too high may waste RAM
and create a bias towards read jobs over write jobs.

.. _choking_algorithm:

.. _seed_choking_algorithm:

.. raw:: html

	<a name="choking_algorithm"></a>
	<a name="seed_choking_algorithm"></a>

+------------------------+------+-----------------------------------+
| name                   | type | default                           |
+========================+======+===================================+
| choking_algorithm      | int  | settings_pack::fixed_slots_choker |
+------------------------+------+-----------------------------------+
| seed_choking_algorithm | int  | settings_pack::round_robin        |
+------------------------+------+-----------------------------------+

``choking_algorithm`` specifies which algorithm to use to determine
how many peers to unchoke. The unchoking algorithm for
downloading torrents is always "tit-for-tat", i.e. the peers we
download the fastest from are unchoked.

The options for choking algorithms are defined in the
choking_algorithm_t enum.

``seed_choking_algorithm`` controls the seeding unchoke behavior.
i.e. How we select which peers to unchoke for seeding torrents.
Since a seeding torrent isn't downloading anything, the
tit-for-tat mechanism cannot be used. The available options are
defined in the seed_choking_algorithm_t enum.

.. _disk_io_write_mode:

.. _disk_io_read_mode:

.. raw:: html

	<a name="disk_io_write_mode"></a>
	<a name="disk_io_read_mode"></a>

+--------------------+------+--------------------------------+
| name               | type | default                        |
+====================+======+================================+
| disk_io_write_mode | int  | DISK_WRITE_MODE                |
+--------------------+------+--------------------------------+
| disk_io_read_mode  | int  | settings_pack::enable_os_cache |
+--------------------+------+--------------------------------+

determines how files are opened when they're in read only mode
versus read and write mode. The options are:

enable_os_cache
  Files are opened normally, with the OS caching reads and writes.
disable_os_cache
  This opens all files in no-cache mode. This corresponds to the
  OS not letting blocks for the files linger in the cache. This
  makes sense in order to avoid the bittorrent client to
  potentially evict all other processes' cache by simply handling
  high throughput and large files. If libtorrent's read cache is
  disabled, enabling this may reduce performance.
write_through
  flush pieces to disk as they complete validation.

One reason to disable caching is that it may help the operating
system from growing its file cache indefinitely.

.. _outgoing_port:

.. _num_outgoing_ports:

.. raw:: html

	<a name="outgoing_port"></a>
	<a name="num_outgoing_ports"></a>

+--------------------+------+---------+
| name               | type | default |
+====================+======+=========+
| outgoing_port      | int  | 0       |
+--------------------+------+---------+
| num_outgoing_ports | int  | 0       |
+--------------------+------+---------+

this is the first port to use for binding outgoing connections to.
This is useful for users that have routers that allow QoS settings
based on local port. when binding outgoing connections to specific
ports, ``num_outgoing_ports`` is the size of the range. It should
be more than a few

.. warning:: setting outgoing ports will limit the ability to keep
   multiple connections to the same client, even for different
   torrents. It is not recommended to change this setting. Its main
   purpose is to use as an escape hatch for cheap routers with QoS
   capability but can only classify flows based on port numbers.

It is a range instead of a single port because of the problems with
failing to reconnect to peers if a previous socket to that peer and
port is in ``TIME_WAIT`` state.

.. _peer_dscp:

.. raw:: html

	<a name="peer_dscp"></a>

+-----------+------+---------+
| name      | type | default |
+===========+======+=========+
| peer_dscp | int  | 0x04    |
+-----------+------+---------+

``peer_dscp`` determines the DSCP field in the IP header of every
packet sent to peers (including web seeds). ``0x0`` means no marking,
``0x04`` represents Lower Effort. For more details see `RFC 8622`_.

.. _`RFC 8622`: http://www.faqs.org/rfcs/rfc8622.html

``peer_tos`` is the backwards compatible name for this setting.

.. _active_downloads:

.. _active_seeds:

.. _active_checking:

.. _active_dht_limit:

.. _active_tracker_limit:

.. _active_lsd_limit:

.. _active_limit:

.. raw:: html

	<a name="active_downloads"></a>
	<a name="active_seeds"></a>
	<a name="active_checking"></a>
	<a name="active_dht_limit"></a>
	<a name="active_tracker_limit"></a>
	<a name="active_lsd_limit"></a>
	<a name="active_limit"></a>

+----------------------+------+---------+
| name                 | type | default |
+======================+======+=========+
| active_downloads     | int  | 3       |
+----------------------+------+---------+
| active_seeds         | int  | 5       |
+----------------------+------+---------+
| active_checking      | int  | 1       |
+----------------------+------+---------+
| active_dht_limit     | int  | 88      |
+----------------------+------+---------+
| active_tracker_limit | int  | 1600    |
+----------------------+------+---------+
| active_lsd_limit     | int  | 60      |
+----------------------+------+---------+
| active_limit         | int  | 500     |
+----------------------+------+---------+

for auto managed torrents, these are the limits they are subject
to. If there are too many torrents some of the auto managed ones
will be paused until some slots free up. ``active_downloads`` and
``active_seeds`` controls how many active seeding and downloading
torrents the queuing mechanism allows. The target number of active
torrents is ``min(active_downloads + active_seeds, active_limit)``.
``active_downloads`` and ``active_seeds`` are upper limits on the
number of downloading torrents and seeding torrents respectively.
Setting the value to -1 means unlimited.

For example if there are 10 seeding torrents and 10 downloading
torrents, and ``active_downloads`` is 4 and ``active_seeds`` is 4,
there will be 4 seeds active and 4 downloading torrents. If the
settings are ``active_downloads`` = 2 and ``active_seeds`` = 4,
then there will be 2 downloading torrents and 4 seeding torrents
active. Torrents that are not auto managed are not counted against
these limits.

``active_checking`` is the limit of number of simultaneous checking
torrents.

``active_limit`` is a hard limit on the number of active (auto
managed) torrents. This limit also applies to slow torrents.

``active_dht_limit`` is the max number of torrents to announce to
the DHT.

``active_tracker_limit`` is the max number of torrents to announce
to their trackers.

``active_lsd_limit`` is the max number of torrents to announce to
the local network over the local service discovery protocol.

You can have more torrents *active*, even though they are not
announced to the DHT, lsd or their tracker. If some peer knows
about you for any reason and tries to connect, it will still be
accepted, unless the torrent is paused, which means it won't accept
any connections.

.. _auto_manage_interval:

.. raw:: html

	<a name="auto_manage_interval"></a>

+----------------------+------+---------+
| name                 | type | default |
+======================+======+=========+
| auto_manage_interval | int  | 30      |
+----------------------+------+---------+

``auto_manage_interval`` is the number of seconds between the
torrent queue is updated, and rotated.

.. _seed_time_limit:

.. raw:: html

	<a name="seed_time_limit"></a>

+-----------------+------+--------------+
| name            | type | default      |
+=================+======+==============+
| seed_time_limit | int  | 24 * 60 * 60 |
+-----------------+------+--------------+

this is the limit on the time a torrent has been an active seed
(specified in seconds) before it is considered having met the seed
limit criteria. See queuing_.

.. _auto_scrape_interval:

.. _auto_scrape_min_interval:

.. raw:: html

	<a name="auto_scrape_interval"></a>
	<a name="auto_scrape_min_interval"></a>

+--------------------------+------+---------+
| name                     | type | default |
+==========================+======+=========+
| auto_scrape_interval     | int  | 1800    |
+--------------------------+------+---------+
| auto_scrape_min_interval | int  | 300     |
+--------------------------+------+---------+

``auto_scrape_interval`` is the number of seconds between scrapes
of queued torrents (auto managed and paused torrents). Auto managed
torrents that are paused, are scraped regularly in order to keep
track of their downloader/seed ratio. This ratio is used to
determine which torrents to seed and which to pause.

``auto_scrape_min_interval`` is the minimum number of seconds
between any automatic scrape (regardless of torrent). In case there
are a large number of paused auto managed torrents, this puts a
limit on how often a scrape request is sent.

.. _max_peerlist_size:

.. _max_paused_peerlist_size:

.. raw:: html

	<a name="max_peerlist_size"></a>
	<a name="max_paused_peerlist_size"></a>

+--------------------------+------+---------+
| name                     | type | default |
+==========================+======+=========+
| max_peerlist_size        | int  | 3000    |
+--------------------------+------+---------+
| max_paused_peerlist_size | int  | 1000    |
+--------------------------+------+---------+

``max_peerlist_size`` is the maximum number of peers in the list of
known peers. These peers are not necessarily connected, so this
number should be much greater than the maximum number of connected
peers. Peers are evicted from the cache when the list grows passed
90% of this limit, and once the size hits the limit, peers are no
longer added to the list. If this limit is set to 0, there is no
limit on how many peers we'll keep in the peer list.

``max_paused_peerlist_size`` is the max peer list size used for
torrents that are paused. This can be used to save memory for paused
torrents, since it's not as important for them to keep a large peer
list.

.. _min_announce_interval:

.. raw:: html

	<a name="min_announce_interval"></a>

+-----------------------+------+---------+
| name                  | type | default |
+=======================+======+=========+
| min_announce_interval | int  | 5 * 60  |
+-----------------------+------+---------+

this is the minimum allowed announce interval for a tracker. This
is specified in seconds and is used as a sanity check on what is
returned from a tracker. It mitigates hammering mis-configured
trackers.

.. _auto_manage_startup:

.. raw:: html

	<a name="auto_manage_startup"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| auto_manage_startup | int  | 60      |
+---------------------+------+---------+

this is the number of seconds a torrent is considered active after
it was started, regardless of upload and download speed. This is so
that newly started torrents are not considered inactive until they
have a fair chance to start downloading.

.. _seeding_piece_quota:

.. raw:: html

	<a name="seeding_piece_quota"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| seeding_piece_quota | int  | 20      |
+---------------------+------+---------+

``seeding_piece_quota`` is the number of pieces to send to a peer,
when seeding, before rotating in another peer to the unchoke set.

.. _max_rejects:

.. raw:: html

	<a name="max_rejects"></a>

+-------------+------+---------+
| name        | type | default |
+=============+======+=========+
| max_rejects | int  | 50      |
+-------------+------+---------+

``max_rejects`` is the number of piece requests we will reject in a
row while a peer is choked before the peer is considered abusive
and is disconnected.

.. _recv_socket_buffer_size:

.. _send_socket_buffer_size:

.. raw:: html

	<a name="recv_socket_buffer_size"></a>
	<a name="send_socket_buffer_size"></a>

+-------------------------+------+---------+
| name                    | type | default |
+=========================+======+=========+
| recv_socket_buffer_size | int  | 0       |
+-------------------------+------+---------+
| send_socket_buffer_size | int  | 0       |
+-------------------------+------+---------+

specifies the buffer sizes set on peer sockets. 0 means the OS
default (i.e. don't change the buffer sizes).
The socket buffer sizes are changed using setsockopt() with
SOL_SOCKET/SO_RCVBUF and SO_SNDBUFFER.

Note that uTP peers share a single UDP socket buffer for each of the
``listen_interfaces``, along with DHT and UDP tracker traffic.
If the buffer size is too small for the combined traffic through the
socket, packets may be dropped.

.. _max_peer_recv_buffer_size:

.. raw:: html

	<a name="max_peer_recv_buffer_size"></a>

+---------------------------+------+-----------------+
| name                      | type | default         |
+===========================+======+=================+
| max_peer_recv_buffer_size | int  | 2 * 1024 * 1024 |
+---------------------------+------+-----------------+

the max number of bytes a single peer connection's receive buffer is
allowed to grow to.

.. _optimistic_disk_retry:

.. raw:: html

	<a name="optimistic_disk_retry"></a>

+-----------------------+------+---------+
| name                  | type | default |
+=======================+======+=========+
| optimistic_disk_retry | int  | 10 * 60 |
+-----------------------+------+---------+

``optimistic_disk_retry`` is the number of seconds from a disk
write errors occur on a torrent until libtorrent will take it out
of the upload mode, to test if the error condition has been fixed.

libtorrent will only do this automatically for auto managed
torrents.

You can explicitly take a torrent out of upload only mode using
set_upload_mode().

.. _max_suggest_pieces:

.. raw:: html

	<a name="max_suggest_pieces"></a>

+--------------------+------+---------+
| name               | type | default |
+====================+======+=========+
| max_suggest_pieces | int  | 16      |
+--------------------+------+---------+

``max_suggest_pieces`` is the max number of suggested piece indices
received from a peer that's remembered. If a peer floods suggest
messages, this limit prevents libtorrent from using too much RAM.

.. _local_service_announce_interval:

.. raw:: html

	<a name="local_service_announce_interval"></a>

+---------------------------------+------+---------+
| name                            | type | default |
+=================================+======+=========+
| local_service_announce_interval | int  | 5 * 60  |
+---------------------------------+------+---------+

``local_service_announce_interval`` is the time between local
network announces for a torrent.
This interval is specified in seconds.

.. _dht_announce_interval:

.. raw:: html

	<a name="dht_announce_interval"></a>

+-----------------------+------+---------+
| name                  | type | default |
+=======================+======+=========+
| dht_announce_interval | int  | 15 * 60 |
+-----------------------+------+---------+

``dht_announce_interval`` is the number of seconds between
announcing torrents to the distributed hash table (DHT).

.. _udp_tracker_token_expiry:

.. raw:: html

	<a name="udp_tracker_token_expiry"></a>

+--------------------------+------+---------+
| name                     | type | default |
+==========================+======+=========+
| udp_tracker_token_expiry | int  | 60      |
+--------------------------+------+---------+

``udp_tracker_token_expiry`` is the number of seconds libtorrent
will keep UDP tracker connection tokens around for. This is
specified to be 60 seconds. The higher this
value is, the fewer packets have to be sent to the UDP tracker. In
order for higher values to work, the tracker needs to be configured
to match the expiration time for tokens.

.. _num_optimistic_unchoke_slots:

.. raw:: html

	<a name="num_optimistic_unchoke_slots"></a>

+------------------------------+------+---------+
| name                         | type | default |
+==============================+======+=========+
| num_optimistic_unchoke_slots | int  | 0       |
+------------------------------+------+---------+

``num_optimistic_unchoke_slots`` is the number of optimistic
unchoke slots to use.
Having a higher number of optimistic unchoke slots mean you will
find the good peers faster but with the trade-off to use up more
bandwidth. 0 means automatic, where libtorrent opens up 20% of your
allowed upload slots as optimistic unchoke slots.

.. _max_pex_peers:

.. raw:: html

	<a name="max_pex_peers"></a>

+---------------+------+---------+
| name          | type | default |
+===============+======+=========+
| max_pex_peers | int  | 50      |
+---------------+------+---------+

the max number of peers we accept from pex messages from a single
peer. this limits the number of concurrent peers any of our peers
claims to be connected to. If they claim to be connected to more
than this, we'll ignore any peer that exceeds this limit

.. _tick_interval:

.. raw:: html

	<a name="tick_interval"></a>

+---------------+------+---------+
| name          | type | default |
+===============+======+=========+
| tick_interval | int  | 500     |
+---------------+------+---------+

``tick_interval`` specifies the number of milliseconds between
internal ticks. This is the frequency with which bandwidth quota is
distributed to peers. It should not be more than one second (i.e.
1000 ms). Setting this to a low value (around 100) means higher
resolution bandwidth quota distribution, setting it to a higher
value saves CPU cycles.

.. _share_mode_target:

.. raw:: html

	<a name="share_mode_target"></a>

+-------------------+------+---------+
| name              | type | default |
+===================+======+=========+
| share_mode_target | int  | 3       |
+-------------------+------+---------+

``share_mode_target`` specifies the target share ratio for share
mode torrents. If set to 3, we'll try to upload 3
times as much as we download. Setting this very high, will make it
very conservative and you might end up not downloading anything
ever (and not affecting your share ratio). It does not make any
sense to set this any lower than 2. For instance, if only 3 peers
need to download the rarest piece, it's impossible to download a
single piece and upload it more than 3 times. If the
share_mode_target is set to more than 3, nothing is downloaded.

.. _upload_rate_limit:

.. _download_rate_limit:

.. raw:: html

	<a name="upload_rate_limit"></a>
	<a name="download_rate_limit"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| upload_rate_limit   | int  | 0       |
+---------------------+------+---------+
| download_rate_limit | int  | 0       |
+---------------------+------+---------+

``upload_rate_limit`` and ``download_rate_limit`` sets
the session-global limits of upload and download rate limits, in
bytes per second. By default peers on the local network are not rate
limited.

A value of 0 means unlimited.

For fine grained control over rate limits, including making them apply
to local peers, see peer-classes_.

.. _dht_upload_rate_limit:

.. raw:: html

	<a name="dht_upload_rate_limit"></a>

+-----------------------+------+---------+
| name                  | type | default |
+=======================+======+=========+
| dht_upload_rate_limit | int  | 8000    |
+-----------------------+------+---------+

the number of bytes per second (on average) the DHT is allowed to send.
If the incoming requests causes to many bytes to be sent in responses,
incoming requests will be dropped until the quota has been replenished.

.. _unchoke_slots_limit:

.. raw:: html

	<a name="unchoke_slots_limit"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| unchoke_slots_limit | int  | 8       |
+---------------------+------+---------+

``unchoke_slots_limit`` is the max number of unchoked peers in the
session. The number of unchoke slots may be ignored depending on
what ``choking_algorithm`` is set to. Setting this limit to -1
means unlimited, i.e. all peers will always be unchoked.

.. _connections_limit:

.. raw:: html

	<a name="connections_limit"></a>

+-------------------+------+---------+
| name              | type | default |
+===================+======+=========+
| connections_limit | int  | 200     |
+-------------------+------+---------+

``connections_limit`` sets a global limit on the number of
connections opened. The number of connections is set to a hard
minimum of at least two per torrent, so if you set a too low
connections limit, and open too many torrents, the limit will not
be met.

.. _connections_slack:

.. raw:: html

	<a name="connections_slack"></a>

+-------------------+------+---------+
| name              | type | default |
+===================+======+=========+
| connections_slack | int  | 10      |
+-------------------+------+---------+

``connections_slack`` is the number of incoming connections
exceeding the connection limit to accept in order to potentially
replace existing ones.

.. _utp_target_delay:

.. _utp_gain_factor:

.. _utp_min_timeout:

.. _utp_syn_resends:

.. _utp_fin_resends:

.. _utp_num_resends:

.. _utp_connect_timeout:

.. _utp_loss_multiplier:

.. raw:: html

	<a name="utp_target_delay"></a>
	<a name="utp_gain_factor"></a>
	<a name="utp_min_timeout"></a>
	<a name="utp_syn_resends"></a>
	<a name="utp_fin_resends"></a>
	<a name="utp_num_resends"></a>
	<a name="utp_connect_timeout"></a>
	<a name="utp_loss_multiplier"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| utp_target_delay    | int  | 100     |
+---------------------+------+---------+
| utp_gain_factor     | int  | 3000    |
+---------------------+------+---------+
| utp_min_timeout     | int  | 500     |
+---------------------+------+---------+
| utp_syn_resends     | int  | 2       |
+---------------------+------+---------+
| utp_fin_resends     | int  | 2       |
+---------------------+------+---------+
| utp_num_resends     | int  | 3       |
+---------------------+------+---------+
| utp_connect_timeout | int  | 3000    |
+---------------------+------+---------+
| utp_loss_multiplier | int  | 50      |
+---------------------+------+---------+

``utp_target_delay`` is the target delay for uTP sockets in
milliseconds. A high value will make uTP connections more
aggressive and cause longer queues in the upload bottleneck. It
cannot be too low, since the noise in the measurements would cause
it to send too slow.
``utp_gain_factor`` is the number of bytes the uTP congestion
window can increase at the most in one RTT.
If this is set too high, the congestion controller reacts
too hard to noise and will not be stable, if it's set too low, it
will react slow to congestion and not back off as fast.

``utp_min_timeout`` is the shortest allowed uTP socket timeout,
specified in milliseconds. The
timeout depends on the RTT of the connection, but is never smaller
than this value. A connection times out when every packet in a
window is lost, or when a packet is lost twice in a row (i.e. the
resent packet is lost as well).

The shorter the timeout is, the faster the connection will recover
from this situation, assuming the RTT is low enough.
``utp_syn_resends`` is the number of SYN packets that are sent (and
timed out) before giving up and closing the socket.
``utp_num_resends`` is the number of times a packet is sent (and
lost or timed out) before giving up and closing the connection.
``utp_connect_timeout`` is the number of milliseconds of timeout
for the initial SYN packet for uTP connections. For each timed out
packet (in a row), the timeout is doubled. ``utp_loss_multiplier``
controls how the congestion window is changed when a packet loss is
experienced. It's specified as a percentage multiplier for
``cwnd``. Do not change this value unless you know what you're doing.
Never set it higher than 100.

.. _mixed_mode_algorithm:

.. raw:: html

	<a name="mixed_mode_algorithm"></a>

+----------------------+------+----------------------------------+
| name                 | type | default                          |
+======================+======+==================================+
| mixed_mode_algorithm | int  | settings_pack::peer_proportional |
+----------------------+------+----------------------------------+

The ``mixed_mode_algorithm`` determines how to treat TCP
connections when there are uTP connections. Since uTP is designed
to yield to TCP, there's an inherent problem when using swarms that
have both TCP and uTP connections. If nothing is done, uTP
connections would often be starved out for bandwidth by the TCP
connections. This mode is ``prefer_tcp``. The ``peer_proportional``
mode simply looks at the current throughput and rate limits all TCP
connections to their proportional share based on how many of the
connections are TCP. This works best if uTP connections are not
rate limited by the global rate limiter (which they aren't by
default).

.. _listen_queue_size:

.. raw:: html

	<a name="listen_queue_size"></a>

+-------------------+------+---------+
| name              | type | default |
+===================+======+=========+
| listen_queue_size | int  | 5       |
+-------------------+------+---------+

``listen_queue_size`` is the value passed in to listen() for the
listen socket. It is the number of outstanding incoming connections
to queue up while we're not actively waiting for a connection to be
accepted. 5 should be sufficient for any
normal client. If this is a high performance server which expects
to receive a lot of connections, or used in a simulator or test, it
might make sense to raise this number. It will not take affect
until the ``listen_interfaces`` settings is updated.

.. _torrent_connect_boost:

.. raw:: html

	<a name="torrent_connect_boost"></a>

+-----------------------+------+---------+
| name                  | type | default |
+=======================+======+=========+
| torrent_connect_boost | int  | 30      |
+-----------------------+------+---------+

``torrent_connect_boost`` is the number of peers to try to connect
to immediately when the first tracker response is received for a
torrent. This is a boost to given to new torrents to accelerate
them starting up. The normal connect scheduler is run once every
second, this allows peers to be connected immediately instead of
waiting for the session tick to trigger connections.
This may not be set higher than 255.

.. _alert_queue_size:

.. raw:: html

	<a name="alert_queue_size"></a>

+------------------+------+---------+
| name             | type | default |
+==================+======+=========+
| alert_queue_size | int  | 2000    |
+------------------+------+---------+

``alert_queue_size`` is the maximum number of alerts queued up
internally. If alerts are not popped, the queue will eventually
fill up to this level. Once the alert queue is full, additional
alerts will be dropped, and not delivered to the client. Once the
client drains the queue, new alerts may be delivered again. In order
to know that alerts have been dropped, see
session_handle::dropped_alerts().

.. _max_metadata_size:

.. raw:: html

	<a name="max_metadata_size"></a>

+-------------------+------+------------------+
| name              | type | default          |
+===================+======+==================+
| max_metadata_size | int  | 3 * 1024 * 10240 |
+-------------------+------+------------------+

``max_metadata_size`` is the maximum allowed size (in bytes) to be
received by the metadata extension, i.e. magnet links.

.. _hashing_threads:

.. raw:: html

	<a name="hashing_threads"></a>

+-----------------+------+---------+
| name            | type | default |
+=================+======+=========+
| hashing_threads | int  | 1       |
+-----------------+------+---------+

``hashing_threads`` is the number of disk I/O threads to use for
piece hash verification. These threads are *in addition* to the
regular disk I/O threads specified by settings_pack::aio_threads.
These threads are only used for full checking of torrents. The
hash checking done while downloading are done by the regular disk
I/O threads.
The hasher threads do not only compute hashes, but also perform
the read from disk. On storage optimal for sequential access,
such as hard drives, this setting should be set to 1, which is
also the default.

.. _checking_mem_usage:

.. raw:: html

	<a name="checking_mem_usage"></a>

+--------------------+------+---------+
| name               | type | default |
+====================+======+=========+
| checking_mem_usage | int  | 256     |
+--------------------+------+---------+

the number of blocks to keep outstanding at any given time when
checking torrents. Higher numbers give faster re-checks but uses
more memory. Specified in number of 16 kiB blocks

.. _predictive_piece_announce:

.. raw:: html

	<a name="predictive_piece_announce"></a>

+---------------------------+------+---------+
| name                      | type | default |
+===========================+======+=========+
| predictive_piece_announce | int  | 0       |
+---------------------------+------+---------+

if set to > 0, pieces will be announced to other peers before they
are fully downloaded (and before they are hash checked). The
intention is to gain 1.5 potential round trip times per downloaded
piece. When non-zero, this indicates how many milliseconds in
advance pieces should be announced, before they are expected to be
completed.

.. _aio_threads:

.. raw:: html

	<a name="aio_threads"></a>

+-------------+------+---------+
| name        | type | default |
+=============+======+=========+
| aio_threads | int  | 10      |
+-------------+------+---------+

for some aio back-ends, ``aio_threads`` specifies the number of
io-threads to use.

.. _tracker_backoff:

.. raw:: html

	<a name="tracker_backoff"></a>

+-----------------+------+---------+
| name            | type | default |
+=================+======+=========+
| tracker_backoff | int  | 250     |
+-----------------+------+---------+

``tracker_backoff`` determines how aggressively to back off from
retrying failing trackers. This value determines *x* in the
following formula, determining the number of seconds to wait until
the next retry:

   delay = 5 + 5 * x / 100 * fails^2

This setting may be useful to make libtorrent more or less
aggressive in hitting trackers.

.. _share_ratio_limit:

.. _seed_time_ratio_limit:

.. raw:: html

	<a name="share_ratio_limit"></a>
	<a name="seed_time_ratio_limit"></a>

+-----------------------+------+---------+
| name                  | type | default |
+=======================+======+=========+
| share_ratio_limit     | int  | 200     |
+-----------------------+------+---------+
| seed_time_ratio_limit | int  | 700     |
+-----------------------+------+---------+

when a seeding torrent reaches either the share ratio (bytes up /
bytes down) or the seed time ratio (seconds as seed / seconds as
downloader) or the seed time limit (seconds as seed) it is
considered done, and it will leave room for other torrents. These
are specified as percentages. Torrents that are considered done will
still be allowed to be seeded, they just won't have priority anymore.
For more, see queuing_.

.. _peer_turnover:

.. _peer_turnover_cutoff:

.. _peer_turnover_interval:

.. raw:: html

	<a name="peer_turnover"></a>
	<a name="peer_turnover_cutoff"></a>
	<a name="peer_turnover_interval"></a>

+------------------------+------+---------+
| name                   | type | default |
+========================+======+=========+
| peer_turnover          | int  | 4       |
+------------------------+------+---------+
| peer_turnover_cutoff   | int  | 90      |
+------------------------+------+---------+
| peer_turnover_interval | int  | 300     |
+------------------------+------+---------+

peer_turnover is the percentage of peers to disconnect every
turnover peer_turnover_interval (if we're at the peer limit), this
is specified in percent when we are connected to more than limit *
peer_turnover_cutoff peers disconnect peer_turnover fraction of the
peers. It is specified in percent peer_turnover_interval is the
interval (in seconds) between optimistic disconnects if the
disconnects happen and how many peers are disconnected is
controlled by peer_turnover and peer_turnover_cutoff

.. _connect_seed_every_n_download:

.. raw:: html

	<a name="connect_seed_every_n_download"></a>

+-------------------------------+------+---------+
| name                          | type | default |
+===============================+======+=========+
| connect_seed_every_n_download | int  | 10      |
+-------------------------------+------+---------+

this setting controls the priority of downloading torrents over
seeding or finished torrents when it comes to making peer
connections. Peer connections are throttled by the connection_speed
and the half-open connection limit. This makes peer connections a
limited resource. Torrents that still have pieces to download are
prioritized by default, to avoid having many seeding torrents use
most of the connection attempts and only give one peer every now
and then to the downloading torrent. libtorrent will loop over the
downloading torrents to connect a peer each, and every n:th
connection attempt, a finished torrent is picked to be allowed to
connect to a peer. This setting controls n.

.. _max_http_recv_buffer_size:

.. raw:: html

	<a name="max_http_recv_buffer_size"></a>

+---------------------------+------+------------+
| name                      | type | default    |
+===========================+======+============+
| max_http_recv_buffer_size | int  | 4*1024*204 |
+---------------------------+------+------------+

the max number of bytes to allow an HTTP response to be when
announcing to trackers or downloading .torrent files via the
``url`` provided in ``add_torrent_params``.

.. _max_retry_port_bind:

.. raw:: html

	<a name="max_retry_port_bind"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| max_retry_port_bind | int  | 10      |
+---------------------+------+---------+

if binding to a specific port fails, should the port be incremented
by one and tried again? This setting specifies how many times to
retry a failed port bind

.. _alert_mask:

.. raw:: html

	<a name="alert_mask"></a>

+------------+------+---------+
| name       | type | default |
+============+======+=========+
| alert_mask | int  | int     |
+------------+------+---------+

a bitmask combining flags from alert_category_t defining which
kinds of alerts to receive

.. _out_enc_policy:

.. _in_enc_policy:

.. raw:: html

	<a name="out_enc_policy"></a>
	<a name="in_enc_policy"></a>

+----------------+------+---------------------------+
| name           | type | default                   |
+================+======+===========================+
| out_enc_policy | int  | settings_pack::pe_enabled |
+----------------+------+---------------------------+
| in_enc_policy  | int  | settings_pack::pe_enabled |
+----------------+------+---------------------------+

control the settings for incoming and outgoing connections
respectively. see enc_policy enum for the available options.
Keep in mind that protocol encryption degrades performance in
several respects:

1. It prevents "zero copy" disk buffers being sent to peers, since
   each peer needs to mutate the data (i.e. encrypt it) the data
   must be copied per peer connection rather than sending the same
   buffer to multiple peers.
2. The encryption itself requires more CPU than plain bittorrent
   protocol. The highest cost is the Diffie Hellman exchange on
   connection setup.
3. The encryption handshake adds several round-trips to the
   connection setup, and delays transferring data.

.. _allowed_enc_level:

.. raw:: html

	<a name="allowed_enc_level"></a>

+-------------------+------+------------------------+
| name              | type | default                |
+===================+======+========================+
| allowed_enc_level | int  | settings_pack::pe_both |
+-------------------+------+------------------------+

determines the encryption level of the connections. This setting
will adjust which encryption scheme is offered to the other peer,
as well as which encryption scheme is selected by the client. See
enc_level enum for options.

.. _inactive_down_rate:

.. _inactive_up_rate:

.. raw:: html

	<a name="inactive_down_rate"></a>
	<a name="inactive_up_rate"></a>

+--------------------+------+---------+
| name               | type | default |
+====================+======+=========+
| inactive_down_rate | int  | 2048    |
+--------------------+------+---------+
| inactive_up_rate   | int  | 2048    |
+--------------------+------+---------+

the download and upload rate limits for a torrent to be considered
active by the queuing mechanism. A torrent whose download rate is
less than ``inactive_down_rate`` and whose upload rate is less than
``inactive_up_rate`` for ``auto_manage_startup`` seconds, is
considered inactive, and another queued torrent may be started.
This logic is disabled if ``dont_count_slow_torrents`` is false.

.. _proxy_type:

.. raw:: html

	<a name="proxy_type"></a>

+------------+------+---------------------+
| name       | type | default             |
+============+======+=====================+
| proxy_type | int  | settings_pack::none |
+------------+------+---------------------+

proxy to use. see proxy_type_t.

.. _proxy_port:

.. raw:: html

	<a name="proxy_port"></a>

+------------+------+---------+
| name       | type | default |
+============+======+=========+
| proxy_port | int  | 0       |
+------------+------+---------+

the port of the proxy server

.. _i2p_port:

.. raw:: html

	<a name="i2p_port"></a>

+----------+------+---------+
| name     | type | default |
+==========+======+=========+
| i2p_port | int  | 0       |
+----------+------+---------+

sets the i2p_ SAM bridge port to connect to. set the hostname with
the ``i2p_hostname`` setting.

.. _i2p: http://www.i2p2.de

.. _urlseed_max_request_bytes:

.. raw:: html

	<a name="urlseed_max_request_bytes"></a>

+---------------------------+------+------------------+
| name                      | type | default          |
+===========================+======+==================+
| urlseed_max_request_bytes | int  | 16 * 1024 * 1024 |
+---------------------------+------+------------------+

The maximum request range of an url seed in bytes. This value
defines the largest possible sequential web seed request. Lower values
are possible but will be ignored if they are lower then piece size.
This value should be related to your download speed to prevent
libtorrent from creating too many expensive http requests per
second. You can select a value as high as you want but keep in mind
that libtorrent can't create parallel requests if the first request
did already select the whole file.
If you combine bittorrent seeds with web seeds and pick strategies
like rarest first you may find your web seed requests split into
smaller parts because we don't download already picked pieces
twice.

.. _web_seed_name_lookup_retry:

.. raw:: html

	<a name="web_seed_name_lookup_retry"></a>

+----------------------------+------+---------+
| name                       | type | default |
+============================+======+=========+
| web_seed_name_lookup_retry | int  | 1800    |
+----------------------------+------+---------+

time to wait until a new retry of a web seed name lookup

.. _close_file_interval:

.. raw:: html

	<a name="close_file_interval"></a>

+---------------------+------+---------------------+
| name                | type | default             |
+=====================+======+=====================+
| close_file_interval | int  | CLOSE_FILE_INTERVAL |
+---------------------+------+---------------------+

the number of seconds between closing the file opened the longest
ago. 0 means to disable the feature. The purpose of this is to
periodically close files to trigger the operating system flushing
disk cache. Specifically it has been observed to be required on
windows to not have the disk cache grow indefinitely.
This defaults to 240 seconds on windows, and disabled on other
systems.

.. _utp_cwnd_reduce_timer:

.. raw:: html

	<a name="utp_cwnd_reduce_timer"></a>

+-----------------------+------+---------+
| name                  | type | default |
+=======================+======+=========+
| utp_cwnd_reduce_timer | int  | 100     |
+-----------------------+------+---------+

When uTP experiences packet loss, it will reduce the congestion
window, and not reduce it again for this many milliseconds, even if
experiencing another lost packet.

.. _max_web_seed_connections:

.. raw:: html

	<a name="max_web_seed_connections"></a>

+--------------------------+------+---------+
| name                     | type | default |
+==========================+======+=========+
| max_web_seed_connections | int  | 3       |
+--------------------------+------+---------+

the max number of web seeds to have connected per torrent at any
given time.

.. _resolver_cache_timeout:

.. raw:: html

	<a name="resolver_cache_timeout"></a>

+------------------------+------+---------+
| name                   | type | default |
+========================+======+=========+
| resolver_cache_timeout | int  | 1200    |
+------------------------+------+---------+

the number of seconds before the internal host name resolver
considers a cache value timed out, negative values are interpreted
as zero.

.. _send_not_sent_low_watermark:

.. raw:: html

	<a name="send_not_sent_low_watermark"></a>

+-----------------------------+------+---------+
| name                        | type | default |
+=============================+======+=========+
| send_not_sent_low_watermark | int  | 16384   |
+-----------------------------+------+---------+

specify the not-sent low watermark for socket send buffers. This
corresponds to the, Linux-specific, ``TCP_NOTSENT_LOWAT`` TCP socket
option.

.. _rate_choker_initial_threshold:

.. raw:: html

	<a name="rate_choker_initial_threshold"></a>

+-------------------------------+------+---------+
| name                          | type | default |
+===============================+======+=========+
| rate_choker_initial_threshold | int  | 1024    |
+-------------------------------+------+---------+

the rate based choker compares the upload rate to peers against a
threshold that increases proportionally by its size for every
peer it visits, visiting peers in decreasing upload rate. The
number of upload slots is determined by the number of peers whose
upload rate exceeds the threshold. This option sets the start
value for this threshold. A higher value leads to fewer unchoke
slots, a lower value leads to more.

.. _upnp_lease_duration:

.. raw:: html

	<a name="upnp_lease_duration"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| upnp_lease_duration | int  | 3600    |
+---------------------+------+---------+

The expiration time of UPnP port-mappings, specified in seconds. 0
means permanent lease. Some routers do not support expiration times
on port-maps (nor correctly returning an error indicating lack of
support). In those cases, set this to 0. Otherwise, don't set it any
lower than 5 minutes.

.. _max_concurrent_http_announces:

.. raw:: html

	<a name="max_concurrent_http_announces"></a>

+-------------------------------+------+---------+
| name                          | type | default |
+===============================+======+=========+
| max_concurrent_http_announces | int  | 50      |
+-------------------------------+------+---------+

limits the number of concurrent HTTP tracker announces. Once the
limit is hit, tracker requests are queued and issued when an
outstanding announce completes.

.. _dht_max_peers_reply:

.. raw:: html

	<a name="dht_max_peers_reply"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| dht_max_peers_reply | int  | 100     |
+---------------------+------+---------+

the maximum number of peers to send in a reply to ``get_peers``

.. _dht_search_branching:

.. raw:: html

	<a name="dht_search_branching"></a>

+----------------------+------+---------+
| name                 | type | default |
+======================+======+=========+
| dht_search_branching | int  | 5       |
+----------------------+------+---------+

the number of concurrent search request the node will send when
announcing and refreshing the routing table. This parameter is called
alpha in the kademlia paper

.. _dht_max_fail_count:

.. raw:: html

	<a name="dht_max_fail_count"></a>

+--------------------+------+---------+
| name               | type | default |
+====================+======+=========+
| dht_max_fail_count | int  | 20      |
+--------------------+------+---------+

the maximum number of failed tries to contact a node before it is
removed from the routing table. If there are known working nodes that
are ready to replace a failing node, it will be replaced immediately,
this limit is only used to clear out nodes that don't have any node
that can replace them.

.. _dht_max_torrents:

.. raw:: html

	<a name="dht_max_torrents"></a>

+------------------+------+---------+
| name             | type | default |
+==================+======+=========+
| dht_max_torrents | int  | 2000    |
+------------------+------+---------+

the total number of torrents to track from the DHT. This is simply an
upper limit to make sure malicious DHT nodes cannot make us allocate
an unbounded amount of memory.

.. _dht_max_dht_items:

.. raw:: html

	<a name="dht_max_dht_items"></a>

+-------------------+------+---------+
| name              | type | default |
+===================+======+=========+
| dht_max_dht_items | int  | 700     |
+-------------------+------+---------+

max number of items the DHT will store

.. _dht_max_peers:

.. raw:: html

	<a name="dht_max_peers"></a>

+---------------+------+---------+
| name          | type | default |
+===============+======+=========+
| dht_max_peers | int  | 500     |
+---------------+------+---------+

the max number of peers to store per torrent (for the DHT)

.. _dht_max_torrent_search_reply:

.. raw:: html

	<a name="dht_max_torrent_search_reply"></a>

+------------------------------+------+---------+
| name                         | type | default |
+==============================+======+=========+
| dht_max_torrent_search_reply | int  | 20      |
+------------------------------+------+---------+

the max number of torrents to return in a torrent search query to the
DHT

.. _dht_block_timeout:

.. raw:: html

	<a name="dht_block_timeout"></a>

+-------------------+------+---------+
| name              | type | default |
+===================+======+=========+
| dht_block_timeout | int  | 5 * 60  |
+-------------------+------+---------+

the number of seconds a DHT node is banned if it exceeds the rate
limit. The rate limit is averaged over 10 seconds to allow for bursts
above the limit.

.. _dht_block_ratelimit:

.. raw:: html

	<a name="dht_block_ratelimit"></a>

+---------------------+------+---------+
| name                | type | default |
+=====================+======+=========+
| dht_block_ratelimit | int  | 5       |
+---------------------+------+---------+

the max number of packets per second a DHT node is allowed to send
without getting banned.

.. _dht_item_lifetime:

.. raw:: html

	<a name="dht_item_lifetime"></a>

+-------------------+------+---------+
| name              | type | default |
+===================+======+=========+
| dht_item_lifetime | int  | 0       |
+-------------------+------+---------+

the number of seconds a immutable/mutable item will be expired.
default is 0, means never expires.

.. _dht_sample_infohashes_interval:

.. raw:: html

	<a name="dht_sample_infohashes_interval"></a>

+--------------------------------+------+---------+
| name                           | type | default |
+================================+======+=========+
| dht_sample_infohashes_interval | int  | 21600   |
+--------------------------------+------+---------+

the info-hashes sample recomputation interval (in seconds).
The node will precompute a subset of the tracked info-hashes and return
that instead of calculating it upon each request. The permissible range
is between 0 and 21600 seconds (inclusive).

.. _dht_max_infohashes_sample_count:

.. raw:: html

	<a name="dht_max_infohashes_sample_count"></a>

+---------------------------------+------+---------+
| name                            | type | default |
+=================================+======+=========+
| dht_max_infohashes_sample_count | int  | 20      |
+---------------------------------+------+---------+

the maximum number of elements in the sampled subset of info-hashes.
If this number is too big, expect the DHT storage implementations
to clamp it in order to allow UDP packets go through

.. _max_piece_count:

.. raw:: html

	<a name="max_piece_count"></a>

+-----------------+------+----------+
| name            | type | default  |
+=================+======+==========+
| max_piece_count | int  | 0x200000 |
+-----------------+------+----------+

``max_piece_count`` is the maximum allowed number of pieces in
metadata received via magnet links. Loading large torrents (with
more pieces than the default limit) may also require passing in
a higher limit to read_resume_data() and
torrent_info::parse_info_section(), if those are used.

.. _metadata_token_limit:

.. raw:: html

	<a name="metadata_token_limit"></a>

+----------------------+------+---------+
| name                 | type | default |
+======================+======+=========+
| metadata_token_limit | int  | 2500000 |
+----------------------+------+---------+

when receiving metadata (torrent file) from peers, this is the
max number of bencoded tokens we're willing to parse. This limit
is meant to prevent DoS attacks on peers. For very large
torrents, this limit may have to be raised.

.. _disk_write_mode:

.. raw:: html

	<a name="disk_write_mode"></a>

+-----------------+------+---------------------------------------------------+
| name            | type | default                                           |
+=================+======+===================================================+
| disk_write_mode | int  | settings_pack::mmap_write_mode_t::auto_mmap_write |
+-----------------+------+---------------------------------------------------+

controls whether disk writes will be made through a memory mapped
file or via normal write calls. This only affects the
mmap_disk_io. When saving to a non-local drive (network share,
NFS or NAS) using memory mapped files is most likely inferior.
When writing to a local SSD (especially in DAX mode) using memory
mapped files likely gives the best performance.
The values for this setting are specified as mmap_write_mode_t.

.. _mmap_file_size_cutoff:

.. raw:: html

	<a name="mmap_file_size_cutoff"></a>

+-----------------------+------+---------+
| name                  | type | default |
+=======================+======+=========+
| mmap_file_size_cutoff | int  | 40      |
+-----------------------+------+---------+

when using mmap_disk_io, files smaller than this number of blocks
will not be memory mapped, but will use normal pread/pwrite
operations. This file size limit is specified in 16 kiB blocks.

.. _i2p_inbound_quantity:

.. _i2p_outbound_quantity:

.. _i2p_inbound_length:

.. _i2p_outbound_length:

.. raw:: html

	<a name="i2p_inbound_quantity"></a>
	<a name="i2p_outbound_quantity"></a>
	<a name="i2p_inbound_length"></a>
	<a name="i2p_outbound_length"></a>

+-----------------------+------+---------+
| name                  | type | default |
+=======================+======+=========+
| i2p_inbound_quantity  | int  | 3       |
+-----------------------+------+---------+
| i2p_outbound_quantity | int  | 3       |
+-----------------------+------+---------+
| i2p_inbound_length    | int  | 3       |
+-----------------------+------+---------+
| i2p_outbound_length   | int  | 3       |
+-----------------------+------+---------+

Configures the SAM session
quantity of I2P inbound and outbound tunnels [1..16].
number of hops for I2P inbound and outbound tunnels [0..7]
Changing these will not trigger a reconnect to the SAM bridge,
they will take effect the next time the SAM connection is
re-established (by restarting or changing i2p_hostname or
i2p_port).

.. _announce_port:

.. raw:: html

	<a name="announce_port"></a>

+---------------+------+---------+
| name          | type | default |
+===============+======+=========+
| announce_port | int  | 0       |
+---------------+------+---------+

``announce_port`` is the port passed along as the ``port`` parameter
to remote trackers such as HTTP or DHT. This setting does not affect
the effective listening port nor local service discovery announcements.
If left as zero (default), the listening port value is used.

.. note::
   This setting is only meant for very special cases where a
   seed's listening port differs from the external port. As an
   example, if a local proxy is used and that the proxy supports
   reverse tunnels through NAT-PMP, the tracker must connect to
   the external NAT-PMP port (configured using ``announce_port``)
   instead of the actual local listening port.

