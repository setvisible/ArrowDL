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

#include "utils.h"

#include "libtorrent/add_torrent_params.hpp"
#include "libtorrent/hex.hpp"               // to_hex, from_hex
#include "libtorrent/magnet_uri.hpp"
#include "libtorrent/torrent_info.hpp"


UniqueId TorrentUtils::toUniqueId(const lt::sha1_hash &hash)
{
    if (!hash.is_all_zeros()) {
        auto hex = lt::aux::to_hex(hash);
        return QString::fromStdString(hex).toUpper();
    }
    return {};
}

lt::sha1_hash TorrentUtils::fromUniqueId(const UniqueId &uuid)
{
    lt::span<char const> in(uuid.toStdString());
    lt::sha1_hash out;
    if (!lt::aux::from_hex(in, out.data())) {
        qWarning() << "invalid info-hash";
        return lt::sha1_hash();
    }
    return out;
}

/******************************************************************************
 ******************************************************************************/
TorrentInitialMetaInfo TorrentUtils::toTorrentInitialMetaInfo(
    std::shared_ptr<lt::torrent_info const> ti)
{
    TorrentInitialMetaInfo m;
    if (!ti || !ti->is_valid()) {
        return m;
    }

    m.bytesTotal        = ti->total_size();
    m.pieceCount        = ti->num_pieces();
    m.pieceByteSize     = ti->piece_length();
    m.pieceLastByteSize = ti->piece_size(ti->last_piece());

    m.infohash      = toString(ti->info_hash());

    m.sslRootCertificate = toString(ti->ssl_cert());

    m.isPrivate     = ti->priv();
    m.isI2P         = ti->is_i2p();

    m.name          = toString(ti->name()); // name of the .torrent file

    m.creationDate  = toDateTime(ti->creation_date());
    m.creator       = toString(ti->creator());
    m.comment       = toString(ti->comment());
    m.magnetLink    = toString(lt::make_magnet_uri(*ti));

    for (auto node : ti->nodes()) {
        m.nodes.append( TorrentNodeInfo(toString(node.first), node.second) );
    }

    //m.bytesMetaData = ti->metadata_size();

    auto files = ti->files();
    for (auto index : files.file_range()) {

        TorrentFileMetaInfo f;

        auto flags = files.file_flags(index);
        if (flags & lt::file_storage::flag_pad_file)   f.setFlag(TorrentFileMetaInfo::Flag::PadFile);
        if (flags & lt::file_storage::flag_hidden)     f.setFlag(TorrentFileMetaInfo::Flag::Hidden);
        if (flags & lt::file_storage::flag_executable) f.setFlag(TorrentFileMetaInfo::Flag::Executable);
        if (flags & lt::file_storage::flag_symlink)    f.setFlag(TorrentFileMetaInfo::Flag::Symlink);

        f.hash              = toString(files.hash(index));

        if (f.flags.testFlag(TorrentFileMetaInfo::Flag::Symlink)) {
            f.symlink       = toString(files.symlink(index));
        }
        f.modifiedTime      = toDateTime(files.mtime(index));
        f.filePath          = toString(files.file_path(index)) ;
        f.fileName          = toString(files.file_name(index));
        f.bytesTotal        = static_cast<qsizetype>(files.file_size(index));
        f.isPadFile         = files.pad_file_at(index);
        f.bytesOffset       = static_cast<qsizetype>(files.file_offset(index));
        f.isPathAbsolute    = files.file_absolute_path(index);
        f.crc32FilePathHash = files.file_path_hash(index, std::string());

        m.files.append(f);
    }


    for (auto tracker : ti->trackers()) {
        auto t = toTorrentTrackerInfo(tracker);
        m.trackers.append(t);
    }

    for (auto similar_torrent : ti->similar_torrents()) {
        m.similarTorrents.append(toString(similar_torrent));
    }

    for (auto collection : ti->collections()) {
        m.collections.append(toString(collection));
    }

    for (auto web_seed : ti->web_seeds()) {
        TorrentWebSeedMetaInfo w;
        w.url   = toString(web_seed.url);
        w.auth  = toString(web_seed.auth);
        for (auto extra_header : web_seed.extra_headers) {
            w.extraHeaders.append(
                        QPair<QString,QString>(
                            toString(extra_header.first),
                            toString(extra_header.second)));
        }
        switch (web_seed.type) {
        case lt::web_seed_entry::url_seed:
            w.type = TorrentWebSeedMetaInfo::Type::UrlSeed;
            break;
        case lt::web_seed_entry::http_seed:
        default:
            w.type = TorrentWebSeedMetaInfo::Type::HttpSeed;
            break;
        }
        m.webSeeds.append(w);
    }
    return m;
}

