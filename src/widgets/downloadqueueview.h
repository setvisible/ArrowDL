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

class IDownloadItem;
typedef QList<IDownloadItem*> DownloadRange;
class DownloadEngine;
class QueueItem;
class QueueView;

class QMenu;
class DownloadQueueView : public QWidget
{
    Q_OBJECT
public:
    explicit DownloadQueueView(QWidget *parent);
    ~DownloadQueueView();

    const DownloadEngine* engine() const;
    void setEngine(DownloadEngine *downloadEngine);

    QMenu* contextMenu() const;
    void setContextMenu(QMenu *contextMenu);

    QSize sizeHint() const override;

    QList<int> columnWidths() const;
    void setColumnWidths(const QList<int> &widths);

signals:
    void doubleClicked(IDownloadItem *item);
    void selectionChanged();

private slots:
    void onJobAdded(DownloadRange range);
    void onJobRemoved(DownloadRange range);
    void onJobStateChanged(IDownloadItem *item);
    void onSelectionChanged();

    void onQueueViewDoubleClicked(const QModelIndex &index);
    void onQueueViewItemSelectionChanged();

    void showContextMenu(const QPoint &pos) ;

private:
    DownloadEngine *m_downloadEngine;
    QueueView *m_queueView;
    QMenu *m_contextMenu;

    int getIndex(IDownloadItem *downloadItem) const;
    QueueItem* getQueueItem(IDownloadItem *downloadItem);
};

#endif // WIDGETS_DOWNLOAD_QUEUE_VIEW_H
