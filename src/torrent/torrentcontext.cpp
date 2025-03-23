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

#include "torrentcontext.h"

#include <Core/NetworkManager>
#include <Core/Settings>
#include <Torrent/Torrent>
#include <Torrent/Utils>
#include <Torrent/WorkerThread>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>

#include <chrono>

#include "libtorrent/add_torrent_params.hpp"
#include "libtorrent/bdecode.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/create_torrent.hpp"
#include "libtorrent/magnet_uri.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/settings_pack.hpp"
#include "libtorrent/version.hpp"

/* Show the thread, of which the debug message comes from */
#define qDebug_1 qDebug() << " + | " // from TorrentContext
#define qDebug_2 qDebug() << " | + " // from WorkerThread

using namespace Qt::Literals::StringLiterals;

const std::chrono::milliseconds TIMEOUT_TERMINATING( 3000 );
const double ONE_THIRD = 0.3334;
const double TWO_THIRDS = 0.6667;

static bool isMagnetSource(const QString &source)
{
    auto url = QUrl::fromUserInput(source);
    return url.scheme().toLower() == "magnet"_L1;
}

static bool isTorrentSource(const QString &source)
{
    auto url = QUrl::fromUserInput(source);
    QFileInfo fi(url.path());
    return fi.suffix().toLower() == "torrent"_L1;
}

static bool isLocalSource(const QString &source)
{
    return isTorrentSource(source) && QUrl::fromUserInput(source).isLocalFile();
}

static bool isDistantSource(const QString &source)
{
    return isTorrentSource(source) && !QUrl::fromUserInput(source).isLocalFile();
}

static bool isInfoHashSource(const QString &source)
{
    Q_UNUSED(source)
    return false; /// \todo implement it
}

static QString localSource(const QString &source)
{
    if (QFileInfo::exists(source)) {
        return source;
    }

    // url can be percent-encoded or pretty-encoded or not encoded at all.
    // Try to figure out the correct path
    auto url = QUrl::fromUserInput(source);
    auto localFile = url.toLocalFile();
    if (QFileInfo::exists(localFile)) {
        return localFile;
    }
    auto fromPercentEncoding = QUrl::fromPercentEncoding(source.toUtf8());
    if (QFileInfo::exists(fromPercentEncoding)) {
        return fromPercentEncoding;
    }

    // Url from app's argument
    auto url2 = QUrl::fromEncoded(source.toLocal8Bit());
    if (QFileInfo::exists(url2.path())) {
        return fromPercentEncoding;
    }

    return {};
}

static bool copyFile(const QString &from, const QString &to)
{
    auto source = localSource(from); // eventually decode percent
    return QFile::copy(source, to);
}

/******************************************************************************
 ******************************************************************************/
TorrentContext& TorrentContext::getInstance()
{
    static TorrentContext instance; // lazy singleton, instantiated on first use
    return instance;
}

TorrentContext::TorrentContext() : QObject()
    , m_workerThread(new WorkerThread(this))
{
    qRegisterMetaType<TorrentData>("TorrentData");
    qRegisterMetaType<TorrentStatus>("TorrentStatus");

    connect(m_workerThread, SIGNAL(metadataUpdated(TorrentData)),
            this, SLOT(onMetadataUpdated(TorrentData)));

    connect(m_workerThread, SIGNAL(dataUpdated(TorrentData)),
            this, SLOT(onDataUpdated(TorrentData)));

    connect(m_workerThread, SIGNAL(statusUpdated(TorrentStatus)),
            this, SLOT(onStatusUpdated(TorrentStatus)));

    connect(m_workerThread, SIGNAL(stopped()), this, SLOT(onStopped()));
    connect(m_workerThread, SIGNAL(finished()), m_workerThread, SLOT(deleteLater()));

    m_workerThread->setEnabled(false);
    m_workerThread->start();
}

TorrentContext::~TorrentContext()
{
    m_workerThread->stop();
    if (!m_workerThread->wait(TIMEOUT_TERMINATING.count())) {
        qDebug_1 << Q_FUNC_INFO << "Terminating...";
        m_workerThread->terminate();
        m_workerThread->wait();
    }
}

