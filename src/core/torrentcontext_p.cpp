/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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

#include "torrentcontext_p.h"

#include "downloadtorrentitem_p.h"

#include <Globals>
#include <Core/DownloadTorrentItem>
#include <Core/ResourceItem>
#include <Core/Settings>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QtMath>

#include <string>    // std::string
#include <algorithm> // std::min, std::max
#include <chrono>

#include "libtorrent/add_torrent_params.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/announce_entry.hpp"
#include "libtorrent/bdecode.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/config.hpp"
#include "libtorrent/entry.hpp"
#include "libtorrent/hex.hpp"               // to_hex, from_hex
#include "libtorrent/identify_client.hpp"
#include "libtorrent/ip_filter.hpp"
#include "libtorrent/magnet_uri.hpp"
#include "libtorrent/operations.hpp"
#include "libtorrent/peer_info.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/settings_pack.hpp"
#include "libtorrent/string_view.hpp"
#include "libtorrent/time.hpp"
#include "libtorrent/torrent_info.hpp"

#ifndef TORRENT_DISABLE_EXTENSIONS
#   include <libtorrent/extensions/smart_ban.hpp>
#   include <libtorrent/extensions/ut_metadata.hpp>
#   include <libtorrent/extensions/ut_pex.hpp>
#endif


static const lt::status_flags_t s_torrent_status_flags =
        lt::torrent_handle::query_distributed_copies
        | lt::torrent_handle::query_accurate_download_counters
        | lt::torrent_handle::query_last_seen_complete
        | lt::torrent_handle::query_pieces
        | lt::torrent_handle::query_verified_pieces
        ;

static inline EndPoint toEndPoint(const lt::tcp::endpoint &endpoint);
static inline lt::tcp::endpoint fromEndPoint(const EndPoint &endpoint);

static inline TorrentTrackerInfo::Source toTrackerSource(const lt::announce_entry::tracker_source &s);

static inline TorrentTrackerInfo toTorrentTrackerInfo(const lt::announce_entry &tracker);
static inline lt::announce_entry fromTorrentTrackerInfo(const TorrentTrackerInfo &tracker);

static inline TorrentFileInfo::Priority toPriority(const lt::download_priority_t &priority);
static inline lt::download_priority_t fromPriority(const TorrentFileInfo::Priority &p);


TorrentContextPrivate::TorrentContextPrivate(TorrentContext *qq)
    : QObject(qq)
    , q(qq)
    , workerThread(Q_NULLPTR)
    , settings(Q_NULLPTR)
{
    qRegisterMetaType<TorrentData>();
    qRegisterMetaType<TorrentStatus>();

    workerThread = new WorkerThread(this);

    connect(workerThread, &WorkerThread::dataUpdated, this, &TorrentContextPrivate::onDataUpdated);
    connect(workerThread, &WorkerThread::statusUpdated, this, &TorrentContextPrivate::onStatusUpdated);

    connect(workerThread, &WorkerThread::stopped, this, &TorrentContextPrivate::onStopped);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    workerThread->setEnabled(false);
    workerThread->start();
}

