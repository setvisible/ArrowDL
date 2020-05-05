/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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
#include <Core/DownloadTorrentItem>
#include <Core/ResourceItem>
#include <Core/Session>
#include <Core/Settings>

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtNetwork/QNetworkAccessManager>


/*!
 * \class DownloadManager
 *
 * The DownloadManager class manages:
 * \li settings
 * \li load/save of the queue
 * \li selection?
 * \li network management
 *
 */

/*!
 * \brief Constructor
 */
DownloadManager::DownloadManager(QObject *parent) : DownloadEngine(parent)
  , m_networkManager(new QNetworkAccessManager(this))
  , m_settings(Q_NULLPTR)
  , m_dirtyQueueTimer(Q_NULLPTR)
  , m_queueFile(QString())
{
    /* Auto save of the queue */
    connect(this, SIGNAL(jobAppended(DownloadRange)), this, SLOT(onQueueChanged(DownloadRange)));
    connect(this, SIGNAL(jobRemoved(DownloadRange)), this, SLOT(onQueueChanged(DownloadRange)));
    connect(this, SIGNAL(jobStateChanged(IDownloadItem*)), this, SLOT(onQueueChanged(IDownloadItem*)));
}

DownloadManager::~DownloadManager()
{
    saveQueue();
}

/******************************************************************************
 ******************************************************************************/
/**
 * \fn void DownloadManager::jobStateChanged(IDownloadItem *downloadItem)
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
    setMaxSimultaneousDownloads(m_settings->maxSimultaneousDownloads());
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
        QList<DownloadItem*> downloadItems;
        Session::read(downloadItems, m_queueFile, this);

        QList<IDownloadItem*> abstractItems;
        foreach (auto item, downloadItems) {
            // Cast items of the list
            abstractItems.append(static_cast<IDownloadItem*>(item));
        }
        clear();
        append(abstractItems, false);
    }
}

void DownloadManager::saveQueue()
{
    if (!m_queueFile.isEmpty()) {
        QList<DownloadItem *> items;

        const bool skipCompleted = m_settings->isRemoveCompletedEnabled();
        const bool skipCanceled = m_settings->isRemoveCanceledEnabled();
        const bool skipPaused = m_settings->isRemovePausedEnabled();

        QList<IDownloadItem *> abstractItems = downloadItems();
        foreach (auto abstractItem, abstractItems) {
            auto item = dynamic_cast<DownloadItem*>(abstractItem);
            if (item) {
                switch (item->state()) {
                case IDownloadItem::Idle:
                case IDownloadItem::Paused:
                case IDownloadItem::Preparing:
                case IDownloadItem::Connecting:
                case IDownloadItem::Downloading:
                case IDownloadItem::Endgame:
                    if (skipPaused) continue;
                    break;

                case IDownloadItem::Completed:
                    if (skipCompleted) continue;
                    break;

                case IDownloadItem::Stopped:
                case IDownloadItem::Skipped:
                case IDownloadItem::NetworkError:
                case IDownloadItem::FileError:
                    if (skipCanceled) continue;
                    break;
                }
                items.append(item);
            }
        }
        Session::write(items, m_queueFile);
    }
}


void DownloadManager::onQueueChanged(DownloadRange /*range*/)
{
    onQueueChanged();
}

void DownloadManager::onQueueChanged(IDownloadItem* /*item*/)
{
    onQueueChanged();
}

void DownloadManager::onQueueChanged()
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
QNetworkAccessManager* DownloadManager::networkManager()
{
    return m_networkManager;
}

/******************************************************************************
 ******************************************************************************/
IDownloadItem* DownloadManager::createItem(const QUrl &url)
{
    ResourceItem* resource = createResourceItem(url);
    auto item = new DownloadItem(this);
    item->setResource(resource);
    return item;
}

IDownloadItem* DownloadManager::createTorrentItem(const QUrl &url)
{
    ResourceItem* resource = createResourceItem(url);
    resource->setTorrentEnabled(true);
    auto item = new DownloadTorrentItem(this);
    item->setResource(resource);
    return item;
}

/******************************************************************************
 ******************************************************************************/
inline ResourceItem* DownloadManager::createResourceItem(const QUrl &url)
{
    QSettings settings;
    settings.beginGroup("Wizard");
    const QString path = settings.value("Path", QString()).toString();
    const QString mask = settings.value("Mask", QString()).toString();
    settings.endGroup();

    auto resource = new ResourceItem();
    resource->setUrl(url.toString().toUtf8());
    resource->setCustomFileName(QString());
    resource->setReferringPage(QString());
    resource->setDescription(QString());
    resource->setDestination(path);
    resource->setMask(mask);
    resource->setCheckSum(QString());
    return resource;
}
