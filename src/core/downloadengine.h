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

#ifndef CORE_DOWNLOAD_ENGINE_H
#define CORE_DOWNLOAD_ENGINE_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

class IDownloadItem;

class DownloadEngine : public QObject
{
    Q_OBJECT

public:
    explicit DownloadEngine(QObject *parent);
    ~DownloadEngine();

    /* Queue Management */
    int count() const;
    void clear();

    virtual void append(IDownloadItem *item, const bool started = false);
    void remove(IDownloadItem *item);
    void remove(const QList<IDownloadItem *> &downloadItems);

    const IDownloadItem* clientForRow(int row) const;

    int maxSimultaneousDownloads() const;
    void setMaxSimultaneousDownloads(int number);

    /* Statistics */
    QList<IDownloadItem *> downloadItems() const;
    QList<IDownloadItem *> waitingJobs() const;
    QList<IDownloadItem *> completedJobs() const;
    QList<IDownloadItem *> pausedJobs() const;
    QList<IDownloadItem *> failedJobs() const;
    QList<IDownloadItem *> runningJobs() const;

    QString totalSpeed() const;

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

    /* Segments */
    void oneMoreSegment();
    void oneFewerSegment();

signals:
    void jobAppended(IDownloadItem *item);
    void jobRemoved(IDownloadItem *item);
    void jobStateChanged(IDownloadItem *item);
    void jobFinished(IDownloadItem *item);

    void selectionChanged();

public slots:

private slots:
    void onChanged();
    void onFinished();
    void startNext(IDownloadItem *item);

private:
    QList<IDownloadItem *> m_items;

    // Pool
    int m_maxSimultaneousDownloads;
    int downloadingCount() const;

    QList<IDownloadItem *> m_selectedItems;
};

#endif // CORE_DOWNLOAD_ENGINE_H
