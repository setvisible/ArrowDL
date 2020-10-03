/* - DownZemAll! - Copyright (C) 2019-present Sebastien Vavassori
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

#include <Globals>
#include <Core/NetworkManager>
#include <Core/ResourceItem>
#include <Core/Settings>
#include <Core/Torrent>

#include <QtCore/QDebug>
#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtCore/QtMath>
#include <QtNetwork/QNetworkReply>

#include <algorithm> // std::min, std::max
#include <chrono>
#include <fstream>   // std::fstream
#include <string>    // std::string


#include "libtorrent/add_torrent_params.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/announce_entry.hpp"
#include "libtorrent/bdecode.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/config.hpp"
#include "libtorrent/create_torrent.hpp"
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


/* Show the thread, of which the debug message comes from */
#define qDebug_1 qDebug() << " + | "
#define qDebug_2 qDebug() << " | + "


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

static inline TorrentError toTorrentError(const lt::error_code &errc, const lt::file_index_t &error_file);

static inline TorrentTrackerInfo toTorrentTrackerInfo(const lt::announce_entry &tracker);
static inline lt::announce_entry fromTorrentTrackerInfo(const TorrentTrackerInfo &tracker);

static inline TorrentFileInfo::Priority toPriority(const lt::download_priority_t &p);
static inline lt::download_priority_t fromPriority(const TorrentFileInfo::Priority &p);

static inline TorrentInfo::TorrentState toState(const lt::torrent_status::state_t &s);
static inline lt::torrent_status::state_t fromState(const TorrentInfo::TorrentState &s);