TorrentHandleInfo TorrentUtils::toTorrentHandleInfo(const lt::torrent_handle &handle)
{
    //qDebug_2 << Q_FUNC_INFO;
    TorrentHandleInfo t;
    if (!handle.is_valid()) {
        return t;
    }

    t.uploadBandwidthLimit      = handle.upload_limit();
    t.downloadBandwidthLimit    = handle.download_limit();
    t.maxUploads                = handle.max_uploads();
    t.maxConnections            = handle.max_connections();

    // ***************
    // Files
    // ***************
    if (handle.torrent_file()) {

        std::vector<std::int64_t> progress;
        handle.file_progress(progress, lt::torrent_handle::piece_granularity);
        auto priorities = handle.get_file_priorities();

        // const std::vector<lt::open_file_state> file_status = handle.file_status();


        // auto list = std::initializer_list<int>();
        const qsizetype count = std::min(
                    { static_cast<qsizetype>(handle.torrent_file()->num_files()),
                      static_cast<qsizetype>(progress.size()),
                      static_cast<qsizetype>(priorities.size()) });

        for (auto index = 0; index < count; ++index) {
            auto findex = static_cast<lt::file_index_t>(index);
            TorrentFileInfo fi;
            fi.bytesReceived = static_cast<qsizetype>(progress.at(static_cast<std::size_t>(index)));
            fi.priority = toPriority(handle.file_priority(findex));
            t.files.append(fi);
        }
    }

    // ***************
    // Peers
    // ***************
    std::vector<lt::peer_info> peers;
    handle.get_peer_info(peers);
    for (auto peer : peers) {
        TorrentPeerInfo d;

        auto peerIp = toString(peer.ip.address().to_string());
        auto peerPort = peer.ip.port();
        d.endpoint = EndPoint(peerIp, peerPort);
        d.userAgent = toString(peer.client);

        d.availablePieces = toBitArray(peer.pieces);

        d.bytesDownloaded = peer.total_download;
        d.bytesUploaded = peer.total_upload;

        d.lastTimeRequested = peer.last_request.count();
        d.lastTimeActive    = peer.last_active.count();
        d.timeDownloadQueue = peer.download_queue_time.count();

        auto flags = peer.flags;
        if (flags & lt::peer_info::interesting)         d.setFlag(TorrentPeerInfo::Flag::Interesting);
        if (flags & lt::peer_info::choked)              d.setFlag(TorrentPeerInfo::Flag::Choked);
        if (flags & lt::peer_info::remote_interested)   d.setFlag(TorrentPeerInfo::Flag::RemoteInterested);
        if (flags & lt::peer_info::remote_choked)       d.setFlag(TorrentPeerInfo::Flag::RemoteChoked);
        if (flags & lt::peer_info::supports_extensions) d.setFlag(TorrentPeerInfo::Flag::SupportsExtensions);
        if (flags & lt::peer_info::local_connection)    d.setFlag(TorrentPeerInfo::Flag::LocalConnection);
        if (flags & lt::peer_info::handshake)           d.setFlag(TorrentPeerInfo::Flag::Handshake);
        if (flags & lt::peer_info::connecting)          d.setFlag(TorrentPeerInfo::Flag::Connecting);
        // if (flags & lt::peer_info::queued)           d.setFlag(TorrentPeerInfo::Flag::Queued);
        if (flags & lt::peer_info::on_parole)           d.setFlag(TorrentPeerInfo::Flag::OnParole);
        if (flags & lt::peer_info::seed)                d.setFlag(TorrentPeerInfo::Flag::Seed);
        if (flags & lt::peer_info::optimistic_unchoke)  d.setFlag(TorrentPeerInfo::Flag::OptimisticUnchoke);
        if (flags & lt::peer_info::snubbed)             d.setFlag(TorrentPeerInfo::Flag::Snubbed);
        if (flags & lt::peer_info::upload_only)         d.setFlag(TorrentPeerInfo::Flag::UploadOnly);
        if (flags & lt::peer_info::endgame_mode)        d.setFlag(TorrentPeerInfo::Flag::Endgame_Mode);
        if (flags & lt::peer_info::holepunched)         d.setFlag(TorrentPeerInfo::Flag::Holepunched);
        if (flags & lt::peer_info::i2p_socket)          d.setFlag(TorrentPeerInfo::Flag::I2pSocket);
        if (flags & lt::peer_info::utp_socket)          d.setFlag(TorrentPeerInfo::Flag::UtpSocket);
        if (flags & lt::peer_info::ssl_socket)          d.setFlag(TorrentPeerInfo::Flag::SslSocket);
        if (flags & lt::peer_info::rc4_encrypted)       d.setFlag(TorrentPeerInfo::Flag::Rc4Encrypted);
        if (flags & lt::peer_info::plaintext_encrypted) d.setFlag(TorrentPeerInfo::Flag::Plaintextencrypted);

        auto sourceFlags = peer.source;
        if (sourceFlags & lt::peer_info::tracker)     d.setSourceFlag(TorrentPeerInfo::SourceFlag::FromTracker);
        if (sourceFlags & lt::peer_info::dht)         d.setSourceFlag(TorrentPeerInfo::SourceFlag::FromDHT);
        if (sourceFlags & lt::peer_info::pex)         d.setSourceFlag(TorrentPeerInfo::SourceFlag::FromPeerExchange);
        if (sourceFlags & lt::peer_info::lsd)         d.setSourceFlag(TorrentPeerInfo::SourceFlag::FromLocalServiceDiscovery);
        if (sourceFlags & lt::peer_info::resume_data) d.setSourceFlag(TorrentPeerInfo::SourceFlag::FromFastResumeData);
        if (sourceFlags & lt::peer_info::incoming)    d.setSourceFlag(TorrentPeerInfo::SourceFlag::FromPeerIncomingData);

        t.peers.append(d);
    }

    // ***************
    // Trackers
    // ***************
    auto trackers = handle.trackers();
    for (auto tracker : trackers) {
        auto ti = toTorrentTrackerInfo(tracker);
        t.trackers.append(ti);
    }

    // ***************
    // Seeds
    // ***************
    for (auto webSeed : handle.http_seeds()) {
        t.httpSeeds.append(toString(webSeed));
    }
    for (auto webSeed : handle.url_seeds()) {
        t.urlSeeds.append(toString(webSeed));
    }

    // ***************
    // Blocks
    // ***************
    // std::vector<lt::partial_piece_info> queue;
    // handle.get_download_queue(queue);

    // ***************
    // Pieces
    // ***************
    {
        std::vector<int> avail;
        handle.piece_availability(avail);
        t.pieceAvailability = QVector<int>(avail.begin(), avail.end());
    }

    {
        auto priorities = handle.get_piece_priorities();
        QVector<TorrentFileInfo::Priority> newPiecePriority;
        for (auto priority : priorities) {
            newPiecePriority.append(toPriority(priority));
        }
        t.piecePriority.swap(newPiecePriority);
    }

    return t;
}

