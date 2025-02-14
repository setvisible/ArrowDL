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

#ifndef CORE_DOWNLOAD_MANAGER_H
#define CORE_DOWNLOAD_MANAGER_H

#include <Core/DownloadEngine>

#include <QtCore/QList>
#include <QtCore/QString>

class AbstractDownloadItem;
class ResourceItem;
class Settings;

class QTimer;
class NetworkManager;
class QNetworkReply;

class DownloadManager : public DownloadEngine
{
    Q_OBJECT

public:
    explicit DownloadManager(QObject *parent);
    ~DownloadManager() override;

    /* Settings */
    Settings* settings() const;
    void setSettings(Settings *settings);

    /* Queue Management */
    NetworkManager* networkManager() const;

    /* Utility */
    AbstractDownloadItem* createItem(const QUrl &url) override;
    AbstractDownloadItem* createTorrentItem(const QUrl &url) override;

private slots:
    void onSettingsChanged();

    void onQueueChanged(const DownloadRange &range);
    void onQueueChanged(AbstractDownloadItem* item);
    void onQueueChanged();

    void loadQueue();
    void saveQueue();

private:
    /* Network parameters (SSL, Proxy, UserAgent...) */
    NetworkManager *m_networkManager = nullptr;
    Settings *m_settings = nullptr;

    /* Crash Recovery */
    QTimer* m_dirtyQueueTimer = nullptr;
    QString m_queueFile = {};

    inline ResourceItem* createResourceItem(const QUrl &url);
};

#endif // CORE_DOWNLOAD_MANAGER_H
