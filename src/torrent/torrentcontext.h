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

#ifndef TORRENT_CONTEXT_H
#define TORRENT_CONTEXT_H

#include <Torrent/ITorrentContext>
#include <Torrent/SettingItem>

#include <QtCore/QObject>

#include "libtorrent/fwd.hpp"

class NetworkManager;
class Settings;
class Torrent;
class WorkerThread;

class QNetworkReply;

/*!
 * @class TorrentContext
 * @brief Represents the libtorrent context.
 */
class TorrentContext : public QObject, public ITorrentContext
{
    Q_OBJECT
    // Note:
    // Scott Meyers mentions in his Effective Modern
    // C++ book, that deleted functions should generally
    // be public as it results in better error messages
    // due to the compilers behavior to check accessibility
    // before deleted status
private:
    TorrentContext();
    ~TorrentContext();
public:
    TorrentContext(TorrentContext const&) = delete; // Don't Implement
    void operator=(TorrentContext const&) = delete; // Don't implement

    static TorrentContext& getInstance();

    static QString upload_rate_limit();
    static QString download_rate_limit();
    static QString connections_limit();
    static QString unchoke_slots_limit();

    static QString version();
    static QString website();

    void setNetworkManager(NetworkManager *networkManager);

    /* Settings */
    Settings* settings() const;
    void setSettings(Settings *settings);

    QList<TorrentSettingItem> allSettingsKeysAndValues() const;
    QList<TorrentSettingItem> presetDefault() const;
    QList<TorrentSettingItem> presetMinCache() const;
    QList<TorrentSettingItem> presetHighPerf() const;

    /* Session */
    bool isEnabled() const;
    void setEnabled(bool enabled);

    /* Torrents */
    void prepareTorrent(Torrent *torrent);
    void stopPrepare(Torrent *torrent);

    bool hasTorrent(Torrent *torrent);
    bool addTorrent(Torrent *torrent);
    void removeTorrent(Torrent *torrent);

    void resumeTorrent(Torrent *torrent);
    void pauseTorrent(Torrent *torrent);

    void setPriority(Torrent *torrent, int index, TorrentFileInfo::Priority p) override;
    void setPriorityByFileOrder(Torrent *torrent, const QList<int> &rows) override;

    static TorrentFileInfo::Priority computePriority(int row, qsizetype count);

signals:
    void changed();

private slots:
    void onSettingsChanged();

    void onNetworkReplyFinished();
    void onStopped();
    void onMetadataUpdated(TorrentData data);
    void onDataUpdated(TorrentData data);
    void onStatusUpdated(TorrentStatus status);

private:
    WorkerThread *m_workerThread = nullptr;
    Settings *m_settings = nullptr;
    NetworkManager *m_networkManager = nullptr;
    QHash<UniqueId, Torrent*> m_hashMap = {};

    QHash<QNetworkReply *, Torrent *> m_currentDownloads = {};

    void downloadMagnetLink(Torrent *torrent);
    void downloadTorrentFile(Torrent *torrent);
    void abortNetworkReply(Torrent *torrent);

    void archiveExistingFile(const QString &filename);
    void writeTorrentFile(const QString &filename, QIODevice *data);
    void writeTorrentFileFromMagnet(const QString &filename, std::shared_ptr<lt::torrent_info const> ti);
    void readTorrentFile(const QString &filename, Torrent *torrent);

    void resetPriorities(Torrent *torrent);

    QList<TorrentSettingItem> _toPreset(const lt::settings_pack all) const;
    static QVariant _get_str(const lt::settings_pack &pack, int index);
    static QVariant _get_int(const lt::settings_pack &pack, int index);
    static QVariant _get_bool(const lt::settings_pack &pack, int index);

    void ensureDestinationPathExists(Torrent *torrent);

    bool _addTorrent(Torrent *torrent); // return false on failure

    // void moveQueueUp(Torrent *torrent);
    // void moveQueueDown(Torrent *torrent);
    // void moveQueueTop(Torrent *torrent);
    // void moveQueueBottom(Torrent *torrent);

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

    inline Torrent *find(const UniqueId &uuid);
    inline lt::torrent_handle find(Torrent *torrent);
};

#endif // TORRENT_CONTEXT_H
