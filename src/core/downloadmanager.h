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

#include <Core/IDownloadManager>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QAbstractTableModel>

class AbstractDownloadItem;
class QueueModel;
class ResourceItem;
class Settings;
class Snapshot;

class QTimer;
class NetworkManager;
class QNetworkReply;

using DownloadRange = QList<AbstractDownloadItem *>;

class DownloadManager : public QObject, public IDownloadManager
{
    Q_OBJECT

public:
    explicit DownloadManager(QObject *parent = nullptr);
    ~DownloadManager();

    // Settings
    Settings* settings() const;
    void setSettings(Settings *settings);

    // Queue
    void clear();
    void append(const QList<AbstractDownloadItem *> &items, bool started = false);
    qsizetype count() const;

    // Run
    void resume(AbstractDownloadItem *item);
    void pause(AbstractDownloadItem *item);
    void cancel(AbstractDownloadItem *item);

    // Statistics
    QList<AbstractDownloadItem *> downloadItems() const;
    QList<AbstractDownloadItem *> completedJobs() const;
    QList<AbstractDownloadItem *> failedJobs() const;
    QList<AbstractDownloadItem *> runningJobs() const;
    qreal totalSpeed();

    // Settings
    int maxSimultaneousDownloads() const;
    void setMaxSimultaneousDownloads(int number);

    //-------------
    // Queue Management
    NetworkManager* networkManager() const; // move somewhere else

    // Utility
    AbstractDownloadItem* createFileItem(const QUrl &url);
    AbstractDownloadItem* createTorrentItem(const QUrl &url);

    QAbstractItemModel *model() const;

signals:
    void jobFinished(AbstractDownloadItem *item);
    void jobRenamed(QString oldName, QString newName, bool success);

public slots:
    void activateSnapshot();

private slots:
    void onItemChanged();
    void onItemFinished();
    void onItemRenamed(const QString &oldName, const QString &newName, bool success);
    void startNext(AbstractDownloadItem *item);

    void onSettingsChanged();

    void onSpeedTimerTimeout();

private:
    QueueModel *m_queueModel = nullptr;
    Snapshot *m_snapshot = nullptr; // Crash Recovery

    // Network parameters (SSL, Proxy, UserAgent...)
    NetworkManager *m_networkManager = nullptr;
    Settings *m_settings = nullptr;

    qreal m_previouSpeed = 0;
    QTimer* m_speedTimer = nullptr;

    // Pool
    int m_maxSimultaneousDownloads = 4;
    qsizetype downloadingCount() const;

    inline ResourceItem* createResourceItem(const QUrl &url);
};

#endif // CORE_DOWNLOAD_MANAGER_H