TorrentContextPrivate::~TorrentContextPrivate()
{
    workerThread->stop();
    if (!workerThread->wait(3000)) {
        qDebug() << Q_FUNC_INFO << "Terminating...";
        workerThread->terminate();
        workerThread->wait();
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::onSettingsChanged()
{
    if (!settings) {
        return;
    }
    lt::settings_pack pack = lt::default_settings(); /*= fromSettings(settings)*/

    const QMap<QString, QVariant> map = settings->torrentSettings();
    foreach (QString key, map.keys()) {
        QVariant value = map.value(key);

        int name = lt::setting_by_name(key.toUtf8().data());

        int type = name & lt::settings_pack::type_mask;

        switch (type) {
        case lt::settings_pack::string_type_base:
        {
            pack.set_str(name, value.toString().toStdString());
            break;
        }
        case lt::settings_pack::int_type_base:
        {
            pack.set_int(name, value.toInt());
            break;
        }
        case lt::settings_pack::bool_type_base:
        {
            pack.set_bool(name, value.toBool());
            break;
        }
        default:
            break;
        }
    }

    workerThread->setSettings(pack);

    bool enabled = settings->isTorrentEnabled();
    workerThread->setEnabled(enabled);
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Return every setting and its default value.
 */
QList<TorrentSettingItem> TorrentContextPrivate::allSettingsKeysAndValues() const
{
    return _toPreset( lt::default_settings() );
}

QList<TorrentSettingItem> TorrentContextPrivate::presetDefault() const
{
    return _toPreset( lt::default_settings() );
}

QList<TorrentSettingItem> TorrentContextPrivate::presetMinCache() const
{
    return _toPreset( lt::min_memory_usage() );
}

QList<TorrentSettingItem> TorrentContextPrivate::presetHighPerf() const
{
    return _toPreset( lt::high_performance_seed() );
}

/******************************************************************************
 ******************************************************************************/
QList<TorrentSettingItem> TorrentContextPrivate::_toPreset(const lt::settings_pack all) const
{
    QList<TorrentSettingItem> ret;

    struct SettingClass {
        int begin;
        int end;
        QVariant (*pf)(const lt::settings_pack&, const int);
    };

    QList<SettingClass> cs = {
        { lt::settings_pack::string_type_base,
          lt::settings_pack::max_string_setting_internal,
          &TorrentContextPrivate::_get_str
        },
        { lt::settings_pack::int_type_base,
          lt::settings_pack::max_int_setting_internal,
          &TorrentContextPrivate::_get_int
        },
        { lt::settings_pack::bool_type_base,
          lt::settings_pack::max_bool_setting_internal,
          &TorrentContextPrivate::_get_bool
        }
    };

    foreach (SettingClass c, cs) {
        for (int index = c.begin; index < c.end; ++index) {

            // Remove non-modifiable settings and settings managed somewhere else.
            switch (index) {
            case lt::settings_pack::user_agent:
            case lt::settings_pack::alert_mask:
                continue;
            default:
                break;
            }

            const char* name = lt::name_for_setting(index);
            if (name == nullptr) {
                continue;
            }
            const QString key = QString::fromUtf8(name);
            if (!key.isEmpty()) {
                QVariant value = c.pf(all, index);
                TorrentSettingItem item;
                item.displayKey = QString("torrent.global.%0").arg(key);
                item.key = key;
                item.value = value;
                item.defaultValue = value;
                ret.append(item);
            }
        }
    }
    return ret;
}

/******************************************************************************
 ******************************************************************************/
QVariant TorrentContextPrivate::_get_str(const lt::settings_pack &pack, const int index)
{
    return QString::fromStdString(pack.get_str(index));
}

QVariant TorrentContextPrivate::_get_int(const lt::settings_pack &pack, const int index)
{
    return pack.get_int(index);
}

QVariant TorrentContextPrivate::_get_bool(const lt::settings_pack &pack, const int index)
{
    return pack.get_bool(index);
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::onStopped()
{  
    qDebug() << Q_FUNC_INFO; // Just to confirm it's stopped:)
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::onDataUpdated(TorrentData data) // onTorrentAdded
{
    qDebug() << Q_FUNC_INFO;
    DownloadTorrentItem *item = find(data.unique_id);
    if (item) {
        item->setMetaInfo(data.metaInfo);
        item->setDetail(data.detail);

        item->data()->m_fileModel->refreshMetaData(data.metaInfo.files);

        item->data()->m_fileModel->refreshData(data.detail.files);
        item->data()->m_peerModel->refreshData(data.detail.peers);
        item->data()->m_trackerModel->refreshData(data.metaInfo.trackers);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::onStatusUpdated(TorrentStatus status)
{
    qDebug() << Q_FUNC_INFO;
    DownloadTorrentItem *item = find(status.unique_id);
    if (item) {
        item->setInfo(status.info);
        item->setDetail(status.detail);

        item->data()->m_fileModel->refreshData(status.detail.files);
        item->data()->m_peerModel->refreshData(status.detail.peers);
        item->data()->m_trackerModel->refreshData(status.detail.trackers);
    }
}


/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::addFromInfoHash(const QString &infohash)
{
    lt::add_torrent_params p;
}

void TorrentContextPrivate::addMagnet(const QString &magnetURI)
{
    qDebug() << Q_FUNC_INFO << magnetURI;

    lt::string_view uri = magnetURI.toStdString().data();

    lt::error_code ec;
    lt::add_torrent_params p = lt::parse_magnet_uri(uri.to_string(), ec);

    if (ec) {
        std::printf("invalid magnet link \"%s\": %s\n"
                    , uri.to_string().c_str(), ec.message().c_str());
        return;
    }

    std::vector<char> resume_data;
    std::printf("adding magnet: %s\n", uri.to_string().c_str());
}

// return false on failure
bool TorrentContextPrivate::addTorrentUrl(DownloadTorrentItem *item)
{
    const QString torrentFileName = item->resource()->url();
    const QString outputPath = item->localFilePath();
    QDir().mkpath(outputPath);

    std::string torrent = torrentFileName.toStdString();
    std::string savepath = outputPath.toStdString();
    lt::error_code ec;
    auto ti = std::make_shared<lt::torrent_info>(torrent, ec);
    if (ec) {
        std::printf("failed to load torrent \"%s\": %s\n"
                    , torrent.c_str(), ec.message().c_str());
        return false;
    }

    lt::add_torrent_params p;
    p.ti = ti;
    p.flags &= ~lt::torrent_flags::duplicate_is_error; // do not raise exception if duplicate
    p.save_path = savepath;

    // Blocking insertion
    lt::error_code ec2;
    lt::torrent_handle h = workerThread->addTorrent(std::move(p), ec2);
    if (ec2) {
        std::printf("failed to add torrent \"%s\": %s\n" , torrent.c_str(), ec2.message().c_str());
        return false;
    }
    if (!h.is_valid()) {
        return false;
    }
    h.pause();
    const UniqueId uuid = h.info_hash();
    hashMap.insert(uuid, item);
    return true;
}


/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::removeTorrent(DownloadTorrentItem *item)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        workerThread->removeTorrent(h); // needs calling lt::session
        const UniqueId uuid = h.info_hash();
        hashMap.remove(uuid);
    }
}

/******************************************************************************
 ******************************************************************************/
bool TorrentContextPrivate::hasTorrent(DownloadTorrentItem *item)
{
    lt::torrent_handle h = find(item);
    return h.is_valid();
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::resumeTorrent(DownloadTorrentItem *item)
{
    qDebug() << Q_FUNC_INFO;
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.resume();
    }
}

void TorrentContextPrivate::pauseTorrent(DownloadTorrentItem *item)
{
    qDebug() << Q_FUNC_INFO;
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.pause();
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::moveQueueUp(DownloadTorrentItem *item)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.queue_position_up();
    }
}

void TorrentContextPrivate::moveQueueDown(DownloadTorrentItem *item)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.queue_position_down();
    }
}

void TorrentContextPrivate::moveQueueTop(DownloadTorrentItem *item)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.queue_position_top();
    }
}

void TorrentContextPrivate::moveQueueBottom(DownloadTorrentItem *item)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.queue_position_bottom();
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::changeFilePriority(DownloadTorrentItem *item,
                                               int index, TorrentFileInfo::Priority p)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        lt::file_index_t findex = static_cast<lt::file_index_t>(index);
        lt::download_priority_t priority = fromPriority(p);
        h.file_priority(findex, priority);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::addSeed(DownloadTorrentItem *item, const TorrentWebSeedMetaInfo &seed)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        if (seed.type == TorrentWebSeedMetaInfo::Type::UrlSeed) {
            h.add_url_seed(seed.url.toStdString());
        } else {
            h.add_http_seed(seed.url.toStdString());
        }
    }
}

void TorrentContextPrivate::removeSeed(DownloadTorrentItem *item, const TorrentWebSeedMetaInfo &seed)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        if (seed.type == TorrentWebSeedMetaInfo::Type::UrlSeed) {
            h.remove_url_seed(seed.url.toStdString());
        } else {
            h.remove_http_seed(seed.url.toStdString());
        }
    }
}

void TorrentContextPrivate::removeAllSeeds(DownloadTorrentItem *item)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        std::set<std::string>::iterator i, end;
        std::set<std::string> seeds;

        seeds = h.url_seeds();
        for (i = seeds.begin(), end = (seeds.end()); i != end; ++i) {
            h.remove_url_seed(*i);
        }

        seeds = h.http_seeds();
        for (i = seeds.begin(), end = (seeds.end()); i != end; ++i) {
            h.remove_http_seed(*i);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::addPeer(DownloadTorrentItem *item, const TorrentPeerMetaInfo &peer)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        lt::tcp::endpoint endpoint = fromEndPoint(peer.endpoint);
        h.connect_peer(endpoint);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::addTracker(DownloadTorrentItem *item,
                                       const TorrentTrackerInfo &tracker)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        lt::announce_entry entry = fromTorrentTrackerInfo(tracker);
        h.add_tracker(entry);
    }
}

