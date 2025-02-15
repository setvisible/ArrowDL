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
#include <Core/AbstractDownloadItem>
#include <Core/DownloadFileItem>
#include <Core/DownloadTorrentItem>
#include <Core/NetworkManager>
#include <Core/ResourceItem>
#include <Core/Session>
#include <Core/Settings>

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
 * \li selection?
 * \li network requests (GET, POST, PUT, HEAD...)
 *
 */

DownloadManager::DownloadManager(QObject *parent) : QObject(parent)
    , m_networkManager(new NetworkManager(this))
    , m_speedTimer(new QTimer(this))
{
    connect(this, SIGNAL(jobFinished(AbstractDownloadItem*)),
            this, SLOT(startNext(AbstractDownloadItem*)));

    connect(m_speedTimer, SIGNAL(timeout()), this, SLOT(onSpeedTimerTimeout()));

    /* Auto save of the queue */
    connect(this, SIGNAL(jobAppended(DownloadRange)), this, SLOT(onQueueChanged(DownloadRange)));
    connect(this, SIGNAL(jobRemoved(DownloadRange)), this, SLOT(onQueueChanged(DownloadRange)));
    connect(this, SIGNAL(jobStateChanged(AbstractDownloadItem*)), this, SLOT(onQueueChanged(AbstractDownloadItem*)));
}

DownloadManager::~DownloadManager()
{
    saveQueue();
    clear();
}

/******************************************************************************
 ******************************************************************************/
/**
 * \fn void DownloadManager::jobStateChanged(AbstractDownloadItem *item)
 * This signal is emited whenever the download data or its progress or its state has changed
 */

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
        QList<AbstractDownloadItem*> items;
        Session::read(items, m_queueFile, this);
        clear();
        append(items, false);
    }
}

