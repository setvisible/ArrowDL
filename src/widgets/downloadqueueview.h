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

#ifndef WIDGETS_DOWNLOAD_QUEUE_VIEW_H
#define WIDGETS_DOWNLOAD_QUEUE_VIEW_H

#include <QtWidgets/QWidget>
#include <QtCore/QModelIndex>

class DownloadItem;
class DownloadManager;
class QueueItem;
class QueueView;

class QMenu;
class DownloadQueueView : public QWidget
{
    Q_OBJECT
public:
    explicit DownloadQueueView(QWidget *parent);
    ~DownloadQueueView();

    const DownloadManager* downloadManager() const;
    void setDownloadManager(DownloadManager *downloadManager);

    QMenu* contextMenu() const;
    void setContextMenu(QMenu *contextMenu);

    QSize sizeHint() const override;

    QList<int> columnWidths() const;
    void setColumnWidths(const QList<int> &widths);

signals:
    void doubleClicked(DownloadItem *item);
    void selectionChanged();

private slots:
    void onJobAdded(DownloadItem *downloadItem);
    void onJobRemoved(DownloadItem *downloadItem);
    void onJobStateChanged(DownloadItem *downloadItem);
    void onSelectionChanged();

    void onQueueViewDoubleClicked(const QModelIndex &index);
    void onQueueViewItemSelectionChanged();

    void showContextMenu(const QPoint &pos) ;

private:
    DownloadManager *m_downloadManager;
    QueueView *m_queueView;
    QMenu *m_contextMenu;

    int getIndex(DownloadItem *downloadItem) const;
    QueueItem* getQueueItem(DownloadItem *downloadItem);
};

#endif // WIDGETS_DOWNLOAD_QUEUE_VIEW_H