void TorrentContextPrivate::removeTracker(DownloadTorrentItem *item,
                                          const TorrentTrackerInfo &tracker)
{
    Q_UNUSED(item)
    Q_UNUSED(tracker)
    /// \todo
}


/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::forceRecheck(DownloadTorrentItem *item)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.force_recheck();
    }
}

void TorrentContextPrivate::forceReannounce(DownloadTorrentItem *item)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.force_reannounce();
    }
}

void TorrentContextPrivate::forceDHTReannounce(DownloadTorrentItem *item)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.force_dht_announce();
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::setSSLCertificatePath(DownloadTorrentItem *item, const QString &path)
{
    Q_UNUSED(item)
    Q_UNUSED(path)
    // todo
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::scrapeTracker(DownloadTorrentItem *item, int index)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.scrape_tracker(index);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::setUploadBandwidth(DownloadTorrentItem *item, int limit)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.set_upload_limit(limit);
    }
}

void TorrentContextPrivate::setDownloadBandwidth(DownloadTorrentItem *item, int limit)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.set_download_limit(limit);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::setMaxUploads(DownloadTorrentItem *item, int limit)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.set_max_uploads(limit);
    }
}

void TorrentContextPrivate::setMaxConnections(DownloadTorrentItem *item, int limit)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        h.set_max_connections(limit);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::renameFile(DownloadTorrentItem *item, int index, const QString &newName)
{
    lt::torrent_handle h = find(item);
    if (h.is_valid()) {
        lt::file_index_t findex = static_cast<lt::file_index_t>(index);
        h.rename_file(findex, newName.toStdString());
    }
}

/******************************************************************************
 ******************************************************************************/
inline DownloadTorrentItem* TorrentContextPrivate::find(const UniqueId &uuid)
{
    return hashMap.value(uuid, nullptr);
}

inline lt::torrent_handle TorrentContextPrivate::find(DownloadTorrentItem *item)
{
    const UniqueId info_hash = hashMap.key(item, UniqueId());
    return workerThread->findTorrent(info_hash);
}

/******************************************************************************
 ******************************************************************************/
WorkerThread::WorkerThread(QObject *parent) : QThread(parent)
{
    m_session_ptr = new lt::session();
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
        pack.set_str(lt::settings_pack::user_agent, userAgent());
        pack.set_int(lt::settings_pack::alert_mask, lt::alert::all_categories);

        m_session_ptr->apply_settings(pack);
    }
}

/******************************************************************************
 ******************************************************************************/
lt::torrent_handle WorkerThread::addTorrent(lt::add_torrent_params const& params,
                                            lt::error_code& ec)
{
    Q_ASSERT(m_session_ptr);
    if (m_session_ptr && m_session_ptr->is_valid()) {
        return m_session_ptr->add_torrent(params, ec);
    }
    return lt::torrent_handle();
}

/******************************************************************************
 ******************************************************************************/
void WorkerThread::removeTorrent(const lt::torrent_handle& h, lt::remove_flags_t options)
{
    Q_ASSERT(m_session_ptr);
    if (m_session_ptr && m_session_ptr->is_valid()) {
        m_session_ptr->remove_torrent(h, options);

        /// \todo also delete the files ?
    }
}

/******************************************************************************
 ******************************************************************************/
lt::torrent_handle WorkerThread::findTorrent(const UniqueId &uuid) const
{
    Q_ASSERT(m_session_ptr);
    if (m_session_ptr && m_session_ptr->is_valid()) {
        return m_session_ptr->find_torrent(uuid);
    }
    return lt::torrent_handle();
}


/******************************************************************************
 ******************************************************************************/
