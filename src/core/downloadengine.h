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

#ifndef CORE_DOWNLOAD_ENGINE_H
#define CORE_DOWNLOAD_ENGINE_H

#include <Core/IDownloadItem>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QTimer>

using DownloadRange = QList<IDownloadItem *>;

class DownloadEngine : public QObject
{
    Q_OBJECT

public:
    explicit DownloadEngine(QObject *parent = nullptr);
    ~DownloadEngine() override;

    /* Queue Management */
    qsizetype count() const;
    void clear();

    virtual void append(const QList<IDownloadItem *> &items, bool started = false);
    virtual void remove(const QList<IDownloadItem *> &items);

    void removeItems(const QList<IDownloadItem *> &items);
    void updateItems(const QList<IDownloadItem *> &items);

    const IDownloadItem* clientForRow(qsizetype row) const;

    int maxSimultaneousDownloads() const;
    void setMaxSimultaneousDownloads(int number);

    /* Statistics */
    QList<IDownloadItem *> downloadItems() const;
    QList<IDownloadItem *> waitingJobs() const;
    QList<IDownloadItem *> completedJobs() const;
    QList<IDownloadItem *> pausedJobs() const;
    QList<IDownloadItem *> failedJobs() const;
    QList<IDownloadItem *> runningJobs() const;

    qreal totalSpeed();

    /* Actions */
    void resume(IDownloadItem *item);
    void pause(IDownloadItem *item);
    void cancel(IDownloadItem *item);

    /* Selection */
    void clearSelection();
    QList<IDownloadItem *> selection() const;
    void setSelection(const QList<IDownloadItem *> &selection);

    bool isSelected(IDownloadItem *item) const;
    void setSelected(IDownloadItem *item, bool isSelected);

    QString selectionToString() const;
    QString selectionToClipboard() const;

    void beginSelectionChange(); // BUGFIX
    void endSelectionChange();

    void moveCurrentTop();
    void moveCurrentUp();
    void moveCurrentDown();
    void moveCurrentBottom();

    /* Segments */
    void oneMoreSegment();
    void oneFewerSegment();

    /* Utility */
    virtual IDownloadItem* createItem(const QUrl &url);
    virtual IDownloadItem* createTorrentItem(const QUrl &url);

signals:
    void jobAppended(DownloadRange range);
    void jobRemoved(DownloadRange range);
    void jobStateChanged(IDownloadItem *item);
    void jobFinished(IDownloadItem *item);
    void jobRenamed(QString oldName, QString newName, bool success);

    void selectionChanged();
    void sortChanged();

public slots:

private slots:
    void onChanged();
    void onFinished();
    void onRenamed(const QString &oldName, const QString &newName, bool success);
    void startNext(IDownloadItem *item);

private slots:
    void onSpeedTimerTimeout();

private:
    QList<IDownloadItem *> m_items = {};

    qreal m_previouSpeed = 0;
    QTimer m_speedTimer;

    // Pool
    int m_maxSimultaneousDownloads = 4;
    qsizetype downloadingCount() const;

    QList<IDownloadItem *> m_selectedItems = {};
    bool m_selectionAboutToChange = false;

    void sortSelectionByIndex();
    void moveUpTo(qsizetype targetIndex);
    void moveDownTo(qsizetype targetIndex);
};

#endif // CORE_DOWNLOAD_ENGINE_H
