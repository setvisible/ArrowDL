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

class AbstractDownloadItem;
class ResourceItem;
class Settings;

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

    // Queue Management
    NetworkManager* networkManager() const;

    qsizetype count() const;
    void clear();

    // virtual void append(const QList<AbstractDownloadItem *> &items, bool started = false);
    // virtual void remove(const QList<AbstractDownloadItem *> &items);
    void append(const QList<AbstractDownloadItem *> &items, bool started = false);
    void remove(const QList<AbstractDownloadItem *> &items);

    void removeItems(const QList<AbstractDownloadItem *> &items);
    void updateItems(const QList<AbstractDownloadItem *> &items);

    const AbstractDownloadItem* clientForRow(qsizetype row) const;

    int maxSimultaneousDownloads() const;
    void setMaxSimultaneousDownloads(int number);

    // Statistics
    QList<AbstractDownloadItem *> downloadItems() const;
    QList<AbstractDownloadItem *> completedJobs() const;
    QList<AbstractDownloadItem *> failedJobs() const;
    QList<AbstractDownloadItem *> runningJobs() const;

    qreal totalSpeed();

    // Actions
    void resume(AbstractDownloadItem *item);
    void pause(AbstractDownloadItem *item);
    void cancel(AbstractDownloadItem *item);

    // Selection
    void clearSelection();
    QList<AbstractDownloadItem *> selection() const;
    void setSelection(const QList<AbstractDownloadItem *> &selection);

    bool isSelected(AbstractDownloadItem *item) const;
    void setSelected(AbstractDownloadItem *item, bool isSelected);

    QString selectionToString() const;
    QString selectionToClipboard() const;

    void beginSelectionChange(); // BUGFIX
    void endSelectionChange();

    void moveCurrentTop();
    void moveCurrentUp();
    void moveCurrentDown();
    void moveCurrentBottom();

    // Settings
    Settings* settings() const;
    void setSettings(Settings *settings);

    // Misc
    void movetoTrash(const QList<AbstractDownloadItem *> &items);

    // Utility
    AbstractDownloadItem* createFileItem(const QUrl &url);
    AbstractDownloadItem* createTorrentItem(const QUrl &url);

signals:
    void jobAppended(DownloadRange range);
    void jobRemoved(DownloadRange range);
    void jobStateChanged(AbstractDownloadItem *item);
    void jobFinished(AbstractDownloadItem *item);
    void jobRenamed(QString oldName, QString newName, bool success);

    void selectionChanged();
    void sortChanged();

private slots:
    void onChanged();
    void onFinished();
    void onRenamed(const QString &oldName, const QString &newName, bool success);
    void startNext(AbstractDownloadItem *item);

    void onSettingsChanged();

    void onQueueChanged(const DownloadRange &range);
    void onQueueChanged(AbstractDownloadItem* item);
    void onQueueChanged();

    void loadQueue();
    void saveQueue();

    void onSpeedTimerTimeout();

private:
    // Network parameters (SSL, Proxy, UserAgent...)
    NetworkManager *m_networkManager = nullptr;
    Settings *m_settings = nullptr;

    // Crash Recovery
    QTimer* m_dirtyQueueTimer = nullptr;
    QString m_queueFile = {};

    QList<AbstractDownloadItem *> m_items = {};

    qreal m_previouSpeed = 0;
    QTimer* m_speedTimer = nullptr;

    // Pool
    int m_maxSimultaneousDownloads = 4;
    qsizetype downloadingCount() const;

    QList<AbstractDownloadItem *> m_selectedItems = {};
    bool m_selectionAboutToChange = false;

    void sortSelectionByIndex();
    void moveUpTo(qsizetype targetIndex);
    void moveDownTo(qsizetype targetIndex);

    inline ResourceItem* createResourceItem(const QUrl &url);
};

#endif // CORE_DOWNLOAD_MANAGER_H
