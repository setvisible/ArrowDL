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

#include "downloadmanager.h"

#include <Core/DownloadItem>
#include <Core/ResourceItem>
#include <Core/Session>
#include <Core/Settings>

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QtCore/QSaveFile>
#include <QtGui/QDesktopServices>
#include <QtNetwork/QtNetwork>

#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

DownloadManager::DownloadManager(QObject *parent) : QObject(parent)
  , m_networkManager(new QNetworkAccessManager(this))
  , m_maxSimultaneousDownloads(4)
  , m_settings(Q_NULLPTR)
  , m_dirtyQueueTimer(Q_NULLPTR)
  , m_queueFile(QString())
{
    /* Auto save of the queue */
    connect(this, SIGNAL(jobAppended(DownloadItem*)), this, SLOT(onQueueChanged(DownloadItem*)));
    connect(this, SIGNAL(jobRemoved(DownloadItem*)), this, SLOT(onQueueChanged(DownloadItem*)));
    connect(this, SIGNAL(jobStateChanged(DownloadItem*)), this, SLOT(onQueueChanged(DownloadItem*)));

    connect(this, SIGNAL(jobFinished(DownloadItem*)), this, SLOT(startNext(DownloadItem*)));
}

DownloadManager::~DownloadManager()
{
    saveQueue();
    clear();
}

/******************************************************************************
 ******************************************************************************/
/**
 * \fn void DownloadManager::jobStateChanged(DownloadItem *downloadItem)
 * This signal is emited whenever the download data or its progress or its state has changed
 */

/******************************************************************************
 ******************************************************************************/
Settings *DownloadManager::settings() const
{
    return m_settings;
}

void DownloadManager::setSettings(Settings *settings)
{
    if (m_settings) {
        disconnect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
    }
    m_settings = settings;
    if (m_settings) {
        connect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
    }
}