void WorkerThread::run()
{
    qDebug() << Q_FUNC_INFO;

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

#ifndef TORRENT_DISABLE_DHT
    if (settings.has_val(lt::settings_pack::dht_upload_rate_limit)) {
        params.dht_settings.upload_rate_limit
                = params.settings.get_int(lt::settings_pack::dht_upload_rate_limit);
    }
    session.set_dht_settings(std::move(params.dht_settings));

    TORRENT_ASSERT(params.dht_storage_constructor);
    session.set_dht_storage(std::move(params.dht_storage_constructor));
#endif

    lt::ip_filter loaded_ip_filter;
    session.set_ip_filter(loaded_ip_filter);

    session.pause();

    // main loop
    while (!shouldQuit) {
        session.post_torrent_updates(s_torrent_status_flags);
        session.post_session_stats();
        session.post_dht_stats();

        QThread::msleep(500);

        std::vector<lt::alert*> alerts;
        session.pop_alerts(&alerts);
        for (auto a : alerts) {
            signalizeAlert(a);
        }
    } // end of main loop

    qDebug() << Q_FUNC_INFO << "Closing session... ";

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
    if (lt::torrent_removed_alert* s = lt::alert_cast<lt::torrent_removed_alert>(a)) {
        //QString hash = toString(s->info_hash);
        //  emit torrentRemoved(hash);
        log(s);
    }
    else if (lt::state_changed_alert* s = lt::alert_cast<lt::state_changed_alert>(a)) {
        //        lt::torrent_status::state_t oldStatus = s->prev_state;
        //        lt::torrent_status::state_t newStatus = s->state;
        //  emit torrentStateChanged(oldState, newState); // rem quel torrent ?
        log(s);
    }
    else if (lt::hash_failed_alert* s = lt::alert_cast<lt::hash_failed_alert>(a)) {
        //        int piece_index = (int)s->piece_index;
        //  emit pieceHashCheckFailed(piece_index);
        log(s);
    }
    else if (lt::torrent_finished_alert* s = lt::alert_cast<lt::torrent_finished_alert>(a)) {
        //        emit torrentFinished();
        log(s);
    }
    else if (lt::torrent_paused_alert* s = lt::alert_cast<lt::torrent_paused_alert>(a)) {
        //        emit torrentPaused();
        log(s);
    }
    else if (lt::torrent_resumed_alert* s = lt::alert_cast<lt::torrent_resumed_alert>(a)) {
        //        emit torrentResumed();
        log(s);
    }
    else if (lt::torrent_checked_alert* s = lt::alert_cast<lt::torrent_checked_alert>(a)) {
        //        emit torrentChecked();
        log(s);
    }
    else if (lt::fastresume_rejected_alert* s = lt::alert_cast<lt::fastresume_rejected_alert>(a)) {
        //        emit torrentFastResumeFailed();
        log(s);
    }
    else if (lt::trackerid_alert* s = lt::alert_cast<lt::trackerid_alert>(a)) {
        //        emit trackeridReceived();
        log(s);
    }
    else if (lt::torrent_error_alert* s = lt::alert_cast<lt::torrent_error_alert>(a)) {
        //        emit torrentError();
        log(s);
    }
    else if (lt::torrent_need_cert_alert* s = lt::alert_cast<lt::torrent_need_cert_alert>(a)) {
        //        emit torrentSSLError();
        log(s);
    }
    else if (lt::add_torrent_alert* s = lt::alert_cast<lt::add_torrent_alert>(a)) {
        signalizeDataUpdated(s->handle, s->params, s->error);
    }
    else if (lt::state_update_alert* s = lt::alert_cast<lt::state_update_alert>(a)) {
        signalizeStatusUpdated(s->status);
    }



    // magnet extension
    else if (lt::metadata_failed_alert* s = lt::alert_cast<lt::metadata_failed_alert>(a)) {
        //        emit metadataFailed();
        log(s);
    }
    else if (lt::metadata_received_alert* s = lt::alert_cast<lt::metadata_received_alert>(a)) {
        Q_UNUSED(s)
        //        torrent_handle h = s->handle;
        //                h.save_resume_data(torrent_handle::save_info_dict);
        //                ++num_outstanding_resume_data;
        //        emit metadataReceived();
    }

    // IP
    else if (lt::external_ip_alert* s = lt::alert_cast<lt::external_ip_alert>(a)) {
        Q_UNUSED(s) //  emit externalIpAddressReceived();
    }
    else if (lt::peer_blocked_alert* s = lt::alert_cast<lt::peer_blocked_alert>(a)) {
        Q_UNUSED(s) //  emit peerIpBlocked();
    }
    // dht
    else if (lt::dht_announce_alert* s = lt::alert_cast<lt::dht_announce_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHT();
    }
    else if (lt::dht_get_peers_alert* s = lt::alert_cast<lt::dht_get_peers_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHTReceviedd();
    }
    else if (lt::dht_bootstrap_alert* s = lt::alert_cast<lt::dht_bootstrap_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHTbootstrapDone();
    }
    else if (lt::dht_error_alert* s = lt::alert_cast<lt::dht_error_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHTError();
    }
    else if (lt::dht_immutable_item_alert* s = lt::alert_cast<lt::dht_immutable_item_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHTImmmutablee();
    }
    else if (lt::dht_mutable_item_alert* s = lt::alert_cast<lt::dht_mutable_item_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHTmutablee();
    }
    else if (lt::dht_put_alert* s = lt::alert_cast<lt::dht_put_alert>(a)) {
        Q_UNUSED(s) //  emit peerDHTput();
    }
    else if (lt::i2p_alert* s = lt::alert_cast<lt::i2p_alert>(a)) {
        Q_UNUSED(s) //  emit i2pError();
    }
    else if (lt::dht_outgoing_get_peers_alert* s = lt::alert_cast<lt::dht_outgoing_get_peers_alert>(a)) {
        Q_UNUSED(s) //  emit dht_get_peers_Sent();
    }



    // Firewall error ?
    else if (lt::udp_error_alert* s = lt::alert_cast<lt::udp_error_alert>(a)) {
        Q_UNUSED(s) //  emit protocolError();
    }
    else if (lt::listen_succeeded_alert* s = lt::alert_cast<lt::listen_succeeded_alert>(a)) {
        Q_UNUSED(s) //  emit hostPortOpened();
    }
    else if (lt::listen_failed_alert* s = lt::alert_cast<lt::listen_failed_alert>(a)) {
        Q_UNUSED(s) //  emit hostPortOpenFailed();
    }
    else if (lt::portmap_alert* s = lt::alert_cast<lt::portmap_alert>(a)) {
        Q_UNUSED(s) //  emit portOpened();
    }
    else if (lt::portmap_error_alert* s = lt::alert_cast<lt::portmap_error_alert>(a)) {
        Q_UNUSED(s) //  emit portOpenFailed();
    }
    else if (lt::portmap_log_alert* s = lt::alert_cast<lt::portmap_log_alert>(a)) {
        Q_UNUSED(s) //  emit debugPort();
    }
    else if (lt::log_alert* s = lt::alert_cast<lt::log_alert>(a)) {
        Q_UNUSED(s) //  emit debugLog();
    }
    else if (lt::torrent_log_alert* s = lt::alert_cast<lt::torrent_log_alert>(a)) {
        Q_UNUSED(s) //  emit debugTorrentLog();
    }
    else if (lt::peer_log_alert* s = lt::alert_cast<lt::peer_log_alert>(a)) {
        Q_UNUSED(s) //  emit debugPeerLog();
    }
    else if (lt::lsd_error_alert* s = lt::alert_cast<lt::lsd_error_alert>(a)) {
        Q_UNUSED(s) //  emit debugLSDPeerLog();
    }
    else if (lt::dht_log_alert* s = lt::alert_cast<lt::dht_log_alert>(a)) {
        Q_UNUSED(s) //  emit debugDHTLog();
    }
    else if (lt::dht_pkt_alert* s = lt::alert_cast<lt::dht_pkt_alert>(a)) {
        Q_UNUSED(s) //  emit dht_pkt_alert();
    }
    else if (lt::dht_get_peers_reply_alert* s = lt::alert_cast<lt::dht_get_peers_reply_alert>(a)) {
        Q_UNUSED(s) //  emit dht_get_peers_reply_alert();
    }
    else if (lt::dht_direct_response_alert* s = lt::alert_cast<lt::dht_direct_response_alert>(a)) {
        Q_UNUSED(s) //  emit dht_direct_response_alert();
    }
    else if (lt::picker_log_alert* s = lt::alert_cast<lt::picker_log_alert>(a)) {
        Q_UNUSED(s) //  emit debugPickerLog();
    }
    else if (lt::session_error_alert* s = lt::alert_cast<lt::session_error_alert>(a)) {
        Q_UNUSED(s) //  emit session_error_alert();
    }
    else if (lt::dht_live_nodes_alert* s = lt::alert_cast<lt::dht_live_nodes_alert>(a)) {
        Q_UNUSED(s) //  emit dht_live_nodes_alert();
    }
    else if (lt::dht_sample_infohashes_alert* s = lt::alert_cast<lt::dht_sample_infohashes_alert>(a)) {
        Q_UNUSED(s) //  emit dht_sample_infohashes_alert();
    }


    /* stats_notification */
    else if (lt::stats_alert* s = lt::alert_cast<lt::stats_alert>(a)) {
        Q_UNUSED(s) //  emit statsUpdated();
    }
    else if (lt::session_stats_alert* s = lt::alert_cast<lt::session_stats_alert>(a)) {
        //        const lt::span<std::int64_t const> counters = s->counters() ;

        // TODO

        Q_UNUSED(s) //  emit sessionStatsUpdated();
    }
    else if (lt::dht_stats_alert* s = lt::alert_cast<lt::dht_stats_alert>(a)) {
        Q_UNUSED(s) //  emit dhtStatsUpdated();
    }
    else if (lt::session_stats_header_alert* s = lt::alert_cast<lt::session_stats_header_alert>(a)) {
        Q_UNUSED(s) //  emit session_stats_header_Updated();
    }


    /* storage_notification */
    //    else if (lt::read_piece_alert* s = lt::alert_cast<lt::read_piece_alert>(a)) {
    //                 emit fileCompleted(index);
    //        }

    else if (lt::file_renamed_alert* s = lt::alert_cast<lt::file_renamed_alert>(a)) {
        //        int index = (int)s->index;
        //        QString newName = QString::fromUtf8(s->new_name());
        Q_UNUSED(s) //  emit fileRenamed(index, newName);
    }
    else if (lt::file_rename_failed_alert* s = lt::alert_cast<lt::file_rename_failed_alert>(a)) {
        //        int index = (int)s->index;
        //        QString errorMessage = QString::fromStdString(s->error.message());
        Q_UNUSED(s) //  emit fileRenameFailed(index, errorMessage);
    }
    else if (lt::storage_moved_alert* s = lt::alert_cast<lt::storage_moved_alert>(a)) {
        Q_UNUSED(s) //  emit storageMoved();
    }
    else if (lt::storage_moved_failed_alert* s = lt::alert_cast<lt::storage_moved_failed_alert>(a)) {
        Q_UNUSED(s) //  emit storageMoveFailed();
    }
    else if (lt::torrent_deleted_alert* s = lt::alert_cast<lt::torrent_deleted_alert>(a)) {
        Q_UNUSED(s) //  emit torrentDeleted();
    }
    else if (lt::torrent_delete_failed_alert* s = lt::alert_cast<lt::torrent_delete_failed_alert>(a)) {
        Q_UNUSED(s) //  emit torrentDeleteFailed();
    }
    else if (lt::save_resume_data_alert* s = lt::alert_cast<lt::save_resume_data_alert>(a)) {
        Q_UNUSED(s) //  emit resumeDataSaved();

    } else if (lt::save_resume_data_failed_alert* s = lt::alert_cast<lt::save_resume_data_failed_alert>(a)) {
        Q_UNUSED(s) //  emit resumeDataSaveFailed();
    }
    else if (lt::file_error_alert* s = lt::alert_cast<lt::file_error_alert>(a)) {
        Q_UNUSED(s) //  emit fileReadOrWriteError();
    }
    else if (lt::cache_flushed_alert* s = lt::alert_cast<lt::cache_flushed_alert>(a)) {
        Q_UNUSED(s) //  emit cache_flushed();
    }




    /* file_progress_notification */
    else if (lt::file_completed_alert* s = lt::alert_cast<lt::file_completed_alert>(a)) {
        //        int index = (int)s->index;
        Q_UNUSED(s) //  emit fileCompleted(index);
    }

    /* performance_warning */
    else if (lt::performance_alert* s = lt::alert_cast<lt::performance_alert>(a)) {
        //        QString message = QString::fromStdString(s->message());
        Q_UNUSED(s) //  emit performanceWarning(message);
    }

    /* tracker_notification & error_notification */
    else if (lt::tracker_error_alert* s = lt::alert_cast<lt::tracker_error_alert>(a)) {
        //        int times_in_row = s->times_in_row;
        QString errorCode = QString::fromStdString(s->error.message());
        Q_UNUSED(s) //  emit trackerConnectionFailed(times_in_row, errorCode);
    }
    else if (lt::tracker_warning_alert* s = lt::alert_cast<lt::tracker_warning_alert>(a)) {
        //        QString message = QString::fromUtf8(s->warning_message());
        Q_UNUSED(s) //  emit trackerMessageReceived(message);
    }
    else if (lt::scrape_reply_alert* s = lt::alert_cast<lt::scrape_reply_alert>(a)) {
        //        int incomplete = s->incomplete;
        //        int complete = s->complete;
        Q_UNUSED(s) //  emit trackerScrapeSucceeded(incomplete, complete);
    }
    else if (lt::scrape_failed_alert* s = lt::alert_cast<lt::scrape_failed_alert>(a)) {
        //        QString errorCode = QString::fromStdString(s->error.message());
        //        QString errorMessage = QString::fromUtf8(s->error_message());
        Q_UNUSED(s) //  emit trackerScrapeFailed(errorCode, errorMessage);
    }
    else if (lt::tracker_reply_alert* s = lt::alert_cast<lt::tracker_reply_alert>(a)) {
        //        int peersCount = s->num_peers;
        Q_UNUSED(s) //  emit trackerInfo(peersCount);
    }
    else if (lt::dht_reply_alert* s = lt::alert_cast<lt::dht_reply_alert>(a)) {
        //        int peersCount = s->num_peers;
        Q_UNUSED(s) //  emit trackerDHTInfo(peersCount);
    }
    else if (lt::tracker_announce_alert* s = lt::alert_cast<lt::tracker_announce_alert>(a)) {
        //        int eventcode = s->event;
        Q_UNUSED(s) //  emit trackerEventSent(eventcode);
    }

    /* peer_notification */
    else if (lt::peer_ban_alert* s = lt::alert_cast<lt::peer_ban_alert>(a)) {
        Q_UNUSED(s) //  emit peerBanned();
    }
    else if (lt::peer_unsnubbed_alert* s = lt::alert_cast<lt::peer_unsnubbed_alert>(a)) {
        Q_UNUSED(s) //  emit peerUnsnubbed();
    }
    else if (lt::peer_snubbed_alert* s = lt::alert_cast<lt::peer_snubbed_alert>(a)) {
        Q_UNUSED(s) //  emit peerSnubbed();
    }
    else if (lt::peer_error_alert* s = lt::alert_cast<lt::peer_error_alert>(a)) {
        //        QString operation = QString::fromUtf8(lt::operation_name(s->op));
        //        QString errorCode = QString::fromStdString(s->error.message());
        Q_UNUSED(s) //  emit peerAboutToDisconnected(operation, errorCode);
    }
    else if (lt::peer_connect_alert* s = lt::alert_cast<lt::peer_connect_alert>(a)) {
        Q_UNUSED(s)
        // don't log every peer we try to connect to
        return;
        //  int socket_type = s->socket_type;
        //  emit peerConnected(socket_type);
    }
    else if (lt::peer_disconnected_alert* s = lt::alert_cast<lt::peer_disconnected_alert>(a)) {
        //        int socket_type = s->socket_type;
        //        operation_t const op;
        //		error_code const error;
        //		close_reason_t const reason;
        Q_UNUSED(s) //  emit peerDisconnected(/*socket_type*/);
    }
    else if (lt::invalid_request_alert* s = lt::alert_cast<lt::invalid_request_alert>(a)) {
        //        peer_request const request;
        //		bool const we_have;
        //		bool const peer_interested;
        //		bool const withheld;
        Q_UNUSED(s) //  emit peerInvalidPieceReceived();
    }
    else if (lt::piece_finished_alert* s = lt::alert_cast<lt::piece_finished_alert>(a)) {
        //        lt::torrent_handle handle = s->handle;
        //        lt::piece_index_t piece_index = s->piece_index;
        Q_UNUSED(s) //  emit pieceFinished();
    }
    else if (lt::lsd_peer_alert* s = lt::alert_cast<lt::lsd_peer_alert>(a)) {
        Q_UNUSED(s) //  emit lsd_peer();
    }
    else if (lt::incoming_connection_alert* s = lt::alert_cast<lt::incoming_connection_alert>(a)) {
        Q_UNUSED(s) //  emit peerIncomingConnectionAccepted();
    }
    else if (lt::incoming_request_alert* s = lt::alert_cast<lt::incoming_request_alert>(a)) {
        Q_UNUSED(s) //  emit peerIncomingrequestAccepted();
    }


    /* block_progress_notification */
    else if (lt::request_dropped_alert* s = lt::alert_cast<lt::request_dropped_alert>(a)) {
        // lt::torrent_handle handle = s->handle;
        // lt::piece_index_t piece_index = s->piece_index;
        Q_UNUSED(s) //  emit peerPieceRejected();
    }
    else if (lt::block_timeout_alert* s = lt::alert_cast<lt::block_timeout_alert>(a)) {
        Q_UNUSED(s) //  emit peerPieceTimeOut();
    }
    else if (lt::block_finished_alert* s = lt::alert_cast<lt::block_finished_alert>(a)) {
        Q_UNUSED(s) //  emit peerPieceFinished();
    }
    else if (lt::block_downloading_alert* s = lt::alert_cast<lt::block_downloading_alert>(a)) {
        Q_UNUSED(s) //  emit peerPieceDownloading();
    }
    else if (lt::unwanted_block_alert* s = lt::alert_cast<lt::unwanted_block_alert>(a)) {
        Q_UNUSED(s) //  emit peerPieceUnwanted();
    }

    else if (lt::url_seed_alert* s = lt::alert_cast<lt::url_seed_alert>(a)) {
        Q_UNUSED(s) //  emit seedUrlFailed();
    }
    else if (lt::block_uploaded_alert* s = lt::alert_cast<lt::block_uploaded_alert>(a)) {
        Q_UNUSED(s) //  emit blockUploaded();
    }


    /* block_progress_notification */
    else if (lt::alerts_dropped_alert* s = lt::alert_cast<lt::alerts_dropped_alert>(a)) {
        Q_UNUSED(s) //  emit alerts_dropped_alert();
    }
    else if (lt::socks5_alert* s = lt::alert_cast<lt::socks5_alert>(a)) {
        Q_UNUSED(s) //  emit socks5_alert();
    }

    else {
        // if we didn't handle the alert, print it to the log
        QString message = QString::fromStdString(s->message());
        qDebug() << Q_FUNC_INFO << "didn't handle the alert: " << message;
    }
}