TorrentMetaInfo TorrentUtils::toTorrentMetaInfo(const lt::add_torrent_params &params)
{
    TorrentMetaInfo m;

    auto ti = params.ti;

    m.initialMetaInfo = toTorrentInitialMetaInfo(ti);
    for (auto tracker : params.trackers) {
        m.trackers2.append(toString(tracker));
    }
    // for (const int tracker_tier : params.tracker_tiers) {
    // ?
    // }
    for (auto dht_node : params.dht_nodes) {
        m.dhtNodes.append( TorrentNodeInfo(toString(dht_node.first), dht_node.second) );
    }

    if (m.initialMetaInfo.name.isEmpty()) {
        m.initialMetaInfo.name = toString(params.name);
    }
    m.outputPath    = toString(params.save_path);

    // for (const lt : :download_priority_t &file_priority : params.file_priorities) {
    // ?
    // }

    m.defaultTrackerId = toString(params.trackerid);
    //  m.flags         = toString(params.flags); // TODO

    if (m.initialMetaInfo.infohash.isEmpty()) {
        m.initialMetaInfo.infohash = toString(params.info_hashes.get_best());
    }

    m.maxUploads     = params.max_uploads;
    m.maxConnections = params.max_connections;

    m.uploadBandwidthLimit   = params.upload_limit;
    m.downloadBandwidthLimit = params.download_limit;

    m.bytesTotalUploaded   = params.total_uploaded;
    m.bytesTotalDownloaded = params.total_downloaded;


    m.activeTimeDuration   = params.active_time;
    m.finishedTimeDuration = params.finished_time;
    m.seedingTimeDuration  = params.seeding_time;

    m.addedTime             = toDateTime(params.added_time);
    m.completedTime         = toDateTime(params.completed_time);
    m.lastSeenCompletedTime = toDateTime(params.last_seen_complete);

    m.seedsInSwarm          = params.num_complete;
    m.peersInSwarm          = params.num_incomplete;
    m.downloadsInSwarm      = params.num_downloaded;

    for (auto http_seed : params.http_seeds) { // todo unify http et url
        m.httpSeeds.append(toString(http_seed));
    }

    for (auto url_seed : params.url_seeds) {
        m.urlSeeds.append(toString(url_seed));

    }

    for (auto peer : params.peers) {
        TorrentPeerInfo p(toEndPoint(peer), QString());
        m.defaultPeers.append(p);
    }

    for (auto banned_peer : params.banned_peers) {
        TorrentPeerInfo p(toEndPoint(banned_peer), QString());
        m.bannedPeers.append(p);
    }

    m.unfinishedPieces = toBitArray(params.unfinished_pieces);
    m.downloadedPieces = toBitArray(params.have_pieces);
    m.verifiedPieces   = toBitArray(params.verified_pieces);

    m.lastTimeDownload = toDateTime(params.last_download);
    m.lastTimeUpload   = toDateTime(params.last_upload);

    return m;
}

