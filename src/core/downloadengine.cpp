/* - DownZemAll! - Copyright (C) 2019-present Sebastien Vavassori
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
#include <QtCore/QtMath>

constexpr int selection_display_limit = 10;
constexpr int msec_speed_display_time = 2000;

DownloadEngine::DownloadEngine(QObject *parent) : QObject(parent)
  , m_maxSimultaneousDownloads(4)
  , m_selectionAboutToChange(false)
{
    connect(this, SIGNAL(jobFinished(IDownloadItem*)),
            this, SLOT(startNext(IDownloadItem*)));

    connect(&m_speedTimer, SIGNAL(timeout()), this, SLOT(onSpeedTimerTimeout()));
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

void DownloadEngine::startNext(IDownloadItem * /*item*/)
{
    if (downloadingCount() < m_maxSimultaneousDownloads) {
        foreach (auto item, m_items) {
            if (item->state() == IDownloadItem::Idle) {
                item->resume();
                startNext(Q_NULLPTR);
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
    remove(m_items);
}

/******************************************************************************
 ******************************************************************************/
void DownloadEngine::append(const QList<IDownloadItem*> &items, bool started)
{    
    if (items.isEmpty()) {
        return;
    }
    foreach (auto item, items) {
        auto downloadItem = dynamic_cast<AbstractDownloadItem*>(item);
        if (!downloadItem) {
            return;
        }

        connect(downloadItem, SIGNAL(changed()), this, SLOT(onChanged()));
        connect(downloadItem, SIGNAL(finished()), this, SLOT(onFinished()));
        connect(downloadItem, SIGNAL(renamed(QString, QString, bool)),
                this, SLOT(onRenamed(QString, QString, bool)));

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
    }

    emit jobAppended(items);

    if (started) {
        startNext(Q_NULLPTR);
    }
}

void DownloadEngine::remove(const QList<IDownloadItem*> &items)
{
    if (items.isEmpty()) {
        return;
    }
    /* First, deselect */
    beginSelectionChange();
    foreach (auto item, items) {
        setSelected(item, false);
    }
    endSelectionChange();

    /* Then, remove */
    foreach (auto item, items) {
        cancel(item); // stop the reply first
        m_items.removeAll(item);
        auto downloadItem = dynamic_cast<AbstractDownloadItem*>(item);
        if (!downloadItem) {
            downloadItem->deleteLater();
        }
    }
    emit jobRemoved(items);
}

void DownloadEngine::updateItems(const QList<IDownloadItem *> &items)
{
    foreach (auto item, items) {
        emit jobStateChanged(item);
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
int DownloadEngine::maxSimultaneousDownloads() const
{
    return m_maxSimultaneousDownloads;
}

void DownloadEngine::setMaxSimultaneousDownloads(int number)
{
    m_maxSimultaneousDownloads = number;
}

/******************************************************************************
 ******************************************************************************/
QList<IDownloadItem *> DownloadEngine::downloadItems() const
{
    return m_items;
}

static inline QList<IDownloadItem*> filter(const QList<IDownloadItem*> &items,
                                           const QList<IDownloadItem::State> &states)
{
    QList<IDownloadItem*> list;
    foreach (auto item, items) {
        foreach (auto state, states) {
            if (item->state() == state) {
                list.append(item);
            }
        }
    }
    return list;
}

QList<IDownloadItem*> DownloadEngine::waitingJobs() const
{
    return filter(m_items, {IDownloadItem::Idle});
}

QList<IDownloadItem*> DownloadEngine::completedJobs() const
{
    return filter(m_items, {IDownloadItem::Completed,
                            IDownloadItem::Seeding});
}

QList<IDownloadItem*> DownloadEngine::pausedJobs() const
{
    return filter(m_items, {IDownloadItem::Paused});
}

QList<IDownloadItem*> DownloadEngine::failedJobs() const
{
    return filter(m_items, {IDownloadItem::Stopped,
                            IDownloadItem::Skipped,
                            IDownloadItem::NetworkError,
                            IDownloadItem::FileError});
}

QList<IDownloadItem*> DownloadEngine::runningJobs() const
{
    return filter(m_items, {IDownloadItem::Preparing,
                            IDownloadItem::Connecting,
                            IDownloadItem::DownloadingMetadata,
                            IDownloadItem::Downloading,
                            IDownloadItem::Endgame});
}

/******************************************************************************
 ******************************************************************************/
void DownloadEngine::onSpeedTimerTimeout()
{
    m_speedTimer.stop();
    m_previouSpeed = 0;
    emit onChanged();
}

double DownloadEngine::totalSpeed()
{
    double speed = 0;
    foreach (auto item, m_items) {
        speed += qMax(item->speed(), 0.0);
    }
    if (speed > 0) {
        m_previouSpeed = speed;
        m_speedTimer.start(msec_speed_display_time);
    }
    return m_previouSpeed;
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
    auto downloadItem = qobject_cast<AbstractDownloadItem *>(sender());
    emit jobStateChanged(downloadItem);
}

void DownloadEngine::onFinished()
{
    auto downloadItem = qobject_cast<AbstractDownloadItem *>(sender());
    emit jobFinished(downloadItem);
}

void DownloadEngine::onRenamed(const QString &oldName, const QString &newName, bool success)
{
    emit jobRenamed(oldName, newName, success);
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
    if (!m_selectionAboutToChange) {
        emit selectionChanged();
    }
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
    if (!m_selectionAboutToChange) {
        emit selectionChanged();
    }
}

QString DownloadEngine::selectionToString() const
{
    QString ret;
    int count = 0;
    foreach (auto item, m_selectedItems) {
        ret += item->localFileName();
        ret += "\n";
        count++;
        if (count > selection_display_limit) {
            ret += tr("... (%0 others)").arg(m_selectedItems.count() - selection_display_limit);
            break;
        }
    }
    return ret;
}

QString DownloadEngine::selectionToClipboard() const
{
    QString ret;
    foreach (auto item, m_selectedItems) {
        ret += item->sourceUrl().toString();
        ret += "\n";
    }
    return ret;
}

/******************************************************************************
 ******************************************************************************/
void DownloadEngine::beginSelectionChange()
{
    m_selectionAboutToChange = true;
}

void DownloadEngine::endSelectionChange()
{
    m_selectionAboutToChange = false;
    emit selectionChanged();
}

/******************************************************************************
 ******************************************************************************/
void DownloadEngine::sortSelectionByIndex()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }
    QMap<int, IDownloadItem*> map;
    for (auto selectedItem : m_selectedItems) {
        auto index = m_items.indexOf(selectedItem);
        map.insert(index, selectedItem);
    }
    m_selectedItems = map.values();
}

void DownloadEngine::moveUpTo(int targetIndex)
{
    for (int i = 0, total = m_selectedItems.size(); i < total; ++i) {
        auto indexToMove = m_items.indexOf(m_selectedItems.at(i));
        for (int j = indexToMove; j > targetIndex + i; --j) {
#if QT_VERSION >= QT_VERSION_CHECK(5,13,0)
            m_items.swapItemsAt(j, j - 1);
#else
            m_items.swap(j, j - 1);
#endif
        }
    }
    emit sortChanged();
}

void DownloadEngine::moveDownTo(int targetIndex)
{
    auto count = m_selectedItems.size() - 1;
    for (int i = count; i >= 0; --i) {
        auto i2 = count - i;
        auto indexToMove = m_items.indexOf(m_selectedItems.at(i));
        for (int j = indexToMove; j < targetIndex - i2; ++j) {
#if QT_VERSION >= QT_VERSION_CHECK(5,13,0)
            m_items.swapItemsAt(j, j + 1);
#else
            m_items.swap(j, j + 1);
#endif
        }
    }
    emit sortChanged();
}

void DownloadEngine::moveCurrentTop()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }
    sortSelectionByIndex();
    moveUpTo(0);
}

void DownloadEngine::moveCurrentUp()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }
    sortSelectionByIndex();
    auto targetIndex = qMax(0, m_items.indexOf(m_selectedItems.first()) - 1);
    moveUpTo(targetIndex);
}