/******************************************************************************
 ******************************************************************************/
static inline QString get_setting_key(int s)
{
    const char* name = lt::name_for_setting(s);
    if (name) {
        return QString::fromUtf8(name);
    }
    return {};
}

QString TorrentContext::upload_rate_limit()
{
    return get_setting_key(lt::settings_pack::upload_rate_limit);
}

QString TorrentContext::download_rate_limit()
{
    return get_setting_key(lt::settings_pack::download_rate_limit);
}

QString TorrentContext::connections_limit()
{
    return get_setting_key(lt::settings_pack::connections_limit);
}

QString TorrentContext::unchoke_slots_limit()
{
    return get_setting_key(lt::settings_pack::unchoke_slots_limit);
}

/******************************************************************************
 ******************************************************************************/
QString TorrentContext::version()
{
    return QString::fromUtf8(libtorrent::version());
}

QString TorrentContext::website()
{
    using namespace Qt::Literals::StringLiterals;
    return "libtorrent"_L1;
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::setNetworkManager(NetworkManager *networkManager)
{
    m_networkManager = networkManager;
}

/******************************************************************************
 ******************************************************************************/
Settings* TorrentContext::settings() const
{
    return m_settings;
}

void TorrentContext::setSettings(Settings *settings)
{
    if (m_settings) {
        disconnect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
    }
    m_settings = settings;
    if (m_settings) {
        connect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
        onSettingsChanged();
    }
}

void TorrentContext::onSettingsChanged()
{
    if (!m_settings) {
        return;
    }
    lt::settings_pack pack = lt::default_settings(); /* = fromSettings(settings)*/

    auto map = m_settings->torrentSettings();
    QMapIterator<QString, QVariant> it(map);
    while (it.hasNext()) {
        it.next();
        auto key = it.key();
        auto value = it.value();

        auto name = lt::setting_by_name(key.toUtf8().data());
        auto type = name & lt::settings_pack::type_mask;

        switch (type) {
        case lt::settings_pack::string_type_base:
            pack.set_str(name, value.toString().toStdString());
            break;
        case lt::settings_pack::int_type_base:
            pack.set_int(name, value.toInt());
            break;
        case lt::settings_pack::bool_type_base:
            pack.set_bool(name, value.toBool());
            break;
        default:
            break;
        }
    }
    m_workerThread->setSettings(pack);

    auto enabled = m_settings->isTorrentEnabled();
    m_workerThread->setEnabled(enabled);

    emit changed();
}

/******************************************************************************
 ******************************************************************************/
bool TorrentContext::isEnabled() const
{
    return m_settings && m_settings->isTorrentEnabled();
}

void TorrentContext::setEnabled(bool enabled)
{
    if (m_settings) {
        m_settings->setTorrentEnabled(enabled);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::prepareTorrent(Torrent *torrent)
{
    try {

        qDebug_1 << Q_FUNC_INFO;
        if (!torrent) {
            return;
        }
        auto info = torrent->info();
        info.state = TorrentInfo::downloading_metadata;
        torrent->setInfo(info, false);

        auto torrentFile = torrent->localFullFileName(); // destination

        if (QFileInfo::exists(torrentFile)) {
            readTorrentFile(torrentFile, torrent);
            return;
        }

        ensureDestinationPathExists(torrent);
        auto source = torrent->url();

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
    } catch (std::exception const& e) {
        qWarning() << "Caught exception in " << Q_FUNC_INFO << ": " << QString::fromUtf8(e.what());
    }
}

void TorrentContext::stopPrepare(Torrent *torrent)
{    
    try {        
        abortNetworkReply(torrent); // abort conventional download
        removeTorrent(torrent);     // also abort magnet metainfo download

    } catch (std::exception const& e) {
        qWarning() << "Caught exception in " << Q_FUNC_INFO << ": " << QString::fromUtf8(e.what());
    }
}

/******************************************************************************
 ******************************************************************************/
bool TorrentContext::hasTorrent(Torrent *torrent)
{
    try {
        qDebug_1 << Q_FUNC_INFO;
        auto handle = find(torrent);
        return handle.is_valid();
    } catch (std::exception const& e) {
        qWarning() << "Caught exception in " << Q_FUNC_INFO << ": " << QString::fromUtf8(e.what());
    }
    return false;
}

/******************************************************************************
 ******************************************************************************/
bool TorrentContext::addTorrent(Torrent *torrent)
{
    try {
        if (!_addTorrent(torrent)) {
            torrent->setError(
                TorrentError::FailedToAddError,
                tr("Bad .torrent format: Can't download it."));
            return false;
        }
        return true;
    } catch (std::exception const& e) {
        qWarning() << "Caught exception in " << Q_FUNC_INFO << ": " << QString::fromUtf8(e.what());
    }
    return false;
}

void TorrentContext::removeTorrent(Torrent *torrent)
{
    try {
        /// \todo rename method?

        qDebug_1 << Q_FUNC_INFO;
        auto handle = find(torrent);
        if (handle.is_valid()) {
            m_workerThread->removeTorrent(handle); // needs calling lt::session
            auto uuid = TorrentUtils::toUniqueId(handle.info_hash());
            m_hashMap.remove(uuid);
        }

    } catch (std::exception const& e) {
        qWarning() << "Caught exception in " << Q_FUNC_INFO << ": " << QString::fromUtf8(e.what());
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::resumeTorrent(Torrent *torrent)
{
    try {
        qDebug_1 << Q_FUNC_INFO;
        auto handle = find(torrent);
        if (handle.is_valid()) {
            handle.resume();
        }
    } catch (std::exception const& e) {
        qWarning() << "Caught exception in " << Q_FUNC_INFO << ": " << QString::fromUtf8(e.what());
    }
}

void TorrentContext::pauseTorrent(Torrent *torrent)
{
    try {
        qDebug_1 << Q_FUNC_INFO;
        auto handle = find(torrent);
        if (handle.is_valid()) {
            handle.pause();
        }
    } catch (std::exception const &e) {
        qWarning() << "Caught exception in " << Q_FUNC_INFO << ": " << QString::fromUtf8(e.what());
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::setPriority(Torrent *torrent, int index, TorrentFileInfo::Priority p)
{
    try {
        Q_ASSERT(torrent);
        torrent->setFilePriority(index, p);

        changeFilePriority(torrent, index, p);
    } catch (std::exception const& e) {
        qWarning() << "Caught exception in " << Q_FUNC_INFO << ": " << QString::fromUtf8(e.what());
    }
}

void TorrentContext::setPriorityByFileOrder(Torrent *torrent, const QList<int> &fileIndexes)
{
    Q_ASSERT(torrent);
    auto fileCount = torrent->fileCount();
    for (auto fileIndex : fileIndexes) {
        auto priority = TorrentContext::computePriority(fileIndex, fileCount);
        setPriority(torrent, fileIndex, priority);
    }
}

TorrentFileInfo::Priority TorrentContext::computePriority(int row, qsizetype count)
{
    if (count < 3) {
        return TorrentFileInfo::Normal;
    }
    auto pos = static_cast<qreal>(row + 1) / static_cast<qreal>(count);
    if (pos < ONE_THIRD) {
        return TorrentFileInfo::High;
    }
    if (pos < TWO_THIRDS) {
        return TorrentFileInfo::Normal;
    }
    return TorrentFileInfo::Low;
}

/******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Return every setting and its default value.
 */
QList<TorrentSettingItem> TorrentContext::allSettingsKeysAndValues() const
{
    return _toPreset( lt::default_settings() );
}

QList<TorrentSettingItem> TorrentContext::presetDefault() const
{
    return _toPreset( lt::default_settings() );
}

QList<TorrentSettingItem> TorrentContext::presetMinCache() const
{
    return _toPreset( lt::min_memory_usage() );
}

QList<TorrentSettingItem> TorrentContext::presetHighPerf() const
{
    return _toPreset( lt::high_performance_seed() );
}

/******************************************************************************
 ******************************************************************************/
QList<TorrentSettingItem> TorrentContext::_toPreset(const lt::settings_pack all) const
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
            &TorrentContext::_get_str
        },
        { lt::settings_pack::int_type_base,
            lt::settings_pack::max_int_setting_internal,
            &TorrentContext::_get_int
        },
        { lt::settings_pack::bool_type_base,
            lt::settings_pack::max_bool_setting_internal,
            &TorrentContext::_get_bool
        }
    };

    for (auto c : cs) {
        for (auto index = c.begin; index < c.end; ++index) {

            // Remove non-modifiable settings and settings managed somewhere else.
            switch (index) {
            case lt::settings_pack::user_agent:
            case lt::settings_pack::alert_mask:
                continue;
            default:
                break;
            }

            auto name = lt::name_for_setting(index);
            if (name == nullptr) {
                continue;
            }
            auto key = QString::fromUtf8(name);
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
QVariant TorrentContext::_get_str(const lt::settings_pack &pack, int index)
{
    return QString::fromStdString(pack.get_str(index));
}

QVariant TorrentContext::_get_int(const lt::settings_pack &pack, int index)
{
    return pack.get_int(index);
}

QVariant TorrentContext::_get_bool(const lt::settings_pack &pack, int index)
{
    return pack.get_bool(index);
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::onStopped()
{
    qDebug_1 << Q_FUNC_INFO; // Just to confirm it's stopped:)
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::onMetadataUpdated(TorrentData data)
{
    qDebug_1 << Q_FUNC_INFO;
    auto torrent = find(data.unique_id);
    if (torrent) {
        torrent->setDetail(data.detail, true);
        torrent->setMetaInfo(data.metaInfo); // setMetaInfo will emit the GUI update signal

        auto handle = m_workerThread->findTorrent(data.unique_id);
        if (handle.is_valid()) {
            auto ti = handle.torrent_file();

            try {
                /*
                 * Try to save the torrent file on disk.
                 *
                 * Note:
                 * Torrent files in BitTorrent v2 throw exception
                 * because Bencode cannot write a file with missing piece hashes.
                 */

                if (ti) {
                    auto torrentFile = torrent->localFullFileName();  // destination
                    ensureDestinationPathExists(torrent);

                    writeTorrentFileFromMagnet(torrentFile, ti);

                    /*
                     * The .torrent file is immediately loaded after being written,
                     * so that the program ensures the file is correctly written
                     * (compliant with the bittorrent format specification).
                     */
                    readTorrentFile(torrentFile, torrent);
                }
                /*
                 * `removeTorrent` aborts the torrent download,
                 * but contrary to the name, it keeps the file on disk.
                 *
                 * This is temporarly disabled.
                 */
                // removeTorrent(torrent);

            } catch (const std::exception &exception) {

                qWarning() << "Caught exception" << QString::fromUtf8(exception.what());

                /*
                 * Safe mode: .torrent file is not saved on disk.
                 */
                if (ti) {
                    auto metaInfo = torrent->metaInfo();

                    TorrentInitialMetaInfo initialMetaInfo = TorrentUtils::toTorrentInitialMetaInfo(ti);
                    metaInfo.initialMetaInfo = initialMetaInfo;

                    auto info = torrent->info();
                    info.state = TorrentInfo::stopped;
                    torrent->setInfo(info, true);
                    torrent->setMetaInfo(metaInfo); // setMetaInfo will emit the GUI update signal

                    resetPriorities(torrent);
                }
            }
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::onDataUpdated(TorrentData data)
{
    qDebug_1 << Q_FUNC_INFO;
    auto torrent = find(data.unique_id);
    if (torrent) {
        torrent->setDetail(data.detail, true);
        torrent->setMetaInfo(data.metaInfo); // setMetaInfo will emit the GUI update signal
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::onStatusUpdated(TorrentStatus status)
{
    qDebug_1 << Q_FUNC_INFO;
    auto torrent = find(status.unique_id);
    if (torrent) {
        torrent->setInfo(status.info, false);
        torrent->setDetail(status.detail, false);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::ensureDestinationPathExists(Torrent *torrent)
{
    auto path = torrent->localFullFileName();
    const QFileInfo fi(path);
    auto outputPath = fi.absolutePath();
    QDir().mkpath(outputPath);
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::downloadMagnetLink(Torrent *torrent)
{
    auto source = torrent->url();
    auto url = QUrl::fromUserInput(source);

    qDebug_1 << Q_FUNC_INFO << url;

    // For magnet link, libtorrent will download the torrent metadata before
    // the torrent itself
    _addTorrent(torrent);
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Download .torrent file with regular download engine
 */
void TorrentContext::downloadTorrentFile(Torrent *torrent)
{
    auto source = torrent->url();
    auto url = QUrl::fromUserInput(source);

    qDebug_1 << Q_FUNC_INFO << url;

    Q_ASSERT(m_networkManager);
    QNetworkReply *reply = m_networkManager->get(url);
    if (!reply) {
        auto m = torrent->metaInfo();
        m.error.type = TorrentError::MetadataDownloadError;
        m.error.message = tr("Network request rejected.");
        torrent->setMetaInfo(m);
        return;
    }
    connect(reply, SIGNAL(finished()),
            this, SLOT(onNetworkReplyFinished()),
            Qt::UniqueConnection);

    m_currentDownloads.insert(reply, torrent);
}

void TorrentContext::abortNetworkReply(Torrent *torrent)
{
    QHashIterator<QNetworkReply *, Torrent *> it(m_currentDownloads);
    while (it.hasNext()) {
        it.next();
        auto currentReply = it.key();
        auto currentTorrent = it.value();
        if (currentTorrent == torrent) {
            currentReply->abort();
            /*
             * Rem: Do not remove(reply*) at this point, onFinished() will do that
             */
        }
    }
}

void TorrentContext::onNetworkReplyFinished()
{
    qDebug_1 << Q_FUNC_INFO;

    auto reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    Torrent *torrent = nullptr;
    if (m_currentDownloads.contains(reply)) {
        torrent = m_currentDownloads.take(reply);
    }
    if (!torrent) {
        return;
    }

    auto url = reply->url();
    qDebug_1 << Q_FUNC_INFO << url;

    if (reply->error() != QNetworkReply::NoError) {
        auto m = torrent->metaInfo();
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
        auto m = torrent->metaInfo();
        m.error.type = TorrentError::MetadataDownloadError;
        m.error.message = tr("No metadata downloaded.");
        torrent->setMetaInfo(m);

        reply->deleteLater();
        return;
    }

    reply->deleteLater();

    auto torrentFile = torrent->localFullFileName(); // destination
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
void TorrentContext::archiveExistingFile(const QString &filename)
{
    if (QFileInfo::exists(filename)) {
        auto archive = QString("%0.0.old").arg(filename);
        int i = 0;
        while (QFileInfo::exists(archive)) {
            i++;
            archive = QString("%0.%1.old").arg(filename, QString::number(i));
        }
        if (!QFile::rename(filename, archive)) {
            qWarning() << "Couldn't rename file '" << filename << "' into '" << archive << "'.'";
        }
    }
}

void TorrentContext::writeTorrentFile(const QString &filename, QIODevice *data)
{
    archiveExistingFile(filename);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data->readAll());
        file.close();
    }
}

void TorrentContext::writeTorrentFileFromMagnet(
    const QString &filename,
    std::shared_ptr<lt::torrent_info const> ti)
{
    /*
     * BUGFIX Libtorrent v2.0.9
     * This method is temporarly deprecated
     * due to bug with generate() when writing v2-mode torrents to disk.
     */
    return;

    archiveExistingFile(filename);

    // Bittorrent Encoding
    lt::create_torrent ct(*ti);

    // BUG
    // Torrent in v2-mode are currently not supported,
    // because the mode doesn't generate piece hash.
    /// \todo add piece layers manually before generate() it
    auto entry = ct.generate(); // this method throws exception

    std::vector<char> buffer;
    lt::bencode(std::back_inserter(buffer), entry);

    // Write
    QByteArray data(&buffer[0], static_cast<int>(buffer.size()));
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::readTorrentFile(const QString &filename, Torrent *torrent)
{
    if (!torrent) {
        return;
    }
    if (!QFileInfo::exists(filename)) {
        return;
    }

    auto initialMetaInfo = m_workerThread->dump(filename);

    auto info = torrent->info();
    info.state = TorrentInfo::stopped;
    torrent->setInfo(info, true);

    auto metaInfo = torrent->metaInfo();
    metaInfo.initialMetaInfo = initialMetaInfo;
    torrent->setMetaInfo(metaInfo); // setMetaInfo will emit the GUI update signal

    resetPriorities(torrent);
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::resetPriorities(Torrent *torrent)
{
    if (!torrent) {
        return;
    }
    // Do a fake 'detail' update, to initialize the torrent.

    auto detail = torrent->detail();
    detail.files.clear();
    auto metaInfo = torrent->metaInfo();
    for (auto index = 0; index < metaInfo.initialMetaInfo.files.count(); ++index) {
        TorrentFileInfo fi;
        fi.priority = TorrentFileInfo::Normal;
        detail.files.append(fi);
    }
    torrent->setDetail(detail, false);
}


/******************************************************************************
 ******************************************************************************/
/*!
 * \brief return false on failure
 */
bool TorrentContext::_addTorrent(Torrent *torrent) // resumeTorrent
{
    auto info = torrent->info();
    info.state = TorrentInfo::checking_files;
    torrent->setInfo(info, false);

    auto source = torrent->url();
    auto torrentFile = torrent->localFullFileName(); // destination

    if (QFileInfo::exists(torrentFile)) {
        source = torrentFile;
    }

    qDebug_1 << Q_FUNC_INFO << source;

    ensureDestinationPathExists(torrent);
    auto outputPath = torrent->localFilePath();

    lt::add_torrent_params p;

    if (isMagnetSource(source)) { // Add from magnet link
        auto info = torrent->info();
        info.state = TorrentInfo::downloading_metadata;
        torrent->setInfo(info, false);

        auto bytes = source.toLatin1();
        // QByteArray bytes = source.toUtf8();

        auto ptr = bytes.constData();
        auto size = static_cast<boost::string_view::size_type>(bytes.size());

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
            auto torrent = source.toStdString();
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

            auto s = source.toLocal8Bit().data();
            lt::sha1_hash h1(s);
            lt::info_hash_t infohashes(h1);
            lt::add_torrent_params p;
            p.info_hashes = infohashes;
        }

        p.file_priorities.clear();
        for (auto fi = 0; fi < torrent->fileCount(); ++fi) {
            auto priority = TorrentUtils::fromPriority(torrent->filePriority(fi));
            p.file_priorities.push_back(priority);
        }
    }

    p.flags &= ~lt::torrent_flags::duplicate_is_error; // do not raise exception if duplicate

    p.save_path = outputPath.toStdString();

    // Blocking insertion
    lt::error_code ec2;
    auto handle = m_workerThread->addTorrent(std::move(p), ec2);
    if (ec2) {
        qDebug_1 << "failed to load torrent";
        qDebug_1 << QString::fromStdString(source.toStdString());
        qDebug_1 << QString::fromStdString(ec2.message());
        return false;
    }

    if (!handle.is_valid()) {
        return false;
    }

    handle.pause();

    auto uuid = TorrentUtils::toUniqueId(handle.info_hash());
    m_hashMap.insert(uuid, torrent);
    return true;
}

// /******************************************************************************
//  ******************************************************************************/
// void TorrentContext::moveQueueUp(Torrent *torrent)
// {
//     qDebug_1 << Q_FUNC_INFO;
//     auto handle = find(torrent);
//     if (handle.is_valid()) {
//         handle.queue_position_up();
//     }
// }
//
// void TorrentContext::moveQueueDown(Torrent *torrent)
// {
//     qDebug_1 << Q_FUNC_INFO;
//     auto handle = find(torrent);
//     if (handle.is_valid()) {
//         handle.queue_position_down();
//     }
// }
//
// void TorrentContext::moveQueueTop(Torrent *torrent)
// {
//     qDebug_1 << Q_FUNC_INFO;
//     auto handle = find(torrent);
//     if (handle.is_valid()) {
//         handle.queue_position_top();
//     }
// }
//
// void TorrentContext::moveQueueBottom(Torrent *torrent)
// {
//     qDebug_1 << Q_FUNC_INFO;
//     auto handle = find(torrent);
//     if (handle.is_valid()) {
//         handle.queue_position_bottom();
//     }
// }
//
/******************************************************************************
 ******************************************************************************/
void TorrentContext::changeFilePriority(
    Torrent *torrent,
    int index, TorrentFileInfo::Priority p)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        auto findex = static_cast<lt::file_index_t>(index);
        auto priority = TorrentUtils::fromPriority(p);
        handle.file_priority(findex, priority);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::addSeed(Torrent *torrent, const TorrentWebSeedMetaInfo &seed)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        if (seed.type == TorrentWebSeedMetaInfo::Type::UrlSeed) {
            handle.add_url_seed(seed.url.toStdString());
        } else {
            handle.add_http_seed(seed.url.toStdString());
        }
    }
}

void TorrentContext::removeSeed(Torrent *torrent, const TorrentWebSeedMetaInfo &seed)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        if (seed.type == TorrentWebSeedMetaInfo::Type::UrlSeed) {
            handle.remove_url_seed(seed.url.toStdString());
        } else {
            handle.remove_http_seed(seed.url.toStdString());
        }
    }
}

void TorrentContext::removeAllSeeds(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        std::set<std::string>::iterator i, end;

        auto url_seeds = handle.url_seeds();
        for (i = url_seeds.begin(), end = (url_seeds.end()); i != end; ++i) {
            handle.remove_url_seed(*i);
        }

        auto http_seeds = handle.http_seeds();
        for (i = http_seeds.begin(), end = (http_seeds.end()); i != end; ++i) {
            handle.remove_http_seed(*i);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::addPeer(Torrent *torrent, const TorrentPeerInfo &peer)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        auto endpoint = TorrentUtils::fromEndPoint(peer.endpoint);
        handle.connect_peer(endpoint);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::addTracker(Torrent *torrent, const TorrentTrackerInfo &tracker)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        auto entry = TorrentUtils::fromTorrentTrackerInfo(tracker);
        handle.add_tracker(entry);
    }
}

void TorrentContext::removeTracker(Torrent *torrent, const TorrentTrackerInfo &tracker)
{
    qDebug_1 << Q_FUNC_INFO;
    Q_UNUSED(torrent)
    Q_UNUSED(tracker)
    /// \todo
}


/******************************************************************************
 ******************************************************************************/
void TorrentContext::forceRecheck(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        handle.force_recheck();
    }
}

void TorrentContext::forceReannounce(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        handle.force_reannounce();
    }
}

void TorrentContext::forceDHTReannounce(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        handle.force_dht_announce();
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::setSSLCertificatePath(Torrent *torrent, const QString &path)
{
    qDebug_1 << Q_FUNC_INFO;
    Q_UNUSED(torrent)
    Q_UNUSED(path)
    // todo
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::scrapeTracker(Torrent *torrent, int index)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        handle.scrape_tracker(index);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::setUploadBandwidth(Torrent *torrent, int limit)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        handle.set_upload_limit(limit);
    }
}

void TorrentContext::setDownloadBandwidth(Torrent *torrent, int limit)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        handle.set_download_limit(limit);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::setMaxUploads(Torrent *torrent, int limit)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        handle.set_max_uploads(limit);
    }
}

void TorrentContext::setMaxConnections(Torrent *torrent, int limit)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        handle.set_max_connections(limit);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::renameFile(Torrent *torrent, int index, const QString &newName)
{
    qDebug_1 << Q_FUNC_INFO;
    auto handle = find(torrent);
    if (handle.is_valid()) {
        auto findex = static_cast<lt::file_index_t>(index);
        handle.rename_file(findex, newName.toStdString());
    }
}

/******************************************************************************
 ******************************************************************************/
inline Torrent* TorrentContext::find(const UniqueId &uuid)
{
    qDebug_1 << Q_FUNC_INFO;
    return m_hashMap.value(uuid, nullptr);
}

inline lt::torrent_handle TorrentContext::find(Torrent *torrent)
{
    qDebug_1 << Q_FUNC_INFO;
    auto info_hash = m_hashMap.key(torrent, UniqueId());
    return m_workerThread->findTorrent(info_hash);
    //    return hashMap.key(item, lt::torrent_handle());
}
