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
#include <Core/AbstractJob>
#include <Core/JobFile>
#include <Core/JobTorrent>
#include <Core/NetworkManager>
#include <Core/QueueModel>
#include <Core/ResourceItem>
#include <Core/Settings>
#include <Core/Snapshot>

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QtMath>
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
 * \li network requests (GET, POST, PUT, HEAD...)
 */
DownloadManager::DownloadManager(QObject *parent) : QObject(parent)
    , m_queueModel(new QueueModel(this))
    , m_networkManager(new NetworkManager(this))
    , m_snapshot(new Snapshot(this))
    , m_speedTimer(new QTimer(this))
{
    connect(this, SIGNAL(jobFinished(AbstractJob*)),
            this, SLOT(startNext(AbstractJob*)));

    connect(m_speedTimer, SIGNAL(timeout()), this, SLOT(onSpeedTimerTimeout()));
}

DownloadManager::~DownloadManager()
{
    clear();
}

/******************************************************************************
 ******************************************************************************/
QAbstractItemModel *DownloadManager::model() const
{
    return m_queueModel;
}

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
    m_snapshot->setSettings(m_settings);
}

void DownloadManager::onSettingsChanged()
{
    setMaxSimultaneousDownloads(m_settings->maxSimultaneousDownloads());
}

void DownloadManager::activateSnapshot()
{
    m_snapshot->shot();
}

/******************************************************************************
 ******************************************************************************/
NetworkManager* DownloadManager::networkManager() const
{
    return m_networkManager;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Reimplement this method allows the Engine to make Items; like a factory.
 * That makes the unit tests of this class easier, allowing dummy items.
 * \remark Optional
 */
AbstractJob* DownloadManager::createFileItem(const QUrl &url)
{
    ResourceItem* resource = createResourceItem(url);
    auto item = new JobFile(this, resource);
    return item;
}

AbstractJob* DownloadManager::createTorrentItem(const QUrl &url)
{
    ResourceItem* resource = createResourceItem(url);
    resource->setType(ResourceItem::Type::Torrent);
    auto item = new JobTorrent(this, resource);
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

/******************************************************************************
 ******************************************************************************/
qsizetype DownloadManager::downloadingCount() const
{
    auto count = 0;
    for (auto item : m_queueModel->items()) {
       if (item->isDownloading()) {
           count++;
       }
    }
    return count;
}

void DownloadManager::startNext(AbstractJob * /*item*/)
{
    if (downloadingCount() < m_maxSimultaneousDownloads) {
       auto rows = m_queueModel->rowCount();
       for (int i = 0; i < rows; ++i) {
           auto index = m_queueModel->index(i, 0);
           AbstractJob* item = model()->data(index, QueueModel::DownloadItemRole).value<AbstractJob*>();
           // for (auto item : m_queueModel->items()) {
           if (item->state() == AbstractJob::Idle) {
               item->resume();
               startNext(nullptr);
               break;
           }
       }
    }
}

/******************************************************************************
 ******************************************************************************/
qsizetype DownloadManager::count() const
{
    return m_queueModel->rowCount();
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::clear()
{
    m_queueModel->removeRows(0, m_queueModel->rowCount());
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::append(const QList<AbstractJob*> &items, bool started)
{
    if (items.isEmpty()) {
        return;
    }
    for (auto item : items) {
        if (!item) {
            return;
        }

        connect(item, SIGNAL(changed()), this, SLOT(onItemChanged()));
        connect(item, SIGNAL(finished()), this, SLOT(onItemFinished()));
        connect(item, SIGNAL(renamed(QString,QString,bool)), this, SLOT(onItemRenamed(QString,QString,bool)));

        if (started) {
            if (item->isResumable()) {
                item->setState(AbstractJob::Idle);
            }
        } else {
            if (item->isPausable()) {
                item->setState(AbstractJob::Paused);
            }
        }
    }

    m_queueModel->append(items); // inserset row ?
    activateSnapshot();

    if (started) {
        startNext(nullptr);
    }
}

/******************************************************************************
 ******************************************************************************/
int DownloadManager::maxSimultaneousDownloads() const
{
    return m_maxSimultaneousDownloads;
}

void DownloadManager::setMaxSimultaneousDownloads(int number)
{
    m_maxSimultaneousDownloads = number;
}

/******************************************************************************
 ******************************************************************************/
QList<AbstractJob *> DownloadManager::downloadItems() const
{
    return  m_queueModel->items();
}

static inline QList<AbstractJob*> filter(
    const QList<AbstractJob*> &items,
    const QList<AbstractJob::State> &states)
{
    QList<AbstractJob*> list;
    for (auto item : items) {
        for (auto state : states) {
            if (item->state() == state) {
                list.append(item);
            }
        }
    }
    return list;
}

QList<AbstractJob*> DownloadManager::completedJobs() const
{
    return filter(
        downloadItems(),
        {AbstractJob::Completed,
         AbstractJob::Seeding});
}

QList<AbstractJob *> DownloadManager::failedJobs() const
{
    return filter(
        downloadItems(),
        {AbstractJob::Stopped,
         AbstractJob::Skipped,
         AbstractJob::NetworkError,
         AbstractJob::FileError});
}

QList<AbstractJob*> DownloadManager::runningJobs() const
{
    return filter(
        downloadItems(),
        {AbstractJob::Preparing,
         AbstractJob::Connecting,
         AbstractJob::DownloadingMetadata,
         AbstractJob::Downloading,
         AbstractJob::Endgame});
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::onSpeedTimerTimeout()
{
    m_speedTimer->stop();
    m_previouSpeed = 0;
    emit onItemChanged();
}

qreal DownloadManager::totalSpeed()
{
    qreal speed = 0;
    for (auto item : downloadItems()) {
        speed += qMax(item->speed(), qreal(0));
    }
    if (speed > 0) {
        m_previouSpeed = speed;
        m_speedTimer->start(MSEC_SPEED_DISPLAY_TIME);
    }
    return m_previouSpeed;
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::resume(AbstractJob *item)
{
    if (item->isResumable()) {
        item->setReadyToResume();
        startNext(item);
    }
}

void DownloadManager::pause(AbstractJob *item)
{
    if (item->isPausable()) {
        item->pause();
    }
}

void DownloadManager::cancel(AbstractJob *item)
{
    if (item->isCancelable()) {
        item->stop();
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::onItemChanged()
{
    activateSnapshot();
}

void DownloadManager::onItemFinished()
{
    auto downloadItem = qobject_cast<AbstractJob *>(sender());
    emit jobFinished(downloadItem);
}

void DownloadManager::onItemRenamed(const QString &oldName, const QString &newName, bool success)
{
    emit jobRenamed(oldName, newName, success);
}
