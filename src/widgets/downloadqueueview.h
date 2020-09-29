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

#ifndef WIDGETS_DOWNLOAD_QUEUE_VIEW_H
#define WIDGETS_DOWNLOAD_QUEUE_VIEW_H

#include <QtWidgets/QWidget>
#include <QtCore/QModelIndex>

class IDownloadItem;
using DownloadRange = QList<IDownloadItem *>;
class DownloadEngine;
class QueueItem;
class QueueView;

class QMenu;
class DownloadQueueView : public QWidget
{
    Q_OBJECT
public:
    explicit DownloadQueueView(QWidget *parent);
    ~DownloadQueueView() Q_DECL_OVERRIDE = default;

    DownloadEngine* engine() const;
    void setEngine(DownloadEngine *downloadEngine);

    QMenu* contextMenu() const;
    void setContextMenu(QMenu *contextMenu);

    QSize sizeHint() const override;

    QByteArray saveState(int version = 0) const;
    bool restoreState(const QByteArray &state, int version = 0);

public slots:
    void rename();

signals:
    void doubleClicked(IDownloadItem *item);
    void selectionChanged();

protected slots:
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onJobAdded(const DownloadRange &range);
    void onJobRemoved(const DownloadRange &range);
    void onJobStateChanged(IDownloadItem *item);
    void onSelectionChanged();
    void onSortChanged();

    void onQueueViewDoubleClicked(const QModelIndex &index);
    void onQueueViewItemSelectionChanged();
    void onQueueItemCommitData(QWidget *editor);
    void onQueueItemDropped(QueueItem *queueItem);

    void showContextMenu(const QPoint &pos) ;

private:
    DownloadEngine *m_downloadEngine;
    QueueView *m_queueView;
    QMenu *m_contextMenu;

    void retranslateUi();

    QList<int> columnWidths() const;
    void setColumnWidths(const QList<int> &widths);

    int getIndex(IDownloadItem *downloadItem) const;
    QueueItem* getQueueItem(IDownloadItem *downloadItem);
};

#endif // WIDGETS_DOWNLOAD_QUEUE_VIEW_H
