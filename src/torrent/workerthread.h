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

#ifndef TORRENT_WORKER_THREAD_H
#define TORRENT_WORKER_THREAD_H

#include <Torrent/TorrentMessage>

#include <QtCore/QThread>

#include "libtorrent/fwd.hpp"
#include "libtorrent/error_code.hpp"    // lt::error_code
#include "libtorrent/session_types.hpp" // lt::remove_flags_t


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
    
    inline void log(lt::alert *s);
};

#endif // TORRENT_WORKER_THREAD_H
