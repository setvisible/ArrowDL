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

#include "queuewidget.h"

#include <Core/AbstractDownloadItem>
#include <Core/MimeDatabase>
#include <Widgets/QueueWidgetItem>

#include <QtCore/QMimeData>
#include <QtCore/QFileInfo>
#include <QtGui/QDrag>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>


QueueWidget::QueueWidget(QWidget *parent) : QTreeWidget(parent)
{
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // To enable the user to move the items around within the view,
    // we must set the list widget's dragDropMode:
    setDragDropMode(QAbstractItemView::DragOnly);
}

void QueueWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->pos();
    }
    QTreeWidget::mousePressEvent(event);
}

void QueueWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }
    if ((event->pos() - dragStartPosition).manhattanLength()
            < QApplication::startDragDistance()) {
        return;
    }
    auto queueItems = toQueueItem(selectedItems());

    QPixmap pixmap;
    QList<QUrl> urls;
    for (auto queueItem : queueItems) {
        auto url = urlFrom(queueItem);
        if (!url.isEmpty()) {
            if (pixmap.isNull()) {
                pixmap = MimeDatabase::fileIcon(url);
            }
            urls << url;
        }
    }
    if (urls.isEmpty())
        return;

    auto drag = new QDrag(this);
    auto mimeData = new QMimeData;
    mimeData->setUrls(urls);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()/2));

    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
    if (dropAction == Qt::MoveAction) {
        for (auto queueItem : queueItems) {
            emit dropped(queueItem);
        }
    }
}

QList<QueueWidgetItem*> QueueWidget::toQueueItem(const QList<QTreeWidgetItem *> &items) const
{
    QList<QueueWidgetItem*> queueItems;
    for (auto item : items) {
        auto queueItem = dynamic_cast<QueueWidgetItem*>(item);
        if (queueItem)
            queueItems << queueItem;
    }
    return queueItems;
}

QUrl QueueWidget::urlFrom(const QueueWidgetItem *queueItem) const
{
    if (!queueItem)
        return {};

    const AbstractDownloadItem* downloadItem = queueItem->downloadItem();
    if (!downloadItem)
        return {};

    const QFileInfo fi(downloadItem->localFullFileName());
    if (!fi.exists())
        return {};

    return QUrl::fromLocalFile(downloadItem->localFullFileName());
}
