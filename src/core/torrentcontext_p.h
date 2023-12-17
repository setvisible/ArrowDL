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

#ifndef CORE_TORRENT_CONTEXT_P_H
#define CORE_TORRENT_CONTEXT_P_H

#include "torrentcontext.h"

#include <Core/TorrentMessage>

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QMap>

#include <vector> // std::vector
#include <ctime>  // std::time_t, definition required by MSVC 2017

#include "libtorrent/fwd.hpp"
#include "libtorrent/bitfield.hpp"      // lt::typed_bitfield
#include "libtorrent/error_code.hpp"    // lt::error_code
#include "libtorrent/session_types.hpp" // lt::remove_flags_t
#include "libtorrent/string_view.hpp"   // lt:string_view
#include "libtorrent/sha1_hash.hpp"     // lt::sha1_hash

class NetworkManager;
class Settings;
class Torrent;
class WorkerThread;

class QIODevice;
class QNetworkReply;

class TorrentContextPrivate : public QObject
{  
    Q_OBJECT

public:
    explicit TorrentContextPrivate(TorrentContext *qq = nullptr);
    ~TorrentContextPrivate() override;

    QList<TorrentSettingItem> allSettingsKeysAndValues() const;
    QList<TorrentSettingItem> presetDefault() const;
    QList<TorrentSettingItem> presetMinCache() const;
    QList<TorrentSettingItem> presetHighPerf() const;

    void prepareTorrent(Torrent *torrent);
    void stopPrepare(Torrent *torrent);

    bool hasTorrent(Torrent *item);

    bool addTorrent(Torrent *torrent); // return false on failure
    void removeTorrent(Torrent *torrent);

    void resumeTorrent(Torrent *item);
    void pauseTorrent(Torrent *torrent);

    void moveQueueUp(Torrent *torrent);
    void moveQueueDown(Torrent *torrent);
    void moveQueueTop(Torrent *torrent);
    void moveQueueBottom(Torrent *torrent);

    void changeFilePriority(Torrent *torrent, int index, TorrentFileInfo::Priority p);

    void addSeed(Torrent *torrent, const TorrentWebSeedMetaInfo &seed);
    void removeSeed(Torrent *torrent, const TorrentWebSeedMetaInfo &seed);
    void removeAllSeeds(Torrent *torrent);

    void addPeer(Torrent *torrent, const TorrentPeerInfo &peer);

    void addTracker(Torrent *torrent, const TorrentTrackerInfo &tracker);
    void removeTracker(Torrent *torrent, const TorrentTrackerInfo &tracker);

    void forceRecheck(Torrent *torrent);
    void forceReannounce(Torrent *torrent);
    void forceDHTReannounce(Torrent *torrent);
    void setSSLCertificatePath(Torrent *torrent, const QString &path);
    void scrapeTracker(Torrent *torrent, int index = -1);

    void setUploadBandwidth(Torrent *torrent, int limit);
    void setDownloadBandwidth(Torrent *torrent, int limit);

    void setMaxUploads(Torrent *torrent, int limit);
    void setMaxConnections(Torrent *torrent, int limit);

    void renameFile(Torrent *torrent, int index, const QString &newName);

public slots:
    void onSettingsChanged();

    void onStopped();
    void onMetadataUpdated(TorrentData data);
    void onDataUpdated(TorrentData data);
    void onStatusUpdated(TorrentStatus status);

public:
    TorrentContext *q = nullptr;
    WorkerThread *workerThread = nullptr;
    Settings *settings = nullptr;
    NetworkManager *networkManager = nullptr;
    QHash<UniqueId, Torrent*> hashMap;

    inline Torrent *find(const UniqueId &uuid);
    inline lt::torrent_handle find(Torrent *torrent);

private slots:
    void onNetworkReplyFinished();

private:
    QHash<QNetworkReply *, Torrent *> m_currentDownloads;
    void downloadMagnetLink(Torrent *torrent);
    void downloadTorrentFile(Torrent *torrent);
    void abortNetworkReply(Torrent *torrent);

    void archiveExistingFile(const QString &filename);
    void writeTorrentFile(const QString &filename, QIODevice *data);    
    void writeTorrentFileFromMagnet(
            const QString &filename, std::shared_ptr<lt::torrent_info const> ti);
    void readTorrentFile(const QString &filename, Torrent *torrent);

    QList<TorrentSettingItem> _toPreset(const lt::settings_pack all) const;
    static QVariant _get_str(const lt::settings_pack &pack, int index);
    static QVariant _get_int(const lt::settings_pack &pack, int index);
    static QVariant _get_bool(const lt::settings_pack &pack, int index);

    void ensureDestinationPathExists(Torrent *torrent);
};

class WorkerThread : public QThread
{
    Q_OBJECT

public:
    WorkerThread(QObject *parent = nullptr);

    void run() override;
    void stop();

    lt::settings_pack settings() const;
    void setSettings(lt::settings_pack &pack);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    lt::torrent_handle addTorrent(lt::add_torrent_params const& params, lt::error_code& ec);
    void removeTorrent(const lt::torrent_handle& h, lt::remove_flags_t options = {});

    lt::torrent_handle findTorrent(const UniqueId &uuid) const;

    TorrentInitialMetaInfo dump(const QString &filename) const;

    static UniqueId toUniqueId(const lt::sha1_hash &hash);
    static lt::sha1_hash fromUniqueId(const UniqueId &uuid);

signals:
    void metadataUpdated(TorrentData data);
    void dataUpdated(TorrentData data);
    void statusUpdated(TorrentStatus status);

    void resumeDataSaved();
    void resumeDataSaveFailed();

    void stopped();

private:
    bool shouldQuit = false;
    lt::session *m_session_ptr = nullptr;

    void signalizeAlert(lt::alert* alert);

    inline void onTorrentAdded(const lt::torrent_handle &handle, const lt::add_torrent_params &params, const lt::error_code &error);
    inline void onMetadataReceived(const lt::torrent_handle &handle);
    inline void onStateUpdated(const std::vector<lt::torrent_status> &status);

    inline void signalizeDataUpdated(const lt::torrent_handle &handle, const lt::add_torrent_params &params);
    inline void signalizeStatusUpdated(const lt::torrent_status &status);

    inline TorrentInitialMetaInfo toTorrentInitialMetaInfo(std::shared_ptr<lt::torrent_info const> ti) const;
    inline TorrentMetaInfo toTorrentMetaInfo(const lt::add_torrent_params &params) const;
    inline TorrentHandleInfo toTorrentHandleInfo(const lt::torrent_handle &handle) const;

    inline QString toString(const std::string &str) const;
    inline QString toString(const lt::string_view &s) const;
    inline QString toString(const lt::sha1_hash &hash) const;
    inline QDateTime toDateTime(const std::time_t &time) const;

    inline void log(lt::alert *s);

protected:
    QBitArray toBitArray(const lt::typed_bitfield<lt::piece_index_t> &pieces) const;
    QBitArray toBitArray(const std::map<lt::piece_index_t, lt::bitfield> &map) const;

};

#endif // CORE_TORRENT_CONTEXT_P_H
