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

#ifndef WIDGETS_QUEUE_WIDGET_H
#define WIDGETS_QUEUE_WIDGET_H

#include <Widgets/DownloadQueueView>

#include <QtWidgets/QTreeWidget>

class QueueWidgetItem;

class QMouseEvent;
class QTreeWidgetItem;

/*!
 * QueueWidget extends QTreeWidget to allow drag and drop.
 */
class QueueWidget : public QTreeWidget
{
    friend class DownloadQueueView; /* To acceed protected members */
    Q_OBJECT

public:
    QueueWidget(QWidget *parent);

signals:
    void dropped(QueueWidgetItem *queueItem);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QPoint dragStartPosition = {};

    QList<QueueWidgetItem *> toQueueItem(const QList<QTreeWidgetItem *> &items) const;
    QUrl urlFrom(const QueueWidgetItem *queueItem) const;
};

#endif // WIDGETS_QUEUE_WIDGET_H
