/* - ArrowDL - Copyright (C) 2019-present Sebastien Vavassori
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TORRENT_UTILS_H
#define TORRENT_UTILS_H

#include <Torrent/TorrentMessage>

#include "libtorrent/socket.hpp"            // lt::tcp::endpoint
#include "libtorrent/units.hpp"             // lt::download_priority_t
#include "libtorrent/torrent_status.hpp"    // lt::torrent_status::state_t

/*! Helper methods to convert std, boost and libtorrent objects to Qt */
class TorrentUtils
{
public:
    static UniqueId toUniqueId(const lt::sha1_hash &hash);
    static lt::sha1_hash fromUniqueId(const UniqueId &uuid);

    static TorrentInitialMetaInfo toTorrentInitialMetaInfo(std::shared_ptr<lt::torrent_info const> ti);
    static TorrentMetaInfo toTorrentMetaInfo(const lt::add_torrent_params &params);
    static TorrentHandleInfo toTorrentHandleInfo(const lt::torrent_handle &handle);

    static QString toString(const std::string &str);
    static QString toString(const lt::string_view &s);
    static QString toString(const lt::sha1_hash &hash);
    static QDateTime toDateTime(const std::time_t &time);

    static QBitArray toBitArray(const lt::typed_bitfield<lt::piece_index_t> &pieces);
    static QBitArray toBitArray(const std::map<lt::piece_index_t, lt::bitfield> &map);

    static EndPoint toEndPoint(const lt::tcp::endpoint &endpoint);
    static lt::tcp::endpoint fromEndPoint(const EndPoint &endpoint);

    static TorrentTrackerInfo::Source toTrackerSource(const lt::announce_entry::tracker_source &s);

    static TorrentError toTorrentError(const lt::error_code &errc, const lt::file_index_t &error_file);

    static TorrentTrackerInfo toTorrentTrackerInfo(const lt::announce_entry &tracker);
    static lt::announce_entry fromTorrentTrackerInfo(const TorrentTrackerInfo &tracker);

    static TorrentFileInfo::Priority toPriority(const lt::download_priority_t &p);
    static lt::download_priority_t fromPriority(const TorrentFileInfo::Priority &p);

    static TorrentInfo::TorrentState toState(const lt::torrent_status::state_t &s);
    static lt::torrent_status::state_t fromState(const TorrentInfo::TorrentState &s);
};

#endif // TORRENT_UTILS_H
