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

#include <QtCore/QDebug>
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
    connect(this, SIGNAL(jobAppended(IDownloadItem*)), this, SLOT(onQueueChanged(IDownloadItem*)));
    connect(this, SIGNAL(jobRemoved(IDownloadItem*)), this, SLOT(onQueueChanged(IDownloadItem*)));
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
        clear();
        foreach (IDownloadItem* item, downloadItems) {
            DownloadEngine::append(item, false);
        }
    }
}

void DownloadManager::saveQueue()
{
    if (!m_queueFile.isEmpty()) {
        QList<DownloadItem *> items;

        /// \todo remove it
        QList<IDownloadItem *> abstratItems = downloadItems();
        foreach (auto abstratItem, abstratItems) {
            DownloadItem* item = static_cast<DownloadItem*>(abstratItem);
            if (item) {
                items.append(item);
            }
        }


        Session::write(items, m_queueFile);
    }
}

void DownloadManager::onQueueChanged(IDownloadItem */*item*/)
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
void DownloadManager::append(IDownloadItem *item, const bool started)
{
    DownloadEngine::append(item, started);
}

void DownloadManager::append(ResourceItem *item, const bool started)
{
    DownloadItem *downloadItem = new DownloadItem(this);
    downloadItem->setResource(item);
    DownloadEngine::append(downloadItem, started);
}

void DownloadManager::append(const QList<ResourceItem *> &downloadItems, const bool started)
{
    foreach (auto item, downloadItems) {
        append(item, started);
    }
}
