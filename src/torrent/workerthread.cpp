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

#include "workerthread.h"

#include <Torrent/Utils>

#include <chrono>

#include "libtorrent/alert_types.hpp"
#include "libtorrent/session.hpp"

#ifndef TORRENT_DISABLE_EXTENSIONS
#   include <libtorrent/extensions/smart_ban.hpp>
#   include <libtorrent/extensions/ut_metadata.hpp>
#   include <libtorrent/extensions/ut_pex.hpp>
#endif

/* Show the thread, of which the debug message comes from */
#define qDebug_1 qDebug() << " + | " // from TorrentContextPrivate
#define qDebug_2 qDebug() << " | + " // from WorkerThread


static const lt::status_flags_t s_torrent_status_flags =
    lt::torrent_handle::query_distributed_copies
    | lt::torrent_handle::query_accurate_download_counters
    | lt::torrent_handle::query_last_seen_complete
    | lt::torrent_handle::query_pieces
    | lt::torrent_handle::query_verified_pieces
    ;

const std::chrono::milliseconds TIMEOUT_REFRESH(500);


WorkerThread::WorkerThread(QObject *parent) : QThread(parent)
  , m_session_ptr(new lt::session())
{
}

/******************************************************************************
 ******************************************************************************/
void WorkerThread::stop()
{
    shouldQuit = true;
}

/******************************************************************************
 ******************************************************************************/
bool WorkerThread::isEnabled() const
{
    Q_ASSERT(m_session_ptr);
    if (m_session_ptr && m_session_ptr->is_valid()) {
        return !m_session_ptr->is_paused();
    }
    return false;
}

void WorkerThread::setEnabled(bool enabled)
{
    qDebug_2 << Q_FUNC_INFO << enabled;
    Q_ASSERT(m_session_ptr);
    if (m_session_ptr && m_session_ptr->is_valid()) {
        if (enabled) {
            m_session_ptr->resume(); // ok async call
        } else {
            m_session_ptr->pause();
        }
    }
}

/******************************************************************************
 ******************************************************************************/
lt::settings_pack WorkerThread::settings() const
{
    Q_ASSERT(m_session_ptr);
    if (m_session_ptr && m_session_ptr->is_valid()) {
        return m_session_ptr->get_settings();
    }
    return lt::default_settings();
}

void WorkerThread::setSettings(lt::settings_pack &pack)
{
    Q_ASSERT(m_session_ptr);
    if (m_session_ptr && m_session_ptr->is_valid()) {

        // Settings that can't be modified by the user
        pack.set_str(lt::settings_pack::user_agent, std::string());
        pack.set_int(lt::settings_pack::alert_mask, lt::alert::all_categories);

        m_session_ptr->apply_settings(pack);
    }
}

/******************************************************************************
 ******************************************************************************/
TorrentInitialMetaInfo WorkerThread::dump(const QString &filename) const
{
    lt::error_code error_code;
    const lt::torrent_info torrent_info(filename.toStdString(), error_code);
    if (error_code) {
        // qWarning() << "failed to decode file '"
        //            << filename
        //            << "' due to"
        //            << QString::fromStdString(error_code.message());
        return {};
    }
    auto ptr_torrent_info= std::make_shared<lt::torrent_info>(torrent_info);
    return TorrentUtils::toTorrentInitialMetaInfo(ptr_torrent_info);
}

/******************************************************************************
 ******************************************************************************/
lt::torrent_handle WorkerThread::addTorrent(lt::add_torrent_params const& params,
                                            lt::error_code& ec)
{
    qDebug_2 << Q_FUNC_INFO;
    Q_ASSERT(m_session_ptr);
    if (m_session_ptr && m_session_ptr->is_valid()) {
        return m_session_ptr->add_torrent(params, ec);
    }
    return {};
}

/******************************************************************************
 ******************************************************************************/
void WorkerThread::removeTorrent(const lt::torrent_handle& h, lt::remove_flags_t options)
{
    qDebug_2 << Q_FUNC_INFO;
    Q_ASSERT(m_session_ptr);
    if (m_session_ptr && m_session_ptr->is_valid()) {
        m_session_ptr->remove_torrent(h, options);
    }
}

