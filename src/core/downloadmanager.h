/* - DownZemAll! - Copyright (C) 2019 Sebastien Vavassori
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

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtNetwork/QNetworkReply>

class DownloadItem;
class ResourceItem;
class Settings;

class QTimer;
class QNetworkAccessManager;

class DownloadManager : public QObject
{
    Q_OBJECT

public:
    explicit DownloadManager(QObject *parent);
    ~DownloadManager();

    /* Settings */
    Settings* settings() const;
    void setSettings(Settings *settings);

    /* Queue Management */
    int count() const;

    void clear();

    QNetworkAccessManager *networkManager();

    void append(DownloadItem *downloadItem, const bool started = false);
    void append(ResourceItem *item, const bool started = false);
    void append(const QList<ResourceItem *> &downloadItems, const bool started = false);

    void remove(DownloadItem *item);
    void remove(const QList<DownloadItem*> &downloadItems);

    const DownloadItem* clientForRow(int row) const;

    /* Statistics */
    QList<DownloadItem*> downloadItems() const;
    QList<DownloadItem*> waitingJobs() const;
    QList<DownloadItem*> completedJobs() const;
    QList<DownloadItem*> pausedJobs() const;
    QList<DownloadItem*> failedJobs() const;
    QList<DownloadItem*> runningJobs() const;

    QString totalSpeed() const;

    /* Actions */
    void resume(DownloadItem *item);
    void pause(DownloadItem *item);
    void cancel(DownloadItem *item);

    /* Selection */
    void clearSelection();
    QList<DownloadItem*> selection() const;
    void setSelection(const QList<DownloadItem *> &selection);

    bool isSelected(DownloadItem *item) const;
    void setSelected(DownloadItem *item, bool isSelected);

    QString selectionToString() const;

signals:
    void jobAppended(DownloadItem *item);
    void jobRemoved(DownloadItem *item);
    void jobStateChanged(DownloadItem *item);

    void selectionChanged();

public slots:

private slots:
    void onSettingsChanged();
    void onQueueChanged(DownloadItem *item);
    void loadQueue();
    void saveQueue();

private:
    QNetworkAccessManager *m_networkManager;
    QList<DownloadItem*> m_items;

    // Pool
    int m_maxSimultaneousDownloads;
    int downloadingCount() const;
    void startNext();

    QList<DownloadItem*> m_selectedItems;
    Settings *m_settings;

    // Crash Recovery
    QTimer* m_dirtyQueueTimer;
    QString m_queueFile;
};

#endif // CORE_DOWNLOAD_MANAGER_H
