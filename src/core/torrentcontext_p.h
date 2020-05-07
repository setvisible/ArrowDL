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

#ifndef CORE_TORRENT_CONTEXT_P_H
#define CORE_TORRENT_CONTEXT_P_H

#include "torrentcontext.h"

#include <Core/Torrent>

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QMap>
#include <QtNetwork/QNetworkAccessManager>

#include <vector> // std::vector
#include <ctime>  // std::time_t, definition required by MSVC 2017

#include "libtorrent/fwd.hpp"
#include "libtorrent/error_code.hpp"    // lt::error_code
#include "libtorrent/session_types.hpp" // lt::remove_flags_t
#include "libtorrent/string_view.hpp"   // lt:string_view


class DownloadTorrentItem;
class Settings;
class WorkerThread;

class TorrentContextPrivate : public QObject
{  
    Q_OBJECT

public:
    explicit TorrentContextPrivate(TorrentContext *qq);
    ~TorrentContextPrivate() Q_DECL_OVERRIDE;

    QList<TorrentSettingItem> allSettingsKeysAndValues() const;
    QList<TorrentSettingItem> presetDefault() const;
    QList<TorrentSettingItem> presetMinCache() const;
    QList<TorrentSettingItem> presetHighPerf() const;

    void prepareTorrent(DownloadTorrentItem *item);
    void stopPrepare(DownloadTorrentItem *item);

    bool hasTorrent(DownloadTorrentItem *item);

    bool addTorrent(DownloadTorrentItem *item); // return false on failure
    void removeTorrent(DownloadTorrentItem *item);

    void resumeTorrent(DownloadTorrentItem *item);
    void pauseTorrent(DownloadTorrentItem *item);

    void moveQueueUp(DownloadTorrentItem *item);
    void moveQueueDown(DownloadTorrentItem *item);
    void moveQueueTop(DownloadTorrentItem *item);
    void moveQueueBottom(DownloadTorrentItem *item);

    void changeFilePriority(DownloadTorrentItem *item, int index, TorrentFileInfo::Priority p);

    void addSeed(DownloadTorrentItem *item, const TorrentWebSeedMetaInfo &seed);
    void removeSeed(DownloadTorrentItem *item, const TorrentWebSeedMetaInfo &seed);
    void removeAllSeeds(DownloadTorrentItem *item);

    void addPeer(DownloadTorrentItem *item, const TorrentPeerMetaInfo &peer);

    void addTracker(DownloadTorrentItem *item, const TorrentTrackerInfo &tracker);
    void removeTracker(DownloadTorrentItem *item, const TorrentTrackerInfo &tracker);

    void forceRecheck(DownloadTorrentItem *item);
    void forceReannounce(DownloadTorrentItem *item);
    void forceDHTReannounce(DownloadTorrentItem *item);
    void setSSLCertificatePath(DownloadTorrentItem *item, const QString &path);
    void scrapeTracker(DownloadTorrentItem *item, int index = -1);

    void setUploadBandwidth(DownloadTorrentItem *item, int limit);
    void setDownloadBandwidth(DownloadTorrentItem *item, int limit);

    void setMaxUploads(DownloadTorrentItem *item, int limit);
    void setMaxConnections(DownloadTorrentItem *item, int limit);

    void renameFile(DownloadTorrentItem *item, int index, const QString &newName);

public slots:
    void onSettingsChanged();

    void onStopped();
    void onDataUpdated(TorrentData data);
    void onStatusUpdated(TorrentStatus status);

public:
    TorrentContext *q;
    WorkerThread *workerThread;
    Settings *settings;
    QHash<UniqueId, DownloadTorrentItem*> hashMap;

    inline DownloadTorrentItem* find(const UniqueId &uuid);
    inline lt::torrent_handle find(DownloadTorrentItem *item);


private slots:
    void onNetworkReplyFinished();
private:
    QNetworkAccessManager m_networkManager;
    QMap<QNetworkReply*, DownloadTorrentItem*> m_currentDownloads;
    void downloadMagnetLink(DownloadTorrentItem *item);
    void downloadTorrentFile(DownloadTorrentItem *item);
    void abortNetworkReply(DownloadTorrentItem *item);

    void writeTorrentFile(const QString &filename, QIODevice *data);
    void readTorrentFile(const QString &filename, DownloadTorrentItem *item);

    QList<TorrentSettingItem> _toPreset(const lt::settings_pack all) const;
    static QVariant _get_str(const lt::settings_pack &pack, const int index);
    static QVariant _get_int(const lt::settings_pack &pack, const int index);
    static QVariant _get_bool(const lt::settings_pack &pack, const int index);
};

class WorkerThread : public QThread
{
    Q_OBJECT

public:
    WorkerThread(QObject *parent = nullptr);

    void run() Q_DECL_OVERRIDE;
    void stop();

    lt::settings_pack settings() const;
    void setSettings(lt::settings_pack &pack);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    lt::torrent_handle addTorrent(lt::add_torrent_params const& params, lt::error_code& ec);
    void removeTorrent(const lt::torrent_handle& h, lt::remove_flags_t options = {});

    lt::torrent_handle findTorrent(const UniqueId &uuid) const;

    TorrentInitialMetaInfo dump(const QString &filename) const;

signals:
    void dataUpdated(TorrentData data);
    void statusUpdated(TorrentStatus status);

    void resumeDataSaved();
    void resumeDataSaveFailed();

    void stopped();

private:
    bool shouldQuit = false;
    lt::session *m_session_ptr = nullptr;

    void signalizeAlert(lt::alert* alert);


    inline void onTorrentAdded(const lt::torrent_handle &handle,
                               const lt::add_torrent_params &params,
                               const lt::error_code &error);
    inline void onMetadataReceived(const lt::torrent_handle &handle);
    inline void onStateUpdated(const std::vector<lt::torrent_status> &status);


    inline void signalizeDataUpdated(const lt::torrent_handle &handle,
                                     const lt::add_torrent_params &params);
    inline void signalizeStatusUpdated(const lt::torrent_status &status);


    inline TorrentInitialMetaInfo toTorrentInitialMetaInfo(std::shared_ptr<lt::torrent_info const> ti) const;
    inline TorrentMetaInfo toTorrentMetaInfo(const lt::add_torrent_params &params) const;
    inline TorrentHandleInfo toTorrentHandleInfo(const lt::torrent_handle &handle) const;


    inline QString toString(const std::string &str) const;
    inline QString toString(const lt::string_view &s) const;

    inline QString toString(const lt::sha1_hash &hash) const;

    inline QDateTime toDateTime(const std::time_t &time) const;

    inline std::string userAgent();

    inline void log(lt::alert *s);
};

#endif // CORE_TORRENT_CONTEXT_P_H
