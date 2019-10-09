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

#include "downloadengine.h"

#include <Core/AbstractDownloadItem>

#include <QtCore/QDebug>

DownloadEngine::DownloadEngine(QObject *parent) : QObject(parent)
  , m_maxSimultaneousDownloads(4)
{
    connect(this, SIGNAL(jobFinished(IDownloadItem*)),
            this, SLOT(startNext(IDownloadItem*)));
}

DownloadEngine::~DownloadEngine()
{
    clear();
}

/******************************************************************************
 ******************************************************************************/
/**
 * \fn void DownloadEngine::jobStateChanged(IDownloadItem *item)
 * This signal is emited whenever the download data or its progress or its state has changed
 */

/******************************************************************************
 ******************************************************************************/
int DownloadEngine::downloadingCount() const
{
    int count = 0;
    foreach (auto item, m_items) {
        if (item->isDownloading()) {
            count++;
        }
    }
    return count;
}

void DownloadEngine::startNext(IDownloadItem */*item*/)
{
    if (downloadingCount() < m_maxSimultaneousDownloads) {
        foreach (auto item, m_items) {
            if (item->state() == IDownloadItem::Idle) {
                item->resume();
                break;
            }
        }
    }
}

/******************************************************************************
 ******************************************************************************/
int DownloadEngine::count() const
{
    return m_items.count();
}

/******************************************************************************
 ******************************************************************************/
void DownloadEngine::clear()
{
    clearSelection();
    foreach (auto item, m_items) {
        remove(item);
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadEngine::append(IDownloadItem *item, const bool started)
{
    AbstractDownloadItem *downloadItem = static_cast<AbstractDownloadItem*>(item);
    if (!downloadItem) {
        return;
    }

    connect(downloadItem, SIGNAL(changed()), this, SLOT(onChanged()));
    connect(downloadItem, SIGNAL(finished()), this, SLOT(onFinished()));
    if (started) {
        if (downloadItem->isResumable()) {
            downloadItem->setState(IDownloadItem::Idle);
        }
    } else {
        if (downloadItem->isPausable()) {
            downloadItem->setState(IDownloadItem::Paused);
        }
    }
    m_items.append(downloadItem);
    emit jobAppended(downloadItem);

    if (started) {
        startNext(0);
    }
}

void DownloadEngine::remove(IDownloadItem *item)
{
    AbstractDownloadItem *downloadItem = static_cast<AbstractDownloadItem*>(item);
    if (!downloadItem) {
        return;
    }

    setSelected(item, false);
    cancel(item); // stop the reply first
    m_items.removeAll(item);
    emit jobRemoved(item);

    downloadItem->deleteLater();
}

void DownloadEngine::remove(const QList<IDownloadItem *> &downloadItems)
{
    foreach (auto item, downloadItems) {
        remove(item);
    }
}

/******************************************************************************
 ******************************************************************************/
const IDownloadItem* DownloadEngine::clientForRow(int row) const
{
    Q_ASSERT(row >=0 && row < m_items.count());
    return m_items.at(row);
}

/******************************************************************************
 ******************************************************************************/
QList<IDownloadItem *> DownloadEngine::downloadItems() const
{
    return m_items;
}

static inline QList<IDownloadItem*> filter(const QList<IDownloadItem*> &downloadItems,
                                           const IDownloadItem::State state)
{
    QList<IDownloadItem*> list;
    foreach (auto item, downloadItems) {
        if (item->state() == state) {
            list.append(item);
        }
    }
    return list;
}

QList<IDownloadItem*> DownloadEngine::waitingJobs() const
{
    return filter(downloadItems(), IDownloadItem::Idle);
}

QList<IDownloadItem*> DownloadEngine::completedJobs() const
{
    return filter(downloadItems(), IDownloadItem::Completed);
}

QList<IDownloadItem*> DownloadEngine::pausedJobs() const
{
    return filter(downloadItems(), IDownloadItem::Paused);
}

QList<IDownloadItem*> DownloadEngine::failedJobs() const
{
    return filter(downloadItems(), IDownloadItem::Stopped);
}

QList<IDownloadItem*> DownloadEngine::runningJobs() const
{
    QList<IDownloadItem*> list;
    foreach (auto item, downloadItems()) {
        IDownloadItem::State state = item->state();
        if ( state != IDownloadItem::Idle &&
             state != IDownloadItem::Completed &&
             state != IDownloadItem::Paused &&
             state != IDownloadItem::Stopped ) {
            list.append(item);
        }
    }
    return list;
}

/******************************************************************************
 ******************************************************************************/
QString DownloadEngine::totalSpeed() const
{
    /// \todo // "750.35 MB/s"
    return "";
}

/******************************************************************************
 ******************************************************************************/
void DownloadEngine::resume(IDownloadItem *item)
{
    if (item->isResumable()) {
        item->setReadyToResume();
        startNext(item);
    }
}

void DownloadEngine::pause(IDownloadItem *item)
{
    if (item->isPausable()) {
        item->pause();
    }
}

void DownloadEngine::cancel(IDownloadItem *item)
{
    if (item->isCancelable()) {
        item->stop();
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadEngine::onChanged()
{
    AbstractDownloadItem *downloadItem = qobject_cast<AbstractDownloadItem *>(sender());
    emit jobStateChanged(downloadItem);
}

void DownloadEngine::onFinished()
{
    AbstractDownloadItem *downloadItem = qobject_cast<AbstractDownloadItem *>(sender());
    emit jobFinished(downloadItem);
}

/******************************************************************************
 ******************************************************************************/
void DownloadEngine::clearSelection()
{
    m_selectedItems.clear();
    emit selectionChanged();
}

QList<IDownloadItem *> DownloadEngine::selection() const
{
    return m_selectedItems;
}

void DownloadEngine::setSelection(const QList<IDownloadItem*> &selection)
{
    m_selectedItems.clear();
    m_selectedItems.append(selection);
    emit selectionChanged();
}

bool DownloadEngine::isSelected(IDownloadItem *item) const
{
    return m_selectedItems.contains(item);
}

void DownloadEngine::setSelected(IDownloadItem* item, bool isSelected)
{
    m_selectedItems.removeAll(item);
    if (isSelected) {
        m_selectedItems.append(item);
    }
    emit selectionChanged();
}

QString DownloadEngine::selectionToString() const
{
    QString ret;
    int count = 0;
    foreach (auto item, m_selectedItems) {
        ret += item->localFileName();
        ret += "\n";
        count++;
        if (count > 10) {
            ret += tr("... (%0 others)").arg(m_selectedItems.count()-10);
        }
    }
    return ret;
}

/******************************************************************************
 ******************************************************************************/
void DownloadEngine::oneMoreSegment()
{
    foreach (auto item, selection()) {
        AbstractDownloadItem *downloadItem = static_cast<AbstractDownloadItem*>(item);
        int segments = downloadItem->maxConnectionSegments();
        segments++;
        downloadItem->setMaxConnectionSegments(segments);
    }
}

void DownloadEngine::oneFewerSegment()
{
    foreach (auto item, selection()) {
        AbstractDownloadItem *downloadItem = static_cast<AbstractDownloadItem*>(item);
        int segments = downloadItem->maxConnectionSegments();
        segments--;
        downloadItem->setMaxConnectionSegments(segments);
    }
}