/******************************************************************************
 ******************************************************************************/
lt::torrent_handle WorkerThread::findTorrent(const UniqueId &uuid) const
{
    qDebug_2 << Q_FUNC_INFO;
    Q_ASSERT(m_session_ptr);
    if (m_session_ptr && m_session_ptr->is_valid()) {
        auto hash = TorrentUtils::fromUniqueId(uuid);
        return m_session_ptr->find_torrent(hash);
    }
    return lt::torrent_handle();
}

/******************************************************************************
 ******************************************************************************/
void WorkerThread::run()
{
    qDebug_2 << Q_FUNC_INFO;

    lt::session& session = *m_session_ptr;

    lt::session_params params;
    auto& settings = params.settings;
    setSettings(settings);


#ifndef TORRENT_DISABLE_EXTENSIONS
    // smart ban plugin
    // 	A plugin that, with a small overhead, can ban peers
    // 	that sends bad data with very high accuracy. Should
    // 	eliminate most problems on poisoned torrents.
    session.add_extension(&lt::create_smart_ban_plugin);

    // uTorrent metadata
    // 	Allows peers to download the metadata (.torrent files) from the swarm
    // 	directly. Makes it possible to join a swarm with just a tracker and
    // 	info-hash.
    session.add_extension(&lt::create_ut_metadata_plugin);

    // uTorrent peer exchange
    // 	Exchanges peers between clients.
    session.add_extension(&lt::create_ut_pex_plugin);

#endif

//#ifndef TORRENT_DISABLE_DHT
//    if (settings.has_val(lt::settings_pack::dht_upload_rate_limit)) {
//        auto key = lt::settings_pack::dht_upload_rate_limit;
//        params.settings.dht_upload_rate_limit
//        params.dht_settings.upload_rate_limit = params.settings.get_int(key);
//    }
//    session.set_dht_settings(std::move(params.dht_settings));
//
//    TORRENT_ASSERT(params.dht_storage_constructor);
//    session.set_dht_storage(std::move(params.dht_storage_constructor));
//#endif

    lt::ip_filter loaded_ip_filter;
    session.set_ip_filter(loaded_ip_filter);

    session.pause();

    // main loop
    while (!shouldQuit) {
        session.post_torrent_updates(s_torrent_status_flags);
        session.post_session_stats();
        session.post_dht_stats();

        QThread::msleep(TIMEOUT_REFRESH.count());

        std::vector<lt::alert*> alerts;
        session.pop_alerts(&alerts);
        for (auto a : alerts) {
            signalizeAlert(a);
        }
    } // end of main loop

    qDebug_2 << Q_FUNC_INFO << "Closing session... ";

    /*
     * The session destructor is blocking by default
     * this allows shutting down asynchrounously
     */
    lt::session_proxy proxy = session.abort();

    /*
     * Don't delete m_session_ptr, instead nullify the pointer.
     * libtorrent will desallocate the handle.
     */
    m_session_ptr = nullptr;

    emit stopped();
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Convert lt::alert to QSignal
 */
void WorkerThread::signalizeAlert(lt::alert* a)
{
    /* status_notification */
    if (auto s = lt::alert_cast<lt::torrent_removed_alert>(a)) {
        //QString hash = toString(s->info_hashes.get_best());
        //  emit torrentRemoved(hash);
    }
    else if (auto s = lt::alert_cast<lt::state_changed_alert>(a)) {
        //        lt::torrent_status::state_t oldStatus = s->prev_state;
        //        lt::torrent_status::state_t newStatus = s->state;
        //  emit torrentStateChanged(oldState, newState); // rem quel torrent ?
    }
    else if (auto s = lt::alert_cast<lt::hash_failed_alert>(a)) {
        //        int piece_index = (int)s->piece_index;
        //  emit pieceHashCheckFailed(piece_index);
    }
    else if (auto s = lt::alert_cast<lt::torrent_finished_alert>(a)) {
        //        emit torrentFinished();
    }
    else if (auto s = lt::alert_cast<lt::torrent_paused_alert>(a)) {
        //        emit torrentPaused();
    }
    else if (auto s = lt::alert_cast<lt::torrent_resumed_alert>(a)) {
        //        emit torrentResumed();
    }
    else if (auto s = lt::alert_cast<lt::torrent_checked_alert>(a)) {
        //        emit torrentChecked();
    }
    else if (auto s = lt::alert_cast<lt::fastresume_rejected_alert>(a)) {
        //        emit torrentFastResumeFailed();
    }
    else if (auto s = lt::alert_cast<lt::trackerid_alert>(a)) {
        //        emit trackeridReceived();
    }
    else if (auto s = lt::alert_cast<lt::torrent_error_alert>(a)) {
        //        emit torrentError();
    }
    else if (auto s = lt::alert_cast<lt::torrent_need_cert_alert>(a)) {
        //        emit torrentSSLError();
    }
    else if (auto s = lt::alert_cast<lt::add_torrent_alert>(a)) {
        onTorrentAdded(s->handle, s->params, s->error);
    }
    else if (auto s = lt::alert_cast<lt::state_update_alert>(a)) {
        /* Note: This alert is emitted very often (each loop) */
        // log(s);
        onStateUpdated(s->status);
    }

    // magnet extension
    else if (auto s = lt::alert_cast<lt::metadata_failed_alert>(a)) {
        //        emit metadataFailed();
        qWarning() << "metadata that was received was corrupt";
    }
    else if (auto s = lt::alert_cast<lt::metadata_received_alert>(a)) {
        onMetadataReceived(s->handle);
    }


    // IP
    else if (auto s = lt::alert_cast<lt::external_ip_alert>(a)) {
        Q_UNUSED(s) //  emit externalIpAddressReceived();
    }
    else if (auto s = lt::alert_cast<lt::peer_blocked_alert>(a)) {
        Q_UNUSED(s) //  emit peerIpBlocked();
    }
    // dht
    else if (auto s = lt::alert_cast<lt::dht_announce_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHT();
    }
    else if (auto s = lt::alert_cast<lt::dht_get_peers_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHTReceviedd();
    }
    else if (auto s = lt::alert_cast<lt::dht_bootstrap_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHTbootstrapDone();
    }
    else if (auto s = lt::alert_cast<lt::dht_error_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHTError();
    }
    else if (auto s = lt::alert_cast<lt::dht_immutable_item_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHTImmmutablee();
    }
    else if (auto s = lt::alert_cast<lt::dht_mutable_item_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHTmutablee();
    }
    else if (auto s = lt::alert_cast<lt::dht_put_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHTput();
    }
    else if (auto s = lt::alert_cast<lt::i2p_alert>(a)) {
        Q_UNUSED(s) //  emit i2pError();
    }
    else if (auto s = lt::alert_cast<lt::dht_outgoing_get_peers_alert>(a)) {
        Q_UNUSED(s) //  emit dht_get_peers_Sent();
    }



    // Firewall error ?
    else if (auto s = lt::alert_cast<lt::udp_error_alert>(a)) {
        Q_UNUSED(s) //  emit protocolError();
    }
    else if (auto s = lt::alert_cast<lt::listen_succeeded_alert>(a)) {
        Q_UNUSED(s) //  emit hostPortOpened();
    }
    else if (auto s = lt::alert_cast<lt::listen_failed_alert>(a)) {
        Q_UNUSED(s) //  emit hostPortOpenFailed();
    }
    else if (auto s = lt::alert_cast<lt::portmap_alert>(a)) {
        Q_UNUSED(s) //  emit portOpened();
    }
    else if (auto s = lt::alert_cast<lt::portmap_error_alert>(a)) {
        Q_UNUSED(s) //  emit portOpenFailed();
    }
    else if (auto s = lt::alert_cast<lt::portmap_log_alert>(a)) {
        Q_UNUSED(s) //  emit debugPort();
    }
    else if (auto s = lt::alert_cast<lt::log_alert>(a)) {
        Q_UNUSED(s) //  emit debugLog();
    }
    else if (auto s = lt::alert_cast<lt::torrent_log_alert>(a)) {
        Q_UNUSED(s) //  emit debugTorrentLog();
    }
    else if (auto s = lt::alert_cast<lt::peer_log_alert>(a)) {
        Q_UNUSED(s) //  emit debugPeerLog();
    }
    else if (auto s = lt::alert_cast<lt::lsd_error_alert>(a)) {
        Q_UNUSED(s) //  emit debugLSDPeerLog();
    }
    else if (auto s = lt::alert_cast<lt::dht_log_alert>(a)) {
        Q_UNUSED(s) //  emit debugDHTLog();
    }
    else if (auto s = lt::alert_cast<lt::dht_pkt_alert>(a)) {
        Q_UNUSED(s) //  emit dht_pkt_alert();
    }
    else if (auto s = lt::alert_cast<lt::dht_get_peers_reply_alert>(a)) {
        Q_UNUSED(s) //  emit dht_get_peers_reply_alert();
    }
    else if (auto s = lt::alert_cast<lt::dht_direct_response_alert>(a)) {
        Q_UNUSED(s) //  emit dht_direct_response_alert();
    }
    else if (auto s = lt::alert_cast<lt::picker_log_alert>(a)) {
        Q_UNUSED(s) //  emit debugPickerLog();
    }
    else if (auto s = lt::alert_cast<lt::session_error_alert>(a)) {
        Q_UNUSED(s) //  emit session_error_alert();
    }
    else if (auto s = lt::alert_cast<lt::dht_live_nodes_alert>(a)) {
        Q_UNUSED(s) //  emit dht_live_nodes_alert();
    }
    else if (auto s = lt::alert_cast<lt::dht_sample_infohashes_alert>(a)) {
        Q_UNUSED(s) //  emit dht_sample_infohashes_alert();
    }


    /* stats_notification */
    else if (auto s = lt::alert_cast<lt::session_stats_alert>(a)) {
        //        const lt::span<std::int64_t const> counters = s->counters() ;

        // TODO

        Q_UNUSED(s) //  emit sessionStatsUpdated();
    }
    else if (auto s = lt::alert_cast<lt::dht_stats_alert>(a)) {
        Q_UNUSED(s) //  emit dhtStatsUpdated();
    }
    else if (auto s = lt::alert_cast<lt::session_stats_header_alert>(a)) {
        Q_UNUSED(s) //  emit session_stats_header_Updated();
    }


    /* storage_notification */
    else if (auto s = lt::alert_cast<lt::read_piece_alert>(a)) {
        Q_UNUSED(s) // emit fileCompleted(index);
    }
    else if (auto s = lt::alert_cast<lt::file_prio_alert>(a)) {
        Q_UNUSED(s)
    }
    else if (auto s = lt::alert_cast<lt::oversized_file_alert>(a)) {
        Q_UNUSED(s)
    }
    else if (auto s = lt::alert_cast<lt::torrent_conflict_alert>(a)) {
        Q_UNUSED(s)
    }
    else if (auto s = lt::alert_cast<lt::peer_info_alert>(a)) {
        Q_UNUSED(s)
    }
    else if (auto s = lt::alert_cast<lt::file_progress_alert>(a)) {
        Q_UNUSED(s)
    }
    else if (auto s = lt::alert_cast<lt::piece_info_alert>(a)) {
        Q_UNUSED(s)
    }
    else if (auto s = lt::alert_cast<lt::piece_availability_alert>(a)) {
        Q_UNUSED(s)
    }
    else if (auto s = lt::alert_cast<lt::tracker_list_alert>(a)) {
        Q_UNUSED(s)
    }

    else if (auto s = lt::alert_cast<lt::file_renamed_alert>(a)) {
        //        int index = (int)s->index;
        //        QString newName = QString::fromUtf8(s->new_name());
        Q_UNUSED(s) //  emit fileRenamed(index, newName);
    }
    else if (auto s = lt::alert_cast<lt::file_rename_failed_alert>(a)) {
        //        int index = (int)s->index;
        //        QString errorMessage = QString::fromStdString(s->error.message());
        Q_UNUSED(s) //  emit fileRenameFailed(index, errorMessage);
    }
    else if (auto s = lt::alert_cast<lt::storage_moved_alert>(a)) {
        Q_UNUSED(s) //  emit storageMoved();
    }
    else if (auto s = lt::alert_cast<lt::storage_moved_failed_alert>(a)) {
        Q_UNUSED(s) //  emit storageMoveFailed();
    }
    else if (auto s = lt::alert_cast<lt::torrent_deleted_alert>(a)) {
        Q_UNUSED(s) //  emit torrentDeleted();
    }
    else if (auto s = lt::alert_cast<lt::torrent_delete_failed_alert>(a)) {
        Q_UNUSED(s) //  emit torrentDeleteFailed();
    }
    else if (auto s = lt::alert_cast<lt::save_resume_data_alert>(a)) {
        Q_UNUSED(s) //  emit resumeDataSaved();

    } else if (auto s = lt::alert_cast<lt::save_resume_data_failed_alert>(a)) {
        Q_UNUSED(s) //  emit resumeDataSaveFailed();
    }
    else if (auto s = lt::alert_cast<lt::file_error_alert>(a)) {
        Q_UNUSED(s) //  emit fileReadOrWriteError();
    }
    else if (auto s = lt::alert_cast<lt::cache_flushed_alert>(a)) {
        Q_UNUSED(s) //  emit cache_flushed();
    }




    /* file_progress_notification */
    else if (auto s = lt::alert_cast<lt::file_completed_alert>(a)) {
        //        int index = (int)s->index;
        Q_UNUSED(s) //  emit fileCompleted(index);
    }

    /* performance_warning */
    else if (auto s = lt::alert_cast<lt::performance_alert>(a)) {
        //        QString message = QString::fromStdString(s->message());
        Q_UNUSED(s) //  emit performanceWarning(message);
    }

    /* tracker_notification & error_notification */
    else if (auto s = lt::alert_cast<lt::tracker_error_alert>(a)) {
        //        int times_in_row = s->times_in_row;
        Q_UNUSED(s) //  emit trackerConnectionFailed(times_in_row, errorCode);
    }
    else if (auto s = lt::alert_cast<lt::tracker_warning_alert>(a)) {
        //        QString message = QString::fromUtf8(s->warning_message());
        Q_UNUSED(s) //  emit trackerMessageReceived(message);
    }
    else if (auto s = lt::alert_cast<lt::scrape_reply_alert>(a)) {
        //        int incomplete = s->incomplete;
        //        int complete = s->complete;
        Q_UNUSED(s) //  emit trackerScrapeSucceeded(incomplete, complete);
    }
    else if (auto s = lt::alert_cast<lt::scrape_failed_alert>(a)) {
        //        QString errorCode = QString::fromStdString(s->error.message());
        //        QString errorMessage = QString::fromUtf8(s->error_message());
        Q_UNUSED(s) //  emit trackerScrapeFailed(errorCode, errorMessage);
    }
    else if (auto s = lt::alert_cast<lt::tracker_reply_alert>(a)) {
        //        int peersCount = s->num_peers;
        Q_UNUSED(s) //  emit trackerInfo(peersCount);
    }
    else if (auto s = lt::alert_cast<lt::dht_reply_alert>(a)) {
        //        int peersCount = s->num_peers;
        Q_UNUSED(s) //  emit trackerDHTInfo(peersCount);
    }
    else if (auto s = lt::alert_cast<lt::tracker_announce_alert>(a)) {
        //        int eventcode = s->event;
        Q_UNUSED(s) //  emit trackerEventSent(eventcode);
    }

    /* peer_notification */
    else if (auto s = lt::alert_cast<lt::peer_ban_alert>(a)) {
        Q_UNUSED(s) //  emit peerBanned();
    }
    else if (auto s = lt::alert_cast<lt::peer_unsnubbed_alert>(a)) {
        Q_UNUSED(s) //  emit peerUnsnubbed();
    }
    else if (auto s = lt::alert_cast<lt::peer_snubbed_alert>(a)) {
        Q_UNUSED(s) //  emit peerSnubbed();
    }
    else if (auto s = lt::alert_cast<lt::peer_error_alert>(a)) {
        //        QString operation = QString::fromUtf8(lt::operation_name(s->op));
        //        QString errorCode = QString::fromStdString(s->error.message());
        Q_UNUSED(s) //  emit peerAboutToDisconnected(operation, errorCode);
    }
    else if (auto s = lt::alert_cast<lt::peer_connect_alert>(a)) {
        Q_UNUSED(s)
        // don't log every peer we try to connect to
        return;
        //  int socket_type = s->socket_type;
        //  emit peerConnected(socket_type);
    }
    else if (auto s = lt::alert_cast<lt::peer_disconnected_alert>(a)) {
        //        int socket_type = s->socket_type;
        //        operation_t const op;
        //		error_code const error;
        //		close_reason_t const reason;
        Q_UNUSED(s) //  emit peerDisconnected(/*socket_type*/);
    }
    else if (auto s = lt::alert_cast<lt::invalid_request_alert>(a)) {
        //        peer_request const request;
        //		bool const we_have;
        //		bool const peer_interested;
        //		bool const withheld;
        Q_UNUSED(s) //  emit peerInvalidPieceReceived();
    }
    else if (auto s = lt::alert_cast<lt::piece_finished_alert>(a)) {
        Q_UNUSED(s) //  emit pieceFinished();
    }
    else if (auto s = lt::alert_cast<lt::lsd_peer_alert>(a)) {
        Q_UNUSED(s) //  emit lsd_peer();
    }
    else if (auto s = lt::alert_cast<lt::incoming_connection_alert>(a)) {
        Q_UNUSED(s) //  emit peerIncomingConnectionAccepted();
    }
    else if (auto s = lt::alert_cast<lt::incoming_request_alert>(a)) {
        Q_UNUSED(s) //  emit peerIncomingrequestAccepted();
    }


    /* block_progress_notification */
    else if (auto s = lt::alert_cast<lt::request_dropped_alert>(a)) {
        Q_UNUSED(s) //  emit peerPieceRejected();
    }
    else if (auto s = lt::alert_cast<lt::block_timeout_alert>(a)) {
        Q_UNUSED(s) //  emit peerPieceTimeOut();
    }
    else if (auto s = lt::alert_cast<lt::block_finished_alert>(a)) {
        Q_UNUSED(s) //  emit peerPieceFinished();
    }
    else if (auto s = lt::alert_cast<lt::block_downloading_alert>(a)) {
        Q_UNUSED(s) //  emit peerPieceDownloading();
    }
    else if (auto s = lt::alert_cast<lt::unwanted_block_alert>(a)) {
        Q_UNUSED(s) //  emit peerPieceUnwanted();
    }

    else if (auto s = lt::alert_cast<lt::url_seed_alert>(a)) {
        Q_UNUSED(s) //  emit seedUrlFailed();
    }
    else if (auto s = lt::alert_cast<lt::block_uploaded_alert>(a)) {
        Q_UNUSED(s) //  emit blockUploaded();
    }


    /* block_progress_notification */
    else if (auto s = lt::alert_cast<lt::alerts_dropped_alert>(a)) {
        Q_UNUSED(s) //  emit alerts_dropped_alert();
        qWarning() << "Alert queue grew too big.";
    }
    else if (auto s = lt::alert_cast<lt::socks5_alert>(a)) {
        Q_UNUSED(s) //  emit socks5_alert();
    }

    else {
        // if we didn't handle the alert, print it to the log
        qWarning() << Q_FUNC_INFO << "didn't handle the alert (unkown/uncatched alert).";
    }
}

/******************************************************************************
 ******************************************************************************/
//static inline TorrentError toTorrentError(const lt::error_code &errc)
//{
//    TorrentError error;

//    return error;
//}

/******************************************************************************
 ******************************************************************************/
inline void WorkerThread::onTorrentAdded(const lt::torrent_handle &handle,
                                         const lt::add_torrent_params &params,
                                         const lt::error_code &error)
{
    qDebug_2 << Q_FUNC_INFO;
    if (error) {
        // Failed to add the torrent

        TorrentData d;
        d.unique_id = TorrentUtils::toUniqueId(handle.info_hash());
        d.metaInfo.error = TorrentError(TorrentError::FailedToAddError);
        d.metaInfo.error.message = QString::fromStdString(error.message());
        emit dataUpdated(d);
        return;
    }


    signalizeDataUpdated(handle, params);
}

/******************************************************************************
 ******************************************************************************/
inline void WorkerThread::onMetadataReceived(const lt::torrent_handle &handle)
{    
    qDebug_2 << Q_FUNC_INFO;
    // received torrent's metadata from magnet link at this point
    if (handle.is_valid()) {

        // set all the files priority to zero, to not download them
        if (handle.torrent_file()) {
            auto fileCount = handle.torrent_file()->num_files();
            for (auto i = 0; i < fileCount; i++) {
                auto findex = static_cast<lt::file_index_t>(i);
                handle.file_priority(findex, lt::dont_download);
            }
        }
        handle.pause();

        TorrentData d;
        d.unique_id = TorrentUtils::toUniqueId(handle.info_hash());
        d.detail = TorrentUtils::toTorrentHandleInfo(handle);
        if (handle.is_valid()) {
            std::shared_ptr<lt::torrent_info const> ti = handle.torrent_file();
            if (ti) {
                d.metaInfo.initialMetaInfo = TorrentUtils::toTorrentInitialMetaInfo(ti);
            }
        }
        emit metadataUpdated(d);
    }
}

/******************************************************************************
 ******************************************************************************/
inline void WorkerThread::onStateUpdated(const std::vector<lt::torrent_status> &status)
{
    for (auto s : status) {
        signalizeStatusUpdated(s);
    }
}

/******************************************************************************
 ******************************************************************************/
inline void WorkerThread::signalizeDataUpdated(const lt::torrent_handle &handle, const lt::add_torrent_params &params)
{
    qDebug_2 << Q_FUNC_INFO;
    if (!handle.is_valid()) {
        return;
    }
    TorrentData d;
    d.unique_id = TorrentUtils::toUniqueId(handle.info_hash());
    d.detail = TorrentUtils::toTorrentHandleInfo(handle);

    auto ti = params.ti;
    if (!ti || !ti->is_valid()) {
        // it's a magnet link perhaps, metadata has not have been received yet
        d.metaInfo.error = TorrentError(TorrentError::NoInfoYetError);
    }

    d.metaInfo = TorrentUtils::toTorrentMetaInfo(params);
    emit dataUpdated(d);
}

inline void WorkerThread::signalizeStatusUpdated(const lt::torrent_status &status)
{
    qDebug_2 << Q_FUNC_INFO;
    auto handle = status.handle;
    if (!handle.is_valid()) {
        return;
    }

    TorrentStatus s;
    s.unique_id = TorrentUtils::toUniqueId(handle.info_hash());
    s.detail = TorrentUtils::toTorrentHandleInfo(handle);

    TorrentInfo t;

    // ***************
    // Errors
    // ***************
    t.error = TorrentUtils::toTorrentError(status.errc, status.error_file);


    // ***************
    // Trackers
    // ***************
    t.lastWorkingTrackerUrl = TorrentUtils::toString(status.current_tracker);


    // ***************
    // Stats
    // ***************
    t.bytesSessionDownloaded    = static_cast<qsizetype>(status.total_download);
    t.bytesSessionUploaded      = static_cast<qsizetype>(status.total_upload);

    t.bytesSessionPayloadDownload = static_cast<qsizetype>(status.total_payload_download);
    t.bytesSessionPayloadUpload   = static_cast<qsizetype>(status.total_payload_upload);

    t.bytesFailed       = static_cast<qsizetype>(status.total_failed_bytes);
    t.bytesRedundant    = static_cast<qsizetype>(status.total_redundant_bytes);

    t.downloadedPieces  = TorrentUtils::toBitArray(status.pieces);
    t.verifiedPieces    = TorrentUtils::toBitArray(status.verified_pieces);

    t.bytesReceived     = static_cast<qsizetype>(status.total_done);
    t.bytesTotal        = static_cast<qsizetype>(status.total);

    t.bytesWantedReceived   = static_cast<qsizetype>(status.total_wanted_done);
    t.bytesWantedTotal      = static_cast<qsizetype>(status.total_wanted);

    t.bytesAllSessionsPayloadDownload   = static_cast<qsizetype>(status.all_time_upload);
    t.bytesAllSessionsPayloadUpload     = static_cast<qsizetype>(status.all_time_download);

    t.addedTime             = TorrentUtils::toDateTime(status.added_time);
    t.completedTime         = TorrentUtils::toDateTime(status.completed_time);
    t.lastSeenCompletedTime = TorrentUtils::toDateTime(status.last_seen_complete);

    t.percent = qreal(100.0f * status.progress);

    t.downloadSpeed = qreal(status.download_rate);
    t.uploadSpeed   = qreal(status.upload_rate);

    t.download_payload_rate = status.download_payload_rate;
    t.upload_payload_rate = status.upload_payload_rate;

    t.connectedSeedsCount = status.num_seeds;
    t.connectedPeersCount = status.num_peers;

    t.completePeersCount   = status.num_complete;
    t.incompletePeersCount = status.num_incomplete;

    t.seedsCount = status.list_seeds;
    t.peersCount = status.list_peers;

    t.candidatePeersCount = status.connect_candidates;

    t.downloadedPiecesCount = status.num_pieces;

    t.distributedFullCopiesCount    = status.distributed_full_copies;
    t.distributedFraction           = status.distributed_fraction;
    t.distributedCopiesFraction     = qreal(status.distributed_copies);

    t.blockSizeInByte = static_cast<qsizetype>(status.block_size);

    t.peersUnchokedCount     = status.num_uploads;
    t.peersConnectionCount   = status.num_connections;

    t.uploadSlotsLimit       = status.uploads_limit;
    t.connectionsNumberLimit = status.connections_limit;

    t.upBandwidthQuotaQueue    = status.up_bandwidth_queue;
    t.downBandwidthQuotaQueue  = status.down_bandwidth_queue;

    t.seedRank = status.seed_rank;

    t.state = TorrentUtils::toState(status.state);

    //  t.need_save_resume = state.need_save_resume;

    t.isSeeding                 = status.is_seeding;
    t.isFinished                = status.is_finished;
    t.hasMetadata               = status.has_metadata;
    t.hasIncoming               = status.has_incoming;
    t.isMovingStorage           = status.moving_storage;

    t.isAnnouncingToTrackers    = status.announcing_to_trackers;
    t.isAnnouncingToLSD         = status.announcing_to_lsd;
    t.isAnnouncingToDHT         = status.announcing_to_dht;

    t.infohash = TorrentUtils::toString(status.info_hashes.get_best());

    // t.lastTimeUpload            = toDateTime2(status.last_upload);
    // t.lastTimeDownload          = toDateTime2(status.last_download);

    // Cumulative counters
    t.activeTimeDuration        = status.active_duration.count();
    t.finishedTimeDuration      = status.finished_duration.count();
    t.seedingTimeDuration       = status.seeding_duration.count();

    // t.flags = status.flags(); // see torrent flags

    s.info = t;
    emit statusUpdated(s);
}

/******************************************************************************
 ******************************************************************************/
void WorkerThread::log(lt::alert *s)
{
    qDebug_2 << "[alert]" << QString::fromStdString(s->message());
}