/******************************************************************************
 ******************************************************************************/
inline void WorkerThread::signalizeDataUpdated(const libtorrent::torrent_handle &handle,
                                               const libtorrent::add_torrent_params &params,
                                               const libtorrent::error_code &error)
{
    qDebug() << Q_FUNC_INFO;
    // ************
    // ERROR
    // ************
    // set to the error, if one occurred while adding the torrent.


    // ************
    // INFOS
    // ************
    std::shared_ptr<lt::torrent_info> ti = params.ti;

    if (!ti->is_valid()) {
        TorrentData d;
        d.unique_id = handle.info_hash();
        d.metaInfo.hasError = true;
        emit dataUpdated(d);
        return;
    }

    TorrentData d;
    d.unique_id = handle.info_hash();
    d.detail = toTorrentHandleInfo(handle);

    TorrentMetaInfo m;
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

    for (const std::pair<std::string, int> &node : ti->nodes()) {
        m.nodes.append( TorrentNodeInfo(toString(node.first), node.second) );
    }

    m.bytesMetaData = ti->metadata_size();

    const lt::file_storage files = ti->files();
    for (const lt::file_index_t index : files.file_range()) {

        TorrentFileMetaInfo f;

        lt::file_flags_t flags = files.file_flags(index);
        if (flags & lt::file_storage::flag_pad_file)   f.setFlag(TorrentFileMetaInfo::Flag::PadFile);
        if (flags & lt::file_storage::flag_hidden)     f.setFlag(TorrentFileMetaInfo::Flag::Hidden);
        if (flags & lt::file_storage::flag_executable) f.setFlag(TorrentFileMetaInfo::Flag::Executable);
        if (flags & lt::file_storage::flag_symlink)    f.setFlag(TorrentFileMetaInfo::Flag::Symlink);

        f.hash              = toString(files.hash(index));

        if (f.flags.testFlag(TorrentFileMetaInfo::Flag::Symlink))
            f.symlink       = toString(files.symlink(index));

        f.modifiedTime      = toDateTime(files.mtime(index));
        f.filePath          = toString(files.file_path(index)) ;
        f.fileName          = toString(files.file_name(index));
        f.bytesTotal        = files.file_size(index);
        f.isPadFile         = files.pad_file_at(index);
        f.bytesOffset       = files.file_offset(index);
        f.isPathAbsolute    = files.file_absolute_path(index);
        f.crc32FilePathHash = files.file_path_hash(index, std::string());

        m.files.append(f);
    }


    for (const lt::announce_entry &tracker : ti->trackers()) {
        TorrentTrackerInfo t = toTorrentTrackerInfo(tracker);
        m.trackers.append(t);
    }

    for (const lt::sha1_hash &similar_torrent : ti->similar_torrents()) {
        m.similarTorrents.append(toString(similar_torrent));
    }

    for (const std::string &collection : ti->collections()) {
        m.collections.append(toString(collection));
    }

    for (const lt::web_seed_entry &web_seed : ti->web_seeds()) {
        TorrentWebSeedMetaInfo w;
        w.url   = toString(web_seed.url);
        w.auth  = toString(web_seed.auth);
        for (const std::pair<std::string, std::string> &extra_header : web_seed.extra_headers) {
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

    // ************
    // PARAMS            // is it modified by user before resume it ?
    // ************
    for (const std::string &tracker : params.trackers) {
        m.trackers2.append(toString(tracker));
    }
    for (const std::pair<std::string, int> &dht_node : params.dht_nodes) {
        m.dhtNodes.append( TorrentNodeInfo(toString(dht_node.first), dht_node.second) );

    }
    m.outputName    = toString(params.name);
    m.outputPath    = toString(params.save_path);


    m.defaultTrackerId = toString(params.trackerid);

    if (m.infohash.isEmpty()) {
        m.infohash = toString(params.info_hash);
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

    for (const std::string &http_seed : params.http_seeds) { // todo unify http et url
        m.httpSeeds.append(toString(http_seed));
    }

    for (const std::string &url_seed : params.url_seeds) {
        m.urlSeeds.append(toString(url_seed));

    }

    for (const lt::tcp::endpoint &peer : params.peers) {
        TorrentPeerMetaInfo p;
        p.endpoint = toEndPoint(peer);
        m.defaultPeers.append(p);
    }

    for (const lt::tcp::endpoint &banned_peer : params.banned_peers) {
        TorrentPeerMetaInfo p;
        p.endpoint = toEndPoint(banned_peer);
        m.bannedPeers.append(p);
    }

    m.lastTimeDownload = toDateTime(params.last_download);
    m.lastTimeUpload   = toDateTime(params.last_upload);


    d.metaInfo = m;
    emit dataUpdated(d);
}

/******************************************************************************
 ******************************************************************************/
inline void WorkerThread::signalizeStatusUpdated(const std::vector<lt::torrent_status> &vec)
{
    foreach (const lt::torrent_status &status, vec) {
        signalizeStatusUpdated(status);
    }
}

/******************************************************************************
 ******************************************************************************/
inline void WorkerThread::signalizeStatusUpdated(const lt::torrent_status &status)
{
    qDebug() << Q_FUNC_INFO;
    lt::torrent_handle handle = status.handle;

    TorrentStatus s;
    s.unique_id = handle.info_hash();
    s.detail = toTorrentHandleInfo(handle);

    TorrentInfo t;

    // ***************
    // Errors
    // ***************
    t.hasError = (static_cast<int>(status.errc.value()) != 0);
    if (t.hasError) {
        qDebug() << Q_FUNC_INFO << toString(status.errc.message());

        if (status.error_file == lt::torrent_status::error_file_none) { /* -1 */
            // Other error type (not file error)
            t.error = TorrentError(TorrentError::UnknownError);

        } else if (status.error_file == lt::torrent_status::error_file_ssl_ctx) {
            t.error = TorrentError(TorrentError::SSLContextError);

        } else if (status.error_file == lt::torrent_status::error_file_metadata) {
            t.error = TorrentError(TorrentError::FileMetadataError);

        } else if (status.error_file == lt::torrent_status::error_file_exception) {
            t.error = TorrentError(TorrentError::FileExceptionError);

        } else if (status.error_file == lt::torrent_status::error_file_partfile) {
            t.error = TorrentError(TorrentError::PartFileError);

        } else {
            const int errorFileIndex = static_cast<int>(status.error_file);
            if (errorFileIndex >= 0) {
                t.error = TorrentError(TorrentError::FileError, errorFileIndex);
            }
        }
    }

    // ***************
    // Trackers
    // ***************
    t.lastWorkingTrackerUrl = toString(status.current_tracker);


    // ***************
    // Stats
    // ***************
    t.bytesSessionDownloaded    = status.total_download;
    t.bytesSessionUploaded      = status.total_upload;

    t.bytesSessionPayloadDownload   = status.total_payload_download;
    t.bytesSessionPayloadUpload     = status.total_payload_upload;

    t.bytesFailed       = status.total_failed_bytes;
    t.bytesRedundant    = status.total_redundant_bytes;

    t.bytesReceived     = status.total_done;
    t.bytesTotal        = status.total;

    t.bytesWantedReceived   = status.total_wanted_done;
    t.bytesWantedTotal      = status.total_wanted;

    t.bytesAllSessionsPayloadDownload   = status.all_time_upload;
    t.bytesAllSessionsPayloadUpload     = status.all_time_download;

    t.addedTime             = toDateTime(status.added_time);
    t.completedTime         = toDateTime(status.completed_time);
    t.lastSeenCompletedTime = toDateTime(status.last_seen_complete);

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

    t.blockSizeInByte = status.block_size;

    t.peersUnchokedCount     = status.num_uploads;
    t.peersConnectionCount   = status.num_connections;

    t.uploadSlotsLimit       = status.uploads_limit;
    t.connectionsNumberLimit = status.connections_limit;

    t.upBandwidthQuotaQueue    = status.up_bandwidth_queue;
    t.downBandwidthQuotaQueue  = status.down_bandwidth_queue;

    t.seedRank = status.seed_rank;

    switch (static_cast<int>(status.state)) {
    case lt::torrent_status::checking_files:
        t.status = IDownloadItem::Idle;

        break;
    case lt::torrent_status::downloading_metadata:
        t.status = IDownloadItem::Preparing;

        break;
    case lt::torrent_status::downloading:
        t.status = IDownloadItem::Downloading;

        break;
    case lt::torrent_status::finished:
    case lt::torrent_status::seeding:
    case lt::torrent_status::allocating:
    case lt::torrent_status::checking_resume_data:
        t.status = IDownloadItem::Completed;

        break;
    default:
        break;
    }

    t.isSeeding                 = status.is_seeding;
    t.isFinished                = status.is_finished;
    t.hasMetadata               = status.has_metadata;
    t.hasIncoming               = status.has_incoming;
    t.isMovingStorage           = status.moving_storage;

    t.isAnnouncingToTrackers    = status.announcing_to_trackers;
    t.isAnnouncingToLSD         = status.announcing_to_lsd;
    t.isAnnouncingToDHT         = status.announcing_to_dht;

    t.infohash = toString(status.info_hash);

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
inline TorrentHandleInfo WorkerThread::toTorrentHandleInfo(const libtorrent::torrent_handle &handle) const
{
    qDebug() << Q_FUNC_INFO;
    TorrentHandleInfo t;

    t.uploadBandwidthLimit      = handle.upload_limit();
    t.downloadBandwidthLimit    = handle.download_limit();
    t.maxUploads                = handle.max_uploads();
    t.maxConnections            = handle.max_connections();

    // ***************
    // Files
    // ***************
    std::vector<std::int64_t> progress;
    handle.file_progress(progress, lt::torrent_handle::piece_granularity);
    std::vector<lt::download_priority_t> priorities = handle.get_file_priorities();

    const int count = std::min({ static_cast<int>(handle.torrent_file()->num_files()),
                                 static_cast<int>(progress.size()),
                                 static_cast<int>(priorities.size())
                               });
    for (int index = 0; index < count; ++index) {
        lt::file_index_t findex = static_cast<lt::file_index_t>(index);
        TorrentFileInfo fi;
        fi.bytesReceived = progress.at(static_cast<std::size_t>(index));
        fi.priority = toPriority(handle.file_priority(findex));
        t.files.append(fi);
    }

    // ***************
    // Peers
    // ***************
    std::vector<lt::peer_info> peers;
    handle.get_peer_info(peers);
    for (const lt::peer_info &peer : peers) {
        TorrentPeerInfo d;

        d.endpoint.ip = toString(peer.ip.address().to_string());
        d.endpoint.port = peer.ip.port();
        d.client = toString(peer.client);

        // todo piecesHoldByThePeer = peer.pieces;

        d.bytesDownloaded = peer.total_download;
        d.bytesUploaded = peer.total_upload;

        d.lastTimeRequested = peer.last_request.count();
        d.lastTimeActive    = peer.last_active.count();
        d.timeDownloadQueue = peer.download_queue_time.count();

        lt::peer_flags_t flags = peer.flags;
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

        lt::peer_source_flags_t sourceFlags = peer.source;
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
    std::vector<lt::announce_entry> trackers = handle.trackers();
    for (const lt::announce_entry &tracker : trackers) {
        TorrentTrackerInfo ti = toTorrentTrackerInfo(tracker);
        t.trackers.append(ti);
    }

    // ***************
    // Seeds
    // ***************
    for (const std::string &webSeed : handle.http_seeds()) {
        t.httpSeeds.append(toString(webSeed));
    }
    for (const std::string &webSeed : handle.url_seeds()) {
        t.urlSeeds.append(toString(webSeed));
    }

    return t;
}


/******************************************************************************
 ******************************************************************************/
inline QString WorkerThread::toString(const std::string &s) const
{
    return QString::fromStdString(s);
}

inline QString WorkerThread::toString(const lt::string_view &s) const
{
    return QString::fromStdString(s.to_string()).toUpper();
}

/******************************************************************************
 ******************************************************************************/
inline QString WorkerThread::toString(const lt::sha1_hash &hash) const
{
    if (!hash.is_all_zeros()) {
        return QString::fromStdString(lt::aux::to_hex(hash)).toUpper();
    }
    return QString();
}

/******************************************************************************
 ******************************************************************************/
inline QDateTime WorkerThread::toDateTime(const std::time_t &time) const
{
    qint64 sec = static_cast<qint64>(time);
    return sec > 0
            ? QDateTime(QDate(1970, 1, 1), QTime(0, 0), Qt::UTC).addSecs(sec)
            : QDateTime();
}


/******************************************************************************
 ******************************************************************************/
static inline EndPoint toEndPoint(const lt::tcp::endpoint &endpoint)
{
    EndPoint ret;
    ret.ip = QString::fromStdString(endpoint.address().to_string());
    ret.port = endpoint.port();
    return ret;
}

static inline lt::tcp::endpoint fromEndPoint(const EndPoint &endpoint)
{
    lt::tcp::endpoint ret;
    ret.address(boost::asio::ip::make_address(endpoint.ip.toStdString()));
    ret.port(static_cast<unsigned short>(endpoint.port));
    return ret;
}

/******************************************************************************
 ******************************************************************************/
static inline TorrentTrackerInfo::Source toTrackerSource(
        const lt::announce_entry::tracker_source &s)
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
static inline TorrentTrackerInfo toTorrentTrackerInfo(const lt::announce_entry &tracker)
{
    TorrentTrackerInfo t;
    t.url           = QString::fromStdString(tracker.url);
    t.trackerId     = QString::fromStdString(tracker.trackerid);
    for (const lt::announce_endpoint &endpoint : tracker.endpoints) {
        EndPoint e = toEndPoint(endpoint.local_endpoint);
        t.endpoints.append(e);
    }
    t.tier          = static_cast<int>(tracker.tier);
    t.failLimit     = static_cast<int>(tracker.fail_limit);
    t.isVerified    = tracker.verified;
    t.source        = toTrackerSource(static_cast<lt::announce_entry::tracker_source>(tracker.source));
    return t;
}

static inline lt::announce_entry fromTorrentTrackerInfo(const TorrentTrackerInfo &tracker)
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
static inline TorrentFileInfo::Priority toPriority(const lt::download_priority_t &p)
{
    if      (p == lt::dont_download   )  return TorrentFileInfo::Priority::Ignore;
    else if (p == lt::low_priority    )  return TorrentFileInfo::Priority::Low;
    else if (p == lt::top_priority    )  return TorrentFileInfo::Priority::High;
    else   /*p == lt::default_priority*/ return TorrentFileInfo::Priority::Normal;
}

static inline lt::download_priority_t fromPriority(const TorrentFileInfo::Priority &p)
{
    switch (p) {
    case TorrentFileInfo::Priority::Ignore: return lt::dont_download;
    case TorrentFileInfo::Priority::Low:    return lt::low_priority;
    case TorrentFileInfo::Priority::High:   return lt::top_priority;
    case TorrentFileInfo::Priority::Normal:
    default:                                return lt::default_priority;
    }
}

/******************************************************************************
 ******************************************************************************/
inline std::string WorkerThread::userAgent()
{
    return QString("%0 %1").arg(STR_APPLICATION_NAME).arg(STR_APPLICATION_VERSION).toStdString();
}

/******************************************************************************
 ******************************************************************************/
void WorkerThread::log(lt::alert *s)
{
    qDebug() << "[alert]" << QString::fromStdString(s->message());
}