QString TorrentUtils::toString(const std::string &s)
{
    return QString::fromStdString(s);
}

QString TorrentUtils::toString(const lt::string_view &s)
{
    return QString::fromStdString(s.to_string()).toUpper();
}

QString TorrentUtils::toString(const lt::sha1_hash &hash)
{
    if (!hash.is_all_zeros()) {
        auto hex = lt::aux::to_hex(hash);
        return QString::fromStdString(hex).toUpper();
    }
    return {};
}

QBitArray TorrentUtils::toBitArray(const lt::typed_bitfield<lt::piece_index_t> &vec)
{
    auto size = vec.size();
    QBitArray ba(size, false);
    for (auto i = 0; i < size; ++i) {
        if (vec.get_bit(static_cast<lt::piece_index_t>(i))) {
            ba.setBit(i);
        }
    }
    return ba;
}

QBitArray TorrentUtils::toBitArray(const std::map<lt::piece_index_t, lt::bitfield> &map)
{
    int size = 0;
    QList<int> indexes;
    for (auto kv : map) {
        auto key = kv.first;
        auto index = static_cast<int>(key);
        indexes.append(index);
        size = qMax(size, index);
    }
    QBitArray ba(size, false);
    for (auto index : indexes) {
        ba.setBit(index);
    }
    return ba;
}

QDateTime TorrentUtils::toDateTime(const std::time_t &time)
{
    auto sec = static_cast<qint64>(time);
    if (sec > 0) {
        return QDateTime(QDate(1970, 1, 1), QTime(0, 0), Qt::UTC).addSecs(sec);
    }
    return {};
}

/******************************************************************************
 ******************************************************************************/
EndPoint TorrentUtils::toEndPoint(const lt::tcp::endpoint &endpoint)
{
    auto ip = QString::fromStdString(endpoint.address().to_string());
    auto port = endpoint.port();
    return EndPoint(ip, port);
}

lt::tcp::endpoint TorrentUtils::fromEndPoint(const EndPoint &endpoint)
{
    lt::tcp::endpoint ret;
    ret.address(boost::asio::ip::make_address(endpoint.ipToString().toStdString()));
    ret.port(static_cast<unsigned short>(endpoint.port()));
    return ret;
}

/******************************************************************************
 ******************************************************************************/
TorrentTrackerInfo::Source TorrentUtils::toTrackerSource(const lt::announce_entry::tracker_source &s)
{
    switch (s) {
    case lt::announce_entry::source_torrent:     return TorrentTrackerInfo::Source::TorrentFile;
    case lt::announce_entry::source_client:      return TorrentTrackerInfo::Source::Client;
    case lt::announce_entry::source_magnet_link: return TorrentTrackerInfo::Source::MagnetLink;
    case lt::announce_entry::source_tex:         return TorrentTrackerInfo::Source::TrackerExchange;
    default:                                     return TorrentTrackerInfo::Source::NoSource;
    }
}

/******************************************************************************
 ******************************************************************************/
