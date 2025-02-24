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

class AbstractJob;
class QueueModel;
class ResourceItem;
class Settings;
class Snapshot;

class QTimer;
class NetworkManager;
class QNetworkReply;

using DownloadRange = QList<AbstractJob *>;

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
    void append(const QList<AbstractJob *> &jobs, bool started = false);
    qsizetype count() const;

    // Run
    void resume(AbstractJob *job);
    void pause(AbstractJob *job);
    void cancel(AbstractJob *job);

    // Statistics
    QList<AbstractJob *> jobs() const;
    QList<AbstractJob *> completedJobs() const;
    QList<AbstractJob *> failedJobs() const;
    QList<AbstractJob *> runningJobs() const;
    qreal totalSpeed();

    // Settings
    int maxSimultaneousDownloads() const;
    void setMaxSimultaneousDownloads(int number);

    //-------------
    // Queue Management
    NetworkManager* networkManager() const; // move somewhere else

    // Utility
    AbstractJob* createJobFile(const QUrl &url);
    AbstractJob* createJobTorrent(const QUrl &url);

    QAbstractItemModel *model() const;

signals:
    void jobFinished(AbstractJob *job);
    void jobRenamed(QString oldName, QString newName, bool success);

public slots:
    void activateSnapshot();

private slots:
    void onJobChanged();
    void onJobFinished();
    void onJobRenamed(const QString &oldName, const QString &newName, bool success);
    void startNext(AbstractJob *job);

    void onSettingsChanged();

    void onSpeedTimerTimeout();

private:
    QueueModel *m_queueModel = nullptr;
    Snapshot *m_snapshot = nullptr;

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