void DownloadEngine::moveCurrentDown()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }
    sortSelectionByIndex();
    auto targetIndex = qMin(m_items.size() - 1, m_items.indexOf(m_selectedItems.last()) + 1);
    moveDownTo(targetIndex);
}

void DownloadEngine::moveCurrentBottom()
{
    if (m_selectedItems.isEmpty()) {
        return;
    }
    sortSelectionByIndex();
    auto targetIndex = m_items.size() - 1;
    moveDownTo(targetIndex);
}

/******************************************************************************
 ******************************************************************************/
void DownloadEngine::oneMoreSegment()
{
    foreach (auto item, selection()) {
        auto downloadItem = dynamic_cast<AbstractDownloadItem*>(item);
        int segments = downloadItem->maxConnectionSegments();
        segments++;
        downloadItem->setMaxConnectionSegments(segments);
    }
}

void DownloadEngine::oneFewerSegment()
{
    foreach (auto item, selection()) {
        auto downloadItem = dynamic_cast<AbstractDownloadItem*>(item);
        int segments = downloadItem->maxConnectionSegments();
        segments--;
        downloadItem->setMaxConnectionSegments(segments);
    }
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Reimplement this method allows the Engine to make Items; like a factory.
 * That makes the unit tests of this class easier, allowing dummy items.
 * \remark Optional
 */
IDownloadItem* DownloadEngine::createItem(const QUrl &/*url*/)
{
    return Q_NULLPTR;
}
/*!
 * \sa DownloadEngine::createItem()
 */
IDownloadItem* DownloadEngine::createTorrentItem(const QUrl &/*url*/)
{
    return Q_NULLPTR;
}
