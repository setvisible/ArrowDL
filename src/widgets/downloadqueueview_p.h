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

#ifndef WIDGETS_DOWNLOAD_QUEUE_VIEW_P_H
#define WIDGETS_DOWNLOAD_QUEUE_VIEW_P_H

#include "downloadqueueview.h"

#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>

class AbstractDownloadItem;

class QueueItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT

public:
    explicit QueueItem(AbstractDownloadItem *downloadItem, QTreeWidget *view);

    AbstractDownloadItem* downloadItem() const { return m_downloadItem; }

public slots:
    void updateItem();

private:
    AbstractDownloadItem *m_downloadItem;
};

/******************************************************************************
 ******************************************************************************/
/*!
 * QueueView extends QTreeWidget to allow drag and drop.
 */
class QueueView : public QTreeWidget
{
    friend class DownloadQueueView; /* To acceed protected members */
    Q_OBJECT

public:
    QueueView(QWidget *parent);

};

#endif // WIDGETS_DOWNLOAD_QUEUE_VIEW_P_H