TorrentError TorrentUtils::toTorrentError(const lt::error_code &errc, const lt::file_index_t &error_file)
{
    TorrentError error;
    bool hasError = (static_cast<int>(errc.value()) != 0);
    if (!hasError) {
        error = {};
    } else {
        error.message = QString::fromStdString(errc.message());

        //qDebug_2 << Q_FUNC_INFO << error.message;

        if (error_file == lt::torrent_status::error_file_none) { /* -1 */
            // Other error type (not file error)
            error = TorrentError(TorrentError::UnknownError);

        } else if (error_file == lt::torrent_status::error_file_ssl_ctx) {
            error = TorrentError(TorrentError::SSLContextError);

        } else if (error_file == lt::torrent_status::error_file_metadata) {
            error = TorrentError(TorrentError::FileMetadataError);

        } else if (error_file == lt::torrent_status::error_file_exception) {
            error = TorrentError(TorrentError::FileExceptionError);

        } else if (error_file == lt::torrent_status::error_file_partfile) {
            error = TorrentError(TorrentError::PartFileError);

        } else {
            const int errorFileIndex = static_cast<int>(error_file);
            if (errorFileIndex >= 0) {
                error = TorrentError(TorrentError::FileError, errorFileIndex);
            }
        }
    }
    return error;
}

/******************************************************************************
 ******************************************************************************/
TorrentTrackerInfo TorrentUtils::toTorrentTrackerInfo(const lt::announce_entry &tracker)
{
    TorrentTrackerInfo t(QString::fromStdString(tracker.url));
    t.trackerId     = QString::fromStdString(tracker.trackerid);
    for (auto endpoint : tracker.endpoints) {
        EndPoint e = toEndPoint(endpoint.local_endpoint);
        t.endpoints.append(e);
    }
    t.tier          = static_cast<int>(tracker.tier);
    t.failLimit     = static_cast<int>(tracker.fail_limit);
    t.isVerified    = tracker.verified;
    t.source        = toTrackerSource(static_cast<lt::announce_entry::tracker_source>(tracker.source));
    return t;
}

lt::announce_entry TorrentUtils::fromTorrentTrackerInfo(const TorrentTrackerInfo &tracker)
{
    lt::announce_entry ret;
    ret.url         = tracker.url.toStdString();
    ret.trackerid   = tracker.trackerId.toStdString();
    ret.tier        = static_cast<std::uint8_t>(tracker.tier);
    ret.fail_limit  = static_cast<std::uint8_t>(tracker.failLimit);
    ret.verified    = tracker.isVerified;
    return ret;
}

/******************************************************************************
 ******************************************************************************/
TorrentFileInfo::Priority TorrentUtils::toPriority(const lt::download_priority_t &p)
{
    if      (p == lt::dont_download   )  return TorrentFileInfo::Priority::Ignore;
    else if (p == lt::low_priority    )  return TorrentFileInfo::Priority::Low;
    else if (p == lt::top_priority    )  return TorrentFileInfo::Priority::High;
    else   /*p == lt::default_priority*/ return TorrentFileInfo::Priority::Normal;
}

lt::download_priority_t TorrentUtils::fromPriority(const TorrentFileInfo::Priority &p)
{
    switch (p) {
    case TorrentFileInfo::Priority::Ignore: return lt::dont_download;
    case TorrentFileInfo::Priority::Low:    return lt::low_priority;
    case TorrentFileInfo::Priority::High:   return lt::top_priority;
    case TorrentFileInfo::Priority::Normal: return lt::default_priority;
    }
    Q_UNREACHABLE();
}

/******************************************************************************
 ******************************************************************************/
TorrentInfo::TorrentState TorrentUtils::toState(const lt::torrent_status::state_t &s)
{
    if      (s == lt::torrent_status::checking_files        ) return TorrentInfo::checking_files        ;
    else if (s == lt::torrent_status::downloading_metadata  ) return TorrentInfo::downloading_metadata  ;
    else if (s == lt::torrent_status::downloading           ) return TorrentInfo::downloading           ;
    else if (s == lt::torrent_status::finished              ) return TorrentInfo::finished              ;
    else if (s == lt::torrent_status::seeding               ) return TorrentInfo::seeding               ;
    else /* s == lt::torrent_status:: */ return TorrentInfo::stopped;
    Q_UNREACHABLE();
}

lt::torrent_status::state_t TorrentUtils::fromState(const TorrentInfo::TorrentState &s)
{
    switch (s) {
    case TorrentInfo::stopped               : return lt::torrent_status::finished; // ?
    case TorrentInfo::checking_files        : return lt::torrent_status::checking_files;
    case TorrentInfo::downloading_metadata  : return lt::torrent_status::downloading_metadata;
    case TorrentInfo::downloading           : return lt::torrent_status::downloading;
    case TorrentInfo::finished              : return lt::torrent_status::finished;
    case TorrentInfo::seeding               : return lt::torrent_status::seeding;
    case TorrentInfo::checking_resume_data  : return lt::torrent_status::checking_resume_data;
    }
    Q_UNREACHABLE();
}
