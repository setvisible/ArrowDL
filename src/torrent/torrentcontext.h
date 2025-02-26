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

#include <Torrent/TorrentBaseContext>

#include <Torrent/SettingItem>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariant>

class NetworkManager;
class Settings;
class Torrent;
class TorrentContextPrivate;


/*!
 * @class TorrentContext
 * @brief Represents the libtorrent context.
 */
class TorrentContext : public QObject, public TorrentBaseContext
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
    ~TorrentContext() override = default;
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

signals:
    void changed();

public slots:

private:
    TorrentContextPrivate *d = nullptr;
    friend class TorrentContextPrivate;
};

#endif // TORRENT_CONTEXT_H