TorrentContextPrivate::TorrentContextPrivate(TorrentContext *qq)
    : QObject(qq)
    , q(qq)
    , workerThread(Q_NULLPTR)
    , settings(Q_NULLPTR)
    , networkManager(Q_NULLPTR)
{
    qRegisterMetaType<TorrentData>("TorrentData");
    qRegisterMetaType<TorrentStatus>("TorrentStatus");

    workerThread = new WorkerThread(this);

    connect(workerThread, &WorkerThread::metadataUpdated, this, &TorrentContextPrivate::onMetadataUpdated);
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
        qDebug_1 << Q_FUNC_INFO << "Terminating...";
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
    QMapIterator<QString, QVariant> it(map);
    while (it.hasNext()) {
        it.next();
        auto key = it.key();
        auto value = it.value();

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
QVariant TorrentContextPrivate::_get_str(const lt::settings_pack &pack, int index)
{
    return QString::fromStdString(pack.get_str(index));
}

QVariant TorrentContextPrivate::_get_int(const lt::settings_pack &pack, int index)
{
    return pack.get_int(index);
}

QVariant TorrentContextPrivate::_get_bool(const lt::settings_pack &pack, int index)
{
    return pack.get_bool(index);
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::onStopped()
{  
    qDebug_1 << Q_FUNC_INFO; // Just to confirm it's stopped:)
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::onMetadataUpdated(TorrentData data)
{
    qDebug_1 << Q_FUNC_INFO;
    Torrent *torrent = find(data.unique_id);
    if (torrent) {
        torrent->setDetail(data.detail, true);
        torrent->setMetaInfo(data.metaInfo); // setMetaInfo will emit the GUI update signal

        lt::torrent_handle handle = workerThread->findTorrent(data.unique_id);
        if (handle.is_valid()) {
            std::shared_ptr<lt::torrent_info const> ti = handle.torrent_file();
            if (ti) {
                const QString torrentFile = torrent->localFullFileName();  // destination
                ensureDestinationPathExists(torrent);

                writeTorrentFileFromMagnet(torrentFile, ti);

                /*
                 * The .torrent file is immediately loaded after being written,
                 * so that the program ensures the file is correctly written
                 * (compliant with the bittorrent format specification).
                 */
                readTorrentFile(torrentFile, torrent);

            }
            removeTorrent(torrent);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::onDataUpdated(TorrentData data)
{
    qDebug_1 << Q_FUNC_INFO;
    Torrent *torrent = find(data.unique_id);
    if (torrent) {
        torrent->setDetail(data.detail, true);
        torrent->setMetaInfo(data.metaInfo); // setMetaInfo will emit the GUI update signal
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::onStatusUpdated(TorrentStatus status)
{
    qDebug_1 << Q_FUNC_INFO;
    Torrent *torrent = find(status.unique_id);
    if (torrent) {
        torrent->setInfo(status.info, false);
        torrent->setDetail(status.detail, false);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::ensureDestinationPathExists(Torrent *torrent)
{
    const QString path = torrent->localFullFileName();
    const QFileInfo fi(path);
    const QString outputPath = fi.absolutePath();
    QDir().mkpath(outputPath);
}

/******************************************************************************
 ******************************************************************************/
static bool isMagnetSource(const QString &source)
{
    const QUrl url = QUrl::fromUserInput(source);
    return url.scheme().toLower() == QLatin1String("magnet");
}

static bool isTorrentSource(const QString &source)
{
    const QUrl url = QUrl::fromUserInput(source);
    QFileInfo fi(url.path());
    return fi.suffix().toLower() == QLatin1String("torrent");
}

static bool isLocalSource(const QString &source)
{
    return isTorrentSource(source) && QUrl::fromUserInput(source).isLocalFile();
}

static bool isDistantSource(const QString &source)
{
    return isTorrentSource(source) && !QUrl::fromUserInput(source).isLocalFile();
}

static bool isInfoHashSource(const QString &/*source*/)
{
    return false; // TODO
}

static QString localSource(const QString &source)
{
    if (QFileInfo::exists(source)) {
        return source;
    }

    // url can be percent-encoded or pretty-encoded or not encoded at all.
    // Try to figure out the correct path
    const QUrl url = QUrl::fromUserInput(source);
    const QString localFile = url.toLocalFile();
    if (QFileInfo::exists(localFile)) {
        return localFile;
    }
    const QString fromPercentEncoding = QUrl::fromPercentEncoding(source.toUtf8());
    if (QFileInfo::exists(fromPercentEncoding)) {
        return fromPercentEncoding;
    }

    // Url from app's argument
    const QUrl url2 = QUrl::fromEncoded(source.toLocal8Bit());
    if (QFileInfo::exists(url2.path())) {
        return fromPercentEncoding;
    }

    return QString();
}

static bool copyFile(const QString &from, const QString &to)
{
    const QString source = localSource(from); // eventually decode percent
    return QFile::copy(source, to);
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::prepareTorrent(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    if (!torrent) {
        return;
    }
    TorrentInfo info = torrent->info();
    info.state = TorrentInfo::downloading_metadata;
    torrent->setInfo(info, false);

    QString torrentFile = torrent->localFullFileName(); // destination

    if (QFileInfo::exists(torrentFile)) {
        readTorrentFile(torrentFile, torrent);
        return;

    } else {
        ensureDestinationPathExists(torrent);
        const QString source = torrent->url();

        if (isMagnetSource(source)) {
            downloadMagnetLink(torrent);

        } else if (isDistantSource(source)) {
            downloadTorrentFile(torrent);

        } else if (isLocalSource(source)) { // Trivial: just move and read
            if (copyFile(source, torrentFile)) {
                readTorrentFile(torrentFile, torrent);
            } else {
                qDebug_1 << Q_FUNC_INFO << "FILE COPY ERROR";
            }

        } else if (isInfoHashSource(source)) {
            // TODO
        } else {
            qDebug_1 << Q_FUNC_INFO << "error: can't prepare, invalid format";
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::stopPrepare(Torrent *torrent)
{   
    abortNetworkReply(torrent); // abort conventional download
    removeTorrent(torrent);     // also abort magnet metainfo download
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::downloadMagnetLink(Torrent *torrent)
{
    const QString source = torrent->url();
    const QUrl url = QUrl::fromUserInput(source);

    qDebug_1 << Q_FUNC_INFO << url;

    // For magnet link, libtorrent will download the torrent metadata before
    // the torrent itself
    addTorrent(torrent);
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Download .torrent file with regular download engine
 */
void TorrentContextPrivate::downloadTorrentFile(Torrent *torrent)
{
    const QString source = torrent->url();
    const QUrl url = QUrl::fromUserInput(source);

    qDebug_1 << Q_FUNC_INFO << url;

    Q_ASSERT(networkManager);
    QNetworkReply *reply = networkManager->get(url);
    if (!reply) {
        auto m = torrent->metaInfo();
        m.error.type = TorrentError::MetadataDownloadError;
        m.error.message = tr("Network request rejected.");
        torrent->setMetaInfo(m);
        return;
    }
    connect(reply, &QNetworkReply::finished,
            this, &TorrentContextPrivate::onNetworkReplyFinished,
            Qt::UniqueConnection);

    m_currentDownloads.insert(reply, torrent);
}

void TorrentContextPrivate::abortNetworkReply(Torrent *torrent)
{
    QMapIterator<QNetworkReply*, Torrent*> it(m_currentDownloads);
    while (it.hasNext()) {
        it.next();
        QNetworkReply* currentReply = it.key();
        Torrent* currentTorrent = it.value();
        if (currentTorrent == torrent) {
            currentReply->abort();
            /*
             * Rem: Do not remove(reply*) at this point, onFinished() will do that
             */
        }
    }
}

void TorrentContextPrivate::onNetworkReplyFinished()
{
    qDebug_1 << Q_FUNC_INFO;

    auto reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    Torrent *torrent = Q_NULLPTR;
    if (m_currentDownloads.contains(reply)) {
        torrent = m_currentDownloads.take(reply);
    }
    if (!torrent) {
        return;
    }

    QUrl url = reply->url();
    qDebug_1 << Q_FUNC_INFO << url;

    if (reply->error() != QNetworkReply::NoError) {
        TorrentMetaInfo m = torrent->metaInfo();
        m.error.type = TorrentError::MetadataDownloadError;
        m.error.message = QString("%0\n\n%1\n\n%2").arg(
                    tr("Can't download metadata."),
                    url.toEncoded().constData(),
                    qPrintable(reply->errorString()));
        torrent->setMetaInfo(m);

        reply->deleteLater();
        return;
    }
    if (reply->bytesAvailable() <= 0) {
        TorrentMetaInfo m = torrent->metaInfo();
        m.error.type = TorrentError::MetadataDownloadError;
        m.error.message = tr("No metadata downloaded.");
        torrent->setMetaInfo(m);

        reply->deleteLater();
        return;
    }

    reply->deleteLater();

    QString torrentFile = torrent->localFullFileName(); // destination
    writeTorrentFile(torrentFile, reply);
    /*
     * The .torrent file is immediately loaded after being written,
     * so that the program ensures the file is correctly written
     * (compliant with the bittorrent format specification).
     */
    readTorrentFile(torrentFile, torrent);
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::archiveExistingFile(const QString &filename)
{
    if (QFileInfo::exists(filename)) {
        QString archive = QString("%0.0.old").arg(filename);
        int i = 0;
        while (QFileInfo::exists(archive)) {
            i++;
            archive = QString("%0.%1.old").arg(filename, QString::number(i));
        }
        archive.append(QString::number(i));
        if (!QFile::rename(filename, archive)) {
            qDebug_1 << "Cannot archive file" << filename << "to" << archive;
        }
    }
}

void TorrentContextPrivate::writeTorrentFile(const QString &filename, QIODevice *data)
{
    archiveExistingFile(filename);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data->readAll());
        file.close();
    }
}

void TorrentContextPrivate::writeTorrentFileFromMagnet(
        const QString &filename, std::shared_ptr<lt::torrent_info const> ti)
{
    archiveExistingFile(filename);

    // Bittorrent Encoding
    lt::create_torrent ct(*ti);
    lt::entry te = ct.generate();
    std::vector<char> buffer;
    bencode(std::back_inserter(buffer), te);

    // Write
    QByteArray data(&buffer[0], static_cast<int>(buffer.size()));
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
    }
    // Alternative:
    // QByteArray ba = another.toLocal8Bit();
    // const char* filename = ba.constData();
    // FILE* f = fopen(filename, "wb+");
    // if (f) {
    //     fwrite(&buffer[0], 1, buffer.size(), f);
    //     fclose(f);
    // }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::readTorrentFile(const QString &filename, Torrent *torrent)
{
    if (!torrent) {
        return;
    }
    if (!QFileInfo::exists(filename)) {
        return;
    }

    TorrentMetaInfo metaInfo = torrent->metaInfo();
    metaInfo.initialMetaInfo = workerThread->dump(filename);

    TorrentInfo info = torrent->info();
    info.state = TorrentInfo::stopped;
    torrent->setInfo(info, true);
    torrent->setMetaInfo(metaInfo); // setMetaInfo will emit the GUI update signal

    // Do a fake 'detail' update, to initialize the torrent.
    TorrentHandleInfo detail = torrent->detail();
    detail.files.clear();
    for (int index = 0; index < metaInfo.initialMetaInfo.files.count(); ++index) {
        TorrentFileInfo fi;
        fi.priority = TorrentFileInfo::Normal;
        detail.files.append(fi);
    }
    torrent->setDetail(detail, false);
}

/******************************************************************************
 ******************************************************************************/
bool TorrentContextPrivate::hasTorrent(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    return h.is_valid();
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief return false on failure
 */
bool TorrentContextPrivate::addTorrent(Torrent *torrent) // resumeTorrent
{
    TorrentInfo info = torrent->info();
    info.state = TorrentInfo::checking_files;
    torrent->setInfo(info, false);

    QString source = torrent->url();
    QString torrentFile = torrent->localFullFileName(); // destination

    if (QFileInfo::exists(torrentFile)) {
        source = torrentFile;
    }

    qDebug_1 << Q_FUNC_INFO << source;

    ensureDestinationPathExists(torrent);
    const QString outputPath = torrent->localFilePath();

    lt::add_torrent_params p;

    if (isMagnetSource(source)) { // Add from magnet link
        TorrentInfo info = torrent->info();
        info.state = TorrentInfo::downloading_metadata;
        torrent->setInfo(info, false);

        QByteArray bytes = source.toLatin1();
        // QByteArray bytes = source.toUtf8();

        const char * ptr = bytes.constData();
        boost::string_view::size_type size
                = static_cast<boost::string_view::size_type>(bytes.size());

        lt::string_view uri { ptr, size };
        lt::error_code ec;
        p = lt::parse_magnet_uri(uri.to_string(), ec);
        if (ec) {
            qDebug_1 << "invalid magnet link:";
            qDebug_1 << QString::fromStdString(uri.to_string());
            qDebug_1 << QString::fromStdString(ec.message());
            return false;
        }

        // https://www.libtorrent.org/manual-ref.html#magnet-links
        p.file_priorities.clear();
        // First 1,000,000 files in the torrent will not be downloaded
        p.file_priorities.assign(1000000, lt::dont_download);

    } else {
        if (isTorrentSource(source)) { // Add from .torrent file
            std::string torrent = source.toStdString();
            lt::error_code ec;
            auto ti = std::make_shared<lt::torrent_info>(torrent, ec);
            if (ec) {
                qDebug_1 << "failed to load torrent";
                qDebug_1 << QString::fromStdString(torrent);
                qDebug_1 << QString::fromStdString(ec.message());
                return false;
            }
            p.ti = ti;

        } else {

            // Add from the info-hash of the torrent
            //
            // set this to the info hash of the torrent to add in case the info-hash
            // is the only known property of the torrent. i.e. you don't have a
            // .torrent file nor a magnet link.

            lt::sha1_hash infohash(source.toStdString());
            lt::add_torrent_params p;
            p.info_hash = infohash;
        }

        p.file_priorities.clear();
        for (int fi = 0; fi < torrent->fileCount(); ++fi) {
            auto priority = fromPriority(torrent->filePriority(fi));
            p.file_priorities.push_back(priority);
        }
    }


    p.flags &= ~lt::torrent_flags::duplicate_is_error; // do not raise exception if duplicate


    p.save_path = outputPath.toStdString();

    // Blocking insertion
    lt::error_code ec2;
    lt::torrent_handle h = workerThread->addTorrent(std::move(p), ec2);
    if (ec2) {
        qDebug_1 << "failed to load torrent";
        qDebug_1 << QString::fromStdString(source.toStdString());
        qDebug_1 << QString::fromStdString(ec2.message());
        return false;
    }

    if (!h.is_valid()) {
        return false;
    }

    h.pause();

    const UniqueId uuid = WorkerThread::toUniqueId(h.info_hash());
    hashMap.insert(uuid, torrent);
    return true;
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::removeTorrent(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        workerThread->removeTorrent(h); // needs calling lt::session
        const UniqueId uuid = WorkerThread::toUniqueId(h.info_hash());
        hashMap.remove(uuid);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::resumeTorrent(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.resume();
    }
}

void TorrentContextPrivate::pauseTorrent(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.pause();
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::moveQueueUp(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.queue_position_up();
    }
}

void TorrentContextPrivate::moveQueueDown(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.queue_position_down();
    }
}

void TorrentContextPrivate::moveQueueTop(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.queue_position_top();
    }
}

void TorrentContextPrivate::moveQueueBottom(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.queue_position_bottom();
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::changeFilePriority(Torrent *torrent,
                                               int index, TorrentFileInfo::Priority p)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        lt::file_index_t findex = static_cast<lt::file_index_t>(index);
        lt::download_priority_t priority = fromPriority(p);
        h.file_priority(findex, priority);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::addSeed(Torrent *torrent, const TorrentWebSeedMetaInfo &seed)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        if (seed.type == TorrentWebSeedMetaInfo::Type::UrlSeed) {
            h.add_url_seed(seed.url.toStdString());
        } else {
            h.add_http_seed(seed.url.toStdString());
        }
    }
}

void TorrentContextPrivate::removeSeed(Torrent *torrent, const TorrentWebSeedMetaInfo &seed)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        if (seed.type == TorrentWebSeedMetaInfo::Type::UrlSeed) {
            h.remove_url_seed(seed.url.toStdString());
        } else {
            h.remove_http_seed(seed.url.toStdString());
        }
    }
}

void TorrentContextPrivate::removeAllSeeds(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
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
void TorrentContextPrivate::addPeer(Torrent *torrent, const TorrentPeerInfo &peer)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        lt::tcp::endpoint endpoint = fromEndPoint(peer.endpoint);
        h.connect_peer(endpoint);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::addTracker(Torrent *torrent,
                                       const TorrentTrackerInfo &tracker)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        lt::announce_entry entry = fromTorrentTrackerInfo(tracker);
        h.add_tracker(entry);
    }
}

void TorrentContextPrivate::removeTracker(Torrent *torrent,
                                          const TorrentTrackerInfo &tracker)
{
    qDebug_1 << Q_FUNC_INFO;
    Q_UNUSED(torrent)
    Q_UNUSED(tracker)
    /// \todo
}


/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::forceRecheck(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.force_recheck();
    }
}

void TorrentContextPrivate::forceReannounce(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.force_reannounce();
    }
}

void TorrentContextPrivate::forceDHTReannounce(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.force_dht_announce();
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::setSSLCertificatePath(Torrent *torrent, const QString &path)
{
    qDebug_1 << Q_FUNC_INFO;
    Q_UNUSED(torrent)
    Q_UNUSED(path)
    // todo
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::scrapeTracker(Torrent *torrent, int index)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.scrape_tracker(index);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::setUploadBandwidth(Torrent *torrent, int limit)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.set_upload_limit(limit);
    }
}

void TorrentContextPrivate::setDownloadBandwidth(Torrent *torrent, int limit)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.set_download_limit(limit);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::setMaxUploads(Torrent *torrent, int limit)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.set_max_uploads(limit);
    }
}

void TorrentContextPrivate::setMaxConnections(Torrent *torrent, int limit)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        h.set_max_connections(limit);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContextPrivate::renameFile(Torrent *torrent, int index, const QString &newName)
{
    qDebug_1 << Q_FUNC_INFO;
    lt::torrent_handle h = find(torrent);
    if (h.is_valid()) {
        lt::file_index_t findex = static_cast<lt::file_index_t>(index);
        h.rename_file(findex, newName.toStdString());
    }
}

/******************************************************************************
 ******************************************************************************/
inline Torrent* TorrentContextPrivate::find(const UniqueId &uuid)
{
    qDebug_1 << Q_FUNC_INFO;
    return hashMap.value(uuid, nullptr);
}

inline lt::torrent_handle TorrentContextPrivate::find(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    const UniqueId info_hash = hashMap.key(torrent, UniqueId());
    return workerThread->findTorrent(info_hash);
    //    return hashMap.key(item, lt::torrent_handle());
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
        pack.set_str(lt::settings_pack::user_agent, userAgent());
        pack.set_int(lt::settings_pack::alert_mask, lt::alert::all_categories);

        m_session_ptr->apply_settings(pack);
    }
}

/******************************************************************************
 ******************************************************************************/
static std::vector<char> load_file(std::string const& filename)
{
    std::fstream in;
    in.exceptions(std::ifstream::failbit);
    in.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
    in.seekg(0, std::ios_base::end);
    size_t const size = size_t(in.tellg());
    in.seekg(0, std::ios_base::beg);
    std::vector<char> ret(size);
    in.read(ret.data(), static_cast<std::streamsize>(size));
    return ret;
}

TorrentInitialMetaInfo WorkerThread::dump(const QString &filename) const
{
    std::vector<char> buf = load_file(filename.toStdString());
    lt::error_code ec;
    int pos = -1;
    lt::load_torrent_limits cfg;
    lt::bdecode_node const e = lt::bdecode(buf, ec, &pos,
                                           cfg.max_decode_depth,
                                           cfg.max_decode_tokens);
    if (ec) {
        qDebug_2 << "failed to decode: '"
                 << QString::fromStdString(ec.message())
                 << "' at character: " << pos;
        return TorrentInitialMetaInfo();
    }
    lt::torrent_info const t(std::move(e), cfg);
    // buf.clear();
    std::shared_ptr<lt::torrent_info> ti = std::make_shared<lt::torrent_info>(t);
    return toTorrentInitialMetaInfo(ti);
}

/******************************************************************************
 ******************************************************************************/
UniqueId WorkerThread::toUniqueId(const lt::sha1_hash &hash)
{
    if (!hash.is_all_zeros()) {
        return QString::fromStdString(lt::aux::to_hex(hash)).toUpper();
    }
    return QString();
}

lt::sha1_hash WorkerThread::fromUniqueId(const UniqueId &uuid)
{
    lt::span<char const> in(uuid.toStdString());
    lt::sha1_hash out;
    if (!lt::aux::from_hex(in, out.data())) {
        qDebug_2 << "invalid info-hash";
        return lt::sha1_hash();
    }
    return out;
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
    return lt::torrent_handle();
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
        auto hash = fromUniqueId(uuid);
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
        log(s);
        onTorrentAdded(s->handle, s->params, s->error);
    }
    else if (lt::state_update_alert* s = lt::alert_cast<lt::state_update_alert>(a)) {
        /* Note: This alert is emitted very often (each loop) */
        // log(s);
        onStateUpdated(s->status);
    }

    // magnet extension
    else if (lt::metadata_failed_alert* s = lt::alert_cast<lt::metadata_failed_alert>(a)) {
        //        emit metadataFailed();
        log(s);
    }
    else if (lt::metadata_received_alert* s = lt::alert_cast<lt::metadata_received_alert>(a)) {
        log(s);
        onMetadataReceived(s->handle);
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
        qDebug_2 << Q_FUNC_INFO << "didn't handle the alert: " << message;
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
        d.unique_id = toUniqueId(handle.info_hash());
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
            int fileCount = handle.torrent_file()->num_files();
            for (int i = 0; i < fileCount; i++) {
                lt::file_index_t findex = static_cast<lt::file_index_t>(i);
                handle.file_priority(findex, lt::dont_download);
            }
        }
        handle.pause();

        TorrentData d;
        d.unique_id = toUniqueId(handle.info_hash());
        d.detail = toTorrentHandleInfo(handle);
        if (handle.is_valid()) {
            std::shared_ptr<lt::torrent_info const> ti = handle.torrent_file();
            if (ti) {
                d.metaInfo.initialMetaInfo = toTorrentInitialMetaInfo(ti);
            }
        }
        emit metadataUpdated(d);
    }
}

/******************************************************************************
 ******************************************************************************/
inline void WorkerThread::onStateUpdated(const std::vector<lt::torrent_status> &status)
{
    foreach (const lt::torrent_status &s, status) {
        signalizeStatusUpdated(s);
    }
}

/******************************************************************************
 ******************************************************************************/
inline void WorkerThread::signalizeDataUpdated(const lt::torrent_handle &handle,
                                               const lt::add_torrent_params &params)
{
    qDebug_2 << Q_FUNC_INFO;
    if (!handle.is_valid()) {
        return;
    }
    TorrentData d;
    d.unique_id = toUniqueId(handle.info_hash());
    d.detail = toTorrentHandleInfo(handle);

    std::shared_ptr<lt::torrent_info> ti = params.ti;
    if (!ti || !ti->is_valid()) {
        // it's a magnet link perhaps, metadata has not have been received yet
        d.metaInfo.error = TorrentError(TorrentError::NoInfoYetError);
    }

    d.metaInfo = toTorrentMetaInfo(params);
    emit dataUpdated(d);
}

/******************************************************************************
 ******************************************************************************/
inline void WorkerThread::signalizeStatusUpdated(const lt::torrent_status &status)
{
    qDebug_2 << Q_FUNC_INFO;
    lt::torrent_handle handle = status.handle;
    if (!handle.is_valid()) {
        return;
    }

    TorrentStatus s;
    s.unique_id = toUniqueId(handle.info_hash());
    s.detail = toTorrentHandleInfo(handle);

    TorrentInfo t;

    // ***************
    // Errors
    // ***************
    t.error = toTorrentError(status.errc, status.error_file);


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

    t.downloadedPieces  = toBitArray(status.pieces);
    t.verifiedPieces    = toBitArray(status.verified_pieces);

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

    t.state = toState(status.state);

    //  t.need_save_resume = state.need_save_resume;

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
inline TorrentInitialMetaInfo WorkerThread::toTorrentInitialMetaInfo(std::shared_ptr<lt::torrent_info const> ti) const
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

        if (f.flags.testFlag(TorrentFileMetaInfo::Flag::Symlink)) {
            f.symlink       = toString(files.symlink(index));
        }
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
    return m;
}

/******************************************************************************
 ******************************************************************************/
inline TorrentHandleInfo WorkerThread::toTorrentHandleInfo(const lt::torrent_handle &handle) const
{
    qDebug_2 << Q_FUNC_INFO;
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
        std::vector<lt::download_priority_t> priorities = handle.get_file_priorities();

        // const std::vector<lt::open_file_state> file_status = handle.file_status();


        // auto list = std::initializer_list<int>();
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
    }

    // ***************
    // Peers
    // ***************
    std::vector<lt::peer_info> peers;
    handle.get_peer_info(peers);
    for (const lt::peer_info &peer : peers) {
        TorrentPeerInfo d;

        QString peerIp = toString(peer.ip.address().to_string());
        int peerPort = peer.ip.port();
        d.endpoint = EndPoint(peerIp, peerPort);
        d.userAgent = toString(peer.client);

        d.availablePieces = toBitArray(peer.pieces);

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
inline TorrentMetaInfo WorkerThread::toTorrentMetaInfo(const lt::add_torrent_params &params) const
{
    TorrentMetaInfo m;

    std::shared_ptr<lt::torrent_info const> ti = params.ti;

    m.initialMetaInfo = toTorrentInitialMetaInfo(ti);
    for (const std::string &tracker : params.trackers) {
        m.trackers2.append(toString(tracker));
    }
    // for (const int tracker_tier : params.tracker_tiers) {
    // ?
    // }
    for (const std::pair<std::string, int> &dht_node : params.dht_nodes) {
        m.dhtNodes.append( TorrentNodeInfo(toString(dht_node.first), dht_node.second) );
    }

    if (m.initialMetaInfo.name.isEmpty()) {
        m.initialMetaInfo.name = toString(params.name);
    }
    m.outputPath    = toString(params.save_path);

    // for (const lt::download_priority_t &file_priority : params.file_priorities) {
    // ?
    // }

    m.defaultTrackerId = toString(params.trackerid);
    //  m.flags         = toString(params.flags); // TODO

    if (m.initialMetaInfo.infohash.isEmpty()) {
        m.initialMetaInfo.infohash = toString(params.info_hash);
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
        TorrentPeerInfo p(toEndPoint(peer), QString());
        m.defaultPeers.append(p);
    }

    for (const lt::tcp::endpoint &banned_peer : params.banned_peers) {
        TorrentPeerInfo p(toEndPoint(banned_peer), QString());
        m.bannedPeers.append(p);
    }
    // for (const std::string &unfinished_piece : params.unfinished_pieces) {
    // }

    m.downloadedPieces = toBitArray(params.have_pieces);
    m.verifiedPieces   = toBitArray(params.verified_pieces);

    m.lastTimeDownload = toDateTime(params.last_download);
    m.lastTimeUpload   = toDateTime(params.last_upload);

    return m;
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
QBitArray WorkerThread::toBitArray(const lt::typed_bitfield<lt::piece_index_t> &vec) const
{
    auto size = vec.size();
    QBitArray ba(size, false);
    for (int i = 0; i < size; ++i) {
        if (vec.get_bit(static_cast<lt::piece_index_t>(i))) {
            ba.setBit(i);
        }
    }
    return ba;
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
    auto ip = QString::fromStdString(endpoint.address().to_string());
    auto port = endpoint.port();
    return EndPoint(ip, port);
}

static inline lt::tcp::endpoint fromEndPoint(const EndPoint &endpoint)
{
    lt::tcp::endpoint ret;
    ret.address(boost::asio::ip::make_address(endpoint.ipToString().toStdString()));
    ret.port(static_cast<unsigned short>(endpoint.port()));
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


static inline TorrentError toTorrentError(const lt::error_code &errc, const lt::file_index_t &error_file)
{
    TorrentError error;
    bool hasError = (static_cast<int>(errc.value()) != 0);
    if (!hasError) {
        error = TorrentError();
    } else {
        error.message = QString::fromStdString(errc.message());

        qDebug_2 << Q_FUNC_INFO << error.message;

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
static inline TorrentTrackerInfo toTorrentTrackerInfo(const lt::announce_entry &tracker)
{
    TorrentTrackerInfo t(QString::fromStdString(tracker.url));
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
    case TorrentFileInfo::Priority::Normal: return lt::default_priority;
    }
    Q_UNREACHABLE();
}

/******************************************************************************
 ******************************************************************************/
static inline TorrentInfo::TorrentState toState(const lt::torrent_status::state_t &s)
{
    if      (s == lt::torrent_status::checking_files        ) return TorrentInfo::checking_files        ;
    else if (s == lt::torrent_status::downloading_metadata  ) return TorrentInfo::downloading_metadata  ;
    else if (s == lt::torrent_status::downloading           ) return TorrentInfo::downloading           ;
    else if (s == lt::torrent_status::finished              ) return TorrentInfo::finished              ;
    else if (s == lt::torrent_status::seeding               ) return TorrentInfo::seeding               ;
    else if (s == lt::torrent_status::allocating            ) return TorrentInfo::allocating            ;
    else /* s == lt::torrent_status:: */ return TorrentInfo::stopped;
    Q_UNREACHABLE();
}

static inline lt::torrent_status::state_t fromState(const TorrentInfo::TorrentState &s)
{
    switch (s) {
    case TorrentInfo::stopped               : return lt::torrent_status::finished; // ?
    case TorrentInfo::checking_files        : return lt::torrent_status::checking_files;
    case TorrentInfo::downloading_metadata  : return lt::torrent_status::downloading_metadata;
    case TorrentInfo::downloading           : return lt::torrent_status::downloading;
    case TorrentInfo::finished              : return lt::torrent_status::finished;
    case TorrentInfo::seeding               : return lt::torrent_status::seeding;
    case TorrentInfo::allocating            : return lt::torrent_status::allocating;
    case TorrentInfo::checking_resume_data  : return lt::torrent_status::checking_resume_data;
    }
    Q_UNREACHABLE();
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
    qDebug_2 << "[alert]" << QString::fromStdString(s->message());
}
