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

#include "queuewidgetitem.h"

#include <Constants>
#include <Core/AbstractDownloadItem>
#include <Core/Format>

QueueWidgetItem::QueueWidgetItem(AbstractDownloadItem *downloadItem, QTreeWidget *view)
    : QObject(view)
    , QTreeWidgetItem(view, QTreeWidgetItem::UserType)
    , m_downloadItem(downloadItem)
{
    this->setSizeHint(COL_2_PROGRESS_BAR, QSize(COLUMN_DEFAULT_WIDTH, ROW_DEFAULT_HEIGHT));
    this->setFlags(Qt::ItemIsEditable | flags());

    connect(m_downloadItem, SIGNAL(changed()), this, SLOT(updateItem()));

    updateItem();
}

static QString estimatedTime(AbstractDownloadItem *downloadItem)
{
    switch (downloadItem->state()) {
    case AbstractDownloadItem::Downloading:
        return Format::timeToString(downloadItem->remainingTime());
        break;
    case AbstractDownloadItem::NetworkError:
    case AbstractDownloadItem::FileError:
        return downloadItem->errorMessage();
        break;
    default:
        return downloadItem->stateToString();
        break;
    }
}

void QueueWidgetItem::updateItem()
{
    QString size;
    if (m_downloadItem->bytesTotal() > 0) {
        size = tr("%0 of %1")
                   .arg(Format::fileSizeToString(m_downloadItem->bytesReceived()),
                        Format::fileSizeToString(m_downloadItem->bytesTotal()));
    } else {
        size = tr("Unknown");
    }

    QString speed = Format::currentSpeedToString(m_downloadItem->speed());

    this->setText(COL_0_FILE_NAME, m_downloadItem->localFileName());
    this->setText(COL_1_WEBSITE_DOMAIN, m_downloadItem->sourceUrl().host()); /// \todo domain only
    this->setData(COL_2_PROGRESS_BAR, StateRole, m_downloadItem->state());
    this->setData(COL_2_PROGRESS_BAR, ProgressRole, m_downloadItem->progress());
    this->setText(COL_3_PERCENT, QString("%0%").arg(qMax(0, m_downloadItem->progress())));
    this->setText(COL_4_SIZE, size);
    this->setText(COL_5_ESTIMATED_TIME, estimatedTime(m_downloadItem));
    this->setText(COL_6_SPEED, speed);

    //item->setText(C_COL_7_SEGMENTS, "Unknown");
    // todo etc...
}
