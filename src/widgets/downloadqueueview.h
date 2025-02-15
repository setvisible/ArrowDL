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

#ifndef WIDGETS_DOWNLOAD_QUEUE_VIEW_H
#define WIDGETS_DOWNLOAD_QUEUE_VIEW_H

#include <Core/AbstractDownloadItem>

#include <QtWidgets/QWidget>
#include <QtCore/QModelIndex>

using DownloadRange = QList<AbstractDownloadItem *>;
class DownloadManager;
class QueueWidgetItem;
class QueueWidget;

class QMenu;

class DownloadQueueView : public QWidget
{
    Q_OBJECT
public:
    explicit DownloadQueueView(QWidget *parent);
    ~DownloadQueueView() override = default;

    DownloadManager* engine() const;
    void setEngine(DownloadManager *downloadManager);

    QMenu* contextMenu() const;
    void setContextMenu(QMenu *contextMenu);

    QSize sizeHint() const override;

    QByteArray saveState(int version = 0) const;
    bool restoreState(const QByteArray &state, int version = 0);

public slots:
    void rename();

signals:
    void doubleClicked(AbstractDownloadItem *item);
    void selectionChanged();

protected slots:
    void changeEvent(QEvent *event) override;

private slots:
    void onJobAdded(const DownloadRange &range);
    void onJobRemoved(const DownloadRange &range);
    void onJobStateChanged(AbstractDownloadItem *item);
    void onSelectionChanged();
    void onSortChanged();

    void onQueueViewDoubleClicked(const QModelIndex &index);
    void onQueueViewItemSelectionChanged();
    void onQueueItemCommitData(QWidget *editor);
    void onQueueItemDropped(QueueWidgetItem *queueItem);

    void showContextMenu(const QPoint &pos) ;

private:
    DownloadManager *m_downloadManager = nullptr;
    QueueWidget *m_queueWidget = nullptr;
    QMenu *m_contextMenu = nullptr;

    void retranslateUi();
    void restylizeUi();

    QList<int> columnWidths() const;
    void setColumnWidths(const QList<int> &widths);

    int getIndex(AbstractDownloadItem *downloadItem) const;
    QueueWidgetItem* getQueueItem(AbstractDownloadItem *downloadItem);
};

#endif // WIDGETS_DOWNLOAD_QUEUE_VIEW_H
