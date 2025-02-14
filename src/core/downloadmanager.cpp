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

#include "downloadmanager.h"

#include <Constants>
#include <Core/DownloadItem>
#include <Core/DownloadTorrentItem>
#include <Core/NetworkManager>
#include <Core/ResourceItem>
#include <Core/Session>
#include <Core/Settings>

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

using namespace Qt::Literals::StringLiterals;


/*!
 * \class DownloadManager
 *
 * The DownloadManager class manages:
 * \li settings persistence
 * \li queue persistence
 * \li selection?
 * \li network requests (GET, POST, PUT, HEAD...)
 *
 */

DownloadManager::DownloadManager(QObject *parent) : DownloadEngine(parent)
  , m_networkManager(new NetworkManager(this))
{
    /* Auto save of the queue */
    connect(this, SIGNAL(jobAppended(DownloadRange)), this, SLOT(onQueueChanged(DownloadRange)));
    connect(this, SIGNAL(jobRemoved(DownloadRange)), this, SLOT(onQueueChanged(DownloadRange)));
    connect(this, SIGNAL(jobStateChanged(AbstractDownloadItem*)), this, SLOT(onQueueChanged(AbstractDownloadItem*)));
}

DownloadManager::~DownloadManager()
{
    saveQueue();
}

/******************************************************************************
 ******************************************************************************/
/**
 * \fn void DownloadManager::jobStateChanged(AbstractDownloadItem *downloadItem)
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
    m_networkManager->setSettings(m_settings);
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

        QList<AbstractDownloadItem*> abstractItems;
        for (auto item : downloadItems) {
            // Cast items of the list
            abstractItems.append(static_cast<AbstractDownloadItem*>(item));
        }
        clear();
        append(abstractItems, false);
    }
}

void DownloadManager::saveQueue()
{
    if (!m_queueFile.isEmpty()) {
        QList<DownloadItem *> items;

        auto skipCompleted = m_settings->isRemoveCompletedEnabled();
        auto skipCanceled = m_settings->isRemoveCanceledEnabled();
        auto skipPaused = m_settings->isRemovePausedEnabled();

        auto abstractItems = downloadItems();
        for (auto abstractItem : abstractItems) {
            auto item = dynamic_cast<DownloadItem*>(abstractItem);
            if (item) {
                switch (item->state()) {
                case AbstractDownloadItem::Idle:
                case AbstractDownloadItem::Paused:
                case AbstractDownloadItem::Preparing:
                case AbstractDownloadItem::Connecting:
                case AbstractDownloadItem::DownloadingMetadata:
                case AbstractDownloadItem::Downloading:
                case AbstractDownloadItem::Endgame:
                    if (skipPaused) continue;
                    break;

                case AbstractDownloadItem::Completed:
                case AbstractDownloadItem::Seeding:
                    if (skipCompleted) continue;
                    break;

                case AbstractDownloadItem::Stopped:
                case AbstractDownloadItem::Skipped:
                case AbstractDownloadItem::NetworkError:
                case AbstractDownloadItem::FileError:
                    if (skipCanceled) continue;
                    break;
                }
                items.append(item);
            }
        }
        Session::write(items, m_queueFile);
    }
}


void DownloadManager::onQueueChanged(const DownloadRange &/*range*/)
{
    onQueueChanged();
}

void DownloadManager::onQueueChanged(AbstractDownloadItem* /*item*/)
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
        m_dirtyQueueTimer->start(MSEC_AUTO_SAVE);
    }
}

/******************************************************************************
 ******************************************************************************/
NetworkManager* DownloadManager::networkManager() const
{
    return m_networkManager;
}

/******************************************************************************
 ******************************************************************************/
AbstractDownloadItem* DownloadManager::createItem(const QUrl &url)
{
    ResourceItem* resource = createResourceItem(url);
    auto item = new DownloadItem(this);
    item->setResource(resource);
    return item;
}

AbstractDownloadItem* DownloadManager::createTorrentItem(const QUrl &url)
{
    ResourceItem* resource = createResourceItem(url);
    resource->setType(ResourceItem::Type::Torrent);
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
    auto path = settings.value("Path"_L1, {}).toString();
    auto mask = settings.value("Mask"_L1, {}).toString();
    settings.endGroup();

    auto resource = new ResourceItem();
    resource->setUrl(url.toString().toUtf8());
    resource->setCustomFileName({});
    resource->setReferringPage({});
    resource->setDescription({});
    resource->setDestination(path);
    resource->setMask(mask);
    resource->setCheckSum({});
    return resource;
}
