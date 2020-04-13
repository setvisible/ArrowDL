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

#ifndef CORE_DOWNLOAD_MANAGER_H
#define CORE_DOWNLOAD_MANAGER_H

#include <Core/DownloadEngine>

#include <QtCore/QList>
#include <QtCore/QString>

class ResourceItem;
class Settings;

class QTimer;
class QNetworkAccessManager;

class DownloadManager : public DownloadEngine
{
    Q_OBJECT

public:
    explicit DownloadManager(QObject *parent);
    ~DownloadManager() Q_DECL_OVERRIDE;

    /* Settings */
    Settings* settings() const;
    void setSettings(Settings *settings);

    /* Queue Management */
    QNetworkAccessManager *networkManager();

    /* Utility */
    IDownloadItem* createItem(const QUrl &url) Q_DECL_OVERRIDE;
    IDownloadItem* createTorrentItem(const QUrl &url, const QByteArray &data) Q_DECL_OVERRIDE;

private slots:
    void onSettingsChanged();

    void onQueueChanged(DownloadRange range);
    void onQueueChanged(IDownloadItem* item);
    void onQueueChanged();

    void loadQueue();
    void saveQueue();

private:
    QNetworkAccessManager *m_networkManager;
    Settings *m_settings;

    // Crash Recovery
    QTimer* m_dirtyQueueTimer;
    QString m_queueFile;

    inline ResourceItem* createResourceItem(const QUrl &url);
};

#endif // CORE_DOWNLOAD_MANAGER_H
