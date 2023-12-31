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

#ifndef CORE_DOWNLOAD_TORRENT_ITEM_H
#define CORE_DOWNLOAD_TORRENT_ITEM_H

#include <Core/DownloadItem>
#include <Core/Torrent>

#include <QtCore/QObject>
#include <QtCore/QString>


class DownloadManager;

class DownloadTorrentItem : public DownloadItem
{
    Q_OBJECT

public:
    DownloadTorrentItem(DownloadManager *downloadManager);
    ~DownloadTorrentItem() override = default;

    void setResource(ResourceItem *resource) override;

    void resume() override;
    void pause() override;
    void stop() override;

    void rename(const QString &newName) override;

    Torrent* torrent() const;

private slots:
    void onTorrentChanged();

private:
    Torrent *m_torrent = nullptr;

    bool isPreparing() const;
    bool isSeeding() const;
};

#endif // CORE_DOWNLOAD_TORRENT_ITEM_H