void DownloadManager::saveQueue()
{
    if (!m_queueFile.isEmpty()) {
        QList<AbstractDownloadItem *> items;

        auto skipCompleted = m_settings->isRemoveCompletedEnabled();
        auto skipCanceled = m_settings->isRemoveCanceledEnabled();
        auto skipPaused = m_settings->isRemovePausedEnabled();

        for (auto item : downloadItems()) {
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
/*!
 * \brief Reimplement this method allows the Engine to make Items; like a factory.
 * That makes the unit tests of this class easier, allowing dummy items.
 * \remark Optional
 */
AbstractDownloadItem* DownloadManager::createFileItem(const QUrl &url)
{
    ResourceItem* resource = createResourceItem(url);
    auto item = new DownloadFileItem(this, resource);
    return item;
}

AbstractDownloadItem* DownloadManager::createTorrentItem(const QUrl &url)
{
    ResourceItem* resource = createResourceItem(url);
    resource->setType(ResourceItem::Type::Torrent);
    auto item = new DownloadTorrentItem(this, resource);
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
    for (auto item : m_items) {
        if (item->isDownloading()) {
            count++;
        }
    }
    return count;
}

void DownloadManager::startNext(AbstractDownloadItem * /*item*/)
{
    if (downloadingCount() < m_maxSimultaneousDownloads) {
        for (auto item : m_items) {
            if (item->state() == AbstractDownloadItem::Idle) {
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
    return m_items.count();
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::clear()
{
    clearSelection();
    removeItems(m_items);
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::append(const QList<AbstractDownloadItem*> &items, bool started)
{
    if (items.isEmpty()) {
        return;
    }
    for (auto item : items) {
        auto downloadItem = dynamic_cast<AbstractDownloadItem*>(item);
        if (!downloadItem) {
            return;
        }

        connect(downloadItem, SIGNAL(changed()), this, SLOT(onChanged()));
        connect(downloadItem, SIGNAL(finished()), this, SLOT(onFinished()));
        connect(downloadItem, SIGNAL(renamed(QString,QString,bool)), this, SLOT(onRenamed(QString,QString,bool)));

        if (started) {
            if (downloadItem->isResumable()) {
                downloadItem->setState(AbstractDownloadItem::Idle);
            }
        } else {
            if (downloadItem->isPausable()) {
                downloadItem->setState(AbstractDownloadItem::Paused);
            }
        }
        m_items.append(downloadItem);
    }

    emit jobAppended(items);

    if (started) {
        startNext(nullptr);
    }
}

void DownloadManager::remove(const QList<AbstractDownloadItem*> &items)
{
    removeItems(items);
}

void DownloadManager::removeItems(const QList<AbstractDownloadItem*> &items)
{
    if (items.isEmpty()) {
        return;
    }
    /* First, deselect */
    beginSelectionChange();
    for (auto item : items) {
        setSelected(item, false);
    }
    endSelectionChange();

    /* Then, remove */
    for (auto item : items) {
        cancel(item); // stop the reply first
        m_items.removeAll(item);
        auto downloadItem = dynamic_cast<AbstractDownloadItem*>(item);
        if (downloadItem) {
            downloadItem->deleteLater();
        }
    }
    emit jobRemoved(items);
}

void DownloadManager::updateItems(const QList<AbstractDownloadItem *> &items)
{
    for (auto item : items) {
        emit jobStateChanged(item);
    }
}

void DownloadManager::movetoTrash(const QList<AbstractDownloadItem*> &items)
{
    if (items.isEmpty()) {
        return;
    }
    /* Then, move to trash */
    for (auto item : items) {
        cancel(item); // stop the reply first
        m_items.removeAll(item);
        auto downloadItem = dynamic_cast<AbstractDownloadItem*>(item);
        if (downloadItem) {
            downloadItem->moveToTrash();
        }
    }
    removeItems(items);
}

/******************************************************************************
 ******************************************************************************/
const AbstractDownloadItem* DownloadManager::clientForRow(qsizetype row) const
{
    Q_ASSERT(row >=0 && row < m_items.count());
    return m_items.at(row);
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
QList<AbstractDownloadItem *> DownloadManager::downloadItems() const
{
    return m_items;
}

static inline QList<AbstractDownloadItem*> filter(
    const QList<AbstractDownloadItem*> &items,
    const QList<AbstractDownloadItem::State> &states)
{
    QList<AbstractDownloadItem*> list;
    for (auto item : items) {
        for (auto state : states) {
            if (item->state() == state) {
                list.append(item);
            }
        }
    }
    return list;
}

QList<AbstractDownloadItem*> DownloadManager::completedJobs() const
{
    return filter(m_items, {AbstractDownloadItem::Completed,
                            AbstractDownloadItem::Seeding});
}

QList<AbstractDownloadItem*> DownloadManager::failedJobs() const
{
    return filter(m_items, {AbstractDownloadItem::Stopped,
                            AbstractDownloadItem::Skipped,
                            AbstractDownloadItem::NetworkError,
                            AbstractDownloadItem::FileError});
}

QList<AbstractDownloadItem*> DownloadManager::runningJobs() const
{
    return filter(m_items, {AbstractDownloadItem::Preparing,
                            AbstractDownloadItem::Connecting,
                            AbstractDownloadItem::DownloadingMetadata,
                            AbstractDownloadItem::Downloading,
                            AbstractDownloadItem::Endgame});
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::onSpeedTimerTimeout()
{
    m_speedTimer->stop();
    m_previouSpeed = 0;
    emit onChanged();
}

qreal DownloadManager::totalSpeed()
{
    qreal speed = 0;
    for (auto item : m_items) {
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
void DownloadManager::resume(AbstractDownloadItem *item)
{
    if (item->isResumable()) {
        item->setReadyToResume();
        startNext(item);
    }
}

void DownloadManager::pause(AbstractDownloadItem *item)
{
    if (item->isPausable()) {
        item->pause();
    }
}

void DownloadManager::cancel(AbstractDownloadItem *item)
{
    if (item->isCancelable()) {
        item->stop();
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::onChanged()
{
    auto downloadItem = qobject_cast<AbstractDownloadItem *>(sender());
    emit jobStateChanged(downloadItem);
}

void DownloadManager::onFinished()
{
    auto downloadItem = qobject_cast<AbstractDownloadItem *>(sender());
    emit jobFinished(downloadItem);
}

void DownloadManager::onRenamed(const QString &oldName, const QString &newName, bool success)
{
    emit jobRenamed(oldName, newName, success);
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::clearSelection()
{
    m_selectedItems.clear();
    emit selectionChanged();
}

QList<AbstractDownloadItem *> DownloadManager::selection() const
{
    return m_selectedItems;
}

void DownloadManager::setSelection(const QList<AbstractDownloadItem*> &selection)
{
    m_selectedItems.clear();
    m_selectedItems.append(selection);
    if (!m_selectionAboutToChange) {
        emit selectionChanged();
    }
}

bool DownloadManager::isSelected(AbstractDownloadItem *item) const
{
    return m_selectedItems.contains(item);
}

void DownloadManager::setSelected(AbstractDownloadItem* item, bool isSelected)
{
    m_selectedItems.removeAll(item);
    if (isSelected) {
        m_selectedItems.append(item);
    }
    if (!m_selectionAboutToChange) {
        emit selectionChanged();
    }
}

QString DownloadManager::selectionToString() const
{
    QString ret;
    int count = 0;
    for (auto item : m_selectedItems) {
        ret += item->localFileName();
        ret += "\n";
        count++;
        if (count > SELECTION_DISPLAY_LIMIT) {
            ret += tr("... (%0 others)").arg(m_selectedItems.count() - SELECTION_DISPLAY_LIMIT);
            break;
        }
    }
    return ret;
}

QString DownloadManager::selectionToClipboard() const
{
    QString ret;
    for (auto item : m_selectedItems) {
        ret += item->sourceUrl().toString();
        ret += "\n";
    }
    return ret;
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::beginSelectionChange()
{
    m_selectionAboutToChange = true;
}

void DownloadManager::endSelectionChange()
{
    m_selectionAboutToChange = false;
    emit selectionChanged();
}

/******************************************************************************
 ******************************************************************************/
void DownloadManager::sortSelectionByIndex()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }
    QMap<qsizetype, AbstractDownloadItem*> map;
    for (auto selectedItem : m_selectedItems) {
        auto index = m_items.indexOf(selectedItem);
        map.insert(index, selectedItem);
    }
    m_selectedItems = map.values();
}

void DownloadManager::moveUpTo(qsizetype targetIndex)
{
    for (auto i = 0; i < m_selectedItems.size(); ++i) {
        auto indexToMove = m_items.indexOf(m_selectedItems.at(i));
        for (auto j = indexToMove; j > targetIndex + i; --j) {
            m_items.swapItemsAt(j, j - 1);
        }
    }
    emit sortChanged();
}

void DownloadManager::moveDownTo(qsizetype targetIndex)
{
    auto count = m_selectedItems.size() - 1;
    for (auto i = count; i >= 0; --i) {
        auto k = count - i;
        auto indexToMove = m_items.indexOf(m_selectedItems.at(i));
        for (auto j = indexToMove; j < targetIndex - k; ++j) {
            m_items.swapItemsAt(j, j + 1);
        }
    }
    emit sortChanged();
}

void DownloadManager::moveCurrentTop()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }
    sortSelectionByIndex();
    moveUpTo(0);
}

void DownloadManager::moveCurrentUp()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }
    sortSelectionByIndex();
    auto targetIndex = qMax(0, m_items.indexOf(m_selectedItems.first()) - 1);
    moveUpTo(targetIndex);
}

void DownloadManager::moveCurrentDown()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }
    sortSelectionByIndex();
    auto targetIndex = qMin(m_items.size() - 1, m_items.indexOf(m_selectedItems.last()) + 1);
    moveDownTo(targetIndex);
}

void DownloadManager::moveCurrentBottom()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }
    sortSelectionByIndex();
    auto targetIndex = m_items.size() - 1;
    moveDownTo(targetIndex);
}