void DownloadManager::onSettingsChanged()
{
    // reload the queue here
    if (m_queueFile != m_settings->database()) {
        m_queueFile = m_settings->database();
        loadQueue();
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::loadQueue()
{
    if (!m_queueFile.isEmpty()) {
        QList<DownloadItem *> downloadItems;
        Session::read(downloadItems, m_queueFile, this);
        clear();
        foreach (auto item, downloadItems) {
            append(item, false);
        }
    }
}

void DownloadManager::saveQueue()
{
    if (!m_queueFile.isEmpty()) {
        Session::write(downloadItems(), m_queueFile);
    }
}

void DownloadManager::onQueueChanged(DownloadItem */*item*/)
{
    if (!m_dirtyQueueTimer) {
        m_dirtyQueueTimer = new QTimer(this);
        m_dirtyQueueTimer->setSingleShot(true);
        connect(m_dirtyQueueTimer, SIGNAL(timeout()), SLOT(saveQueue()));
    }
    if (!m_dirtyQueueTimer->isActive()) {
        m_dirtyQueueTimer->start(3000);
    }
}

/******************************************************************************
 ******************************************************************************/
int DownloadManager::downloadingCount() const
{
    int count = 0;
    foreach (auto item, m_items) {
        if (item->isDownloading()) {
            count++;
        }
    }
    return count;
}

void DownloadManager::startNext(DownloadItem */*item*/)
{
    if (downloadingCount() < m_maxSimultaneousDownloads) {
        foreach (auto item, m_items) {
            if (item->state() == DownloadItem::Idle) {
                item->resume();
                break;
            }
        }
    }
}

/******************************************************************************
 ******************************************************************************/
int DownloadManager::count() const
{
    return m_items.count();
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::clear()
{
    clearSelection();
    foreach (auto item, m_items) {
        remove(item);
    }
}

QNetworkAccessManager* DownloadManager::networkManager()
{
    return m_networkManager;
}

void DownloadManager::append(DownloadItem *downloadItem, const bool started)
{    
    connect(downloadItem, SIGNAL(changed()), this, SLOT(onChanged()));
    connect(downloadItem, SIGNAL(finished()), this, SLOT(onFinished()));
    if (started) {
        //        downloadItem->resume();
        if (downloadItem->isResumable()) {
            downloadItem->setState(DownloadItem::Idle);
        }
    } else {
        //        downloadItem->pause();
        if (downloadItem->isPausable()) {
            downloadItem->setState(DownloadItem::Paused);
        }
    }
    m_items << downloadItem;
    emit jobAppended(downloadItem);
}

void DownloadManager::append(ResourceItem *item, const bool started)
{
    DownloadItem *downloadItem = new DownloadItem(this);
    downloadItem->setResource(item);
    append(downloadItem, started);
}

void DownloadManager::append(const QList<ResourceItem *> &downloadItems, const bool started)
{
    foreach (auto item, downloadItems) {
        append(item, started);
    }
}

void DownloadManager::remove(DownloadItem *item)
{
    setSelected(item, false);
    cancel(item); // stop the reply first
    m_items.removeAll(item);
    emit jobRemoved(item);
    item->deleteLater();
}

void DownloadManager::remove(const QList<DownloadItem *> &downloadItems)
{
    foreach (auto item, downloadItems) {
        remove(item);
    }
}

/******************************************************************************
 ******************************************************************************/
const DownloadItem* DownloadManager::clientForRow(int row) const
{
    Q_ASSERT(row >=0 && row < m_items.count());
    return m_items.at(row);
}

/******************************************************************************
 ******************************************************************************/
QList<DownloadItem*> DownloadManager::downloadItems() const
{
    return m_items;
}

static inline QList<DownloadItem*> filter(const QList<DownloadItem*> &downloadItems,
                                          const DownloadItem::State state)
{
    QList<DownloadItem*> list;
    foreach (auto item, downloadItems) {
        if (item->state() == state) {
            list.append(item);
        }
    }
    return list;
}

QList<DownloadItem*> DownloadManager::waitingJobs() const
{
    return filter(downloadItems(), DownloadItem::Idle);
}

QList<DownloadItem*> DownloadManager::completedJobs() const
{
    return filter(downloadItems(), DownloadItem::Completed);
}

QList<DownloadItem*> DownloadManager::pausedJobs() const
{
    return filter(downloadItems(), DownloadItem::Paused);
}

QList<DownloadItem*> DownloadManager::failedJobs() const
{
    return filter(downloadItems(), DownloadItem::Stopped);
}

QList<DownloadItem*> DownloadManager::runningJobs() const
{
    QList<DownloadItem*> list;
    foreach (auto item, downloadItems()) {
        DownloadItem::State state = item->state();
        if ( state != DownloadItem::Idle &&
             state != DownloadItem::Completed &&
             state != DownloadItem::Paused &&
             state != DownloadItem::Stopped ) {
            list.append(item);
        }
    }
    return list;
}

/******************************************************************************
 ******************************************************************************/
QString DownloadManager::totalSpeed() const
{
    /// \todo // "750.35 MB/s"
    return "";
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::resume(DownloadItem *item)
{
    if (item->isResumable()) {
        item->resume();
    }
}

void DownloadManager::pause(DownloadItem *item)
{
    if (item->isPausable()) {
        item->pause();
    }
}

void DownloadManager::cancel(DownloadItem *item)
{
    if (item->isCancelable()) {
        item->stop();
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::onChanged()
{
    DownloadItem *downloadItem = qobject_cast<DownloadItem *>(sender());
    emit jobStateChanged(downloadItem);
}

void DownloadManager::onFinished()
{
    DownloadItem *downloadItem = qobject_cast<DownloadItem *>(sender());
    emit jobFinished(downloadItem);
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::clearSelection()
{
    m_selectedItems.clear();
    emit selectionChanged();
}

QList<DownloadItem*> DownloadManager::selection() const
{
    return m_selectedItems;
}

void DownloadManager::setSelection(const QList<DownloadItem*> &selection)
{
    m_selectedItems.clear();
    m_selectedItems.append(selection);
    emit selectionChanged();
}

bool DownloadManager::isSelected(DownloadItem *item) const
{
    return m_selectedItems.contains(item);
}

void DownloadManager::setSelected(DownloadItem* item, bool isSelected)
{
    m_selectedItems.removeAll(item);
    if (isSelected) {
        m_selectedItems.append(item);
    }
    emit selectionChanged();
}

QString DownloadManager::selectionToString() const
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
