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

#include "downloadqueueview.h"

#include <Constants>
#include <Core/AbstractDownloadItem>
#include <Core/DownloadManager>
#include <Widgets/Globals>
#include <Widgets/QueueWidget>
#include <Widgets/QueueWidgetItem>
#include <Widgets/QueueWidgetItemDelegate>

#include <QtCore/QDebug>
#include <QtCore/QEvent>
#include <QtCore/QIODevice>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>


DownloadQueueView::DownloadQueueView(QWidget *parent) : QWidget(parent)
    , m_queueWidget(new QueueWidget(this))
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    // Main queue list
    m_queueWidget->setItemDelegate(new QueueWidgetItemDelegate(this));
    m_queueWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_queueWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_queueWidget->setAlternatingRowColors(false);
    m_queueWidget->setRootIsDecorated(false);
    m_queueWidget->setMidLineWidth(3);

    setColumnWidths(QList<int>());

    // Edit with second click
    m_queueWidget->setEditTriggers(QAbstractItemView::SelectedClicked);

    connect(m_queueWidget, SIGNAL(itemSelectionChanged()), this, SLOT(onQueueViewItemSelectionChanged()));
    connect(m_queueWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onQueueViewDoubleClicked(QModelIndex)));

    connect(m_queueWidget->itemDelegate(), SIGNAL(commitData(QWidget*)), this, SLOT(onQueueItemCommitData(QWidget*)));

    // Drag-n-Drop
    connect(m_queueWidget, SIGNAL(dropped(QueueWidgetItem*)), this, SLOT(onQueueItemDropped(QueueWidgetItem*)));

    auto layout = new QGridLayout(this);
    layout->addWidget(m_queueWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(layout);

    retranslateUi();
}

/******************************************************************************
 ******************************************************************************/
QSize DownloadQueueView::sizeHint() const
{
    auto header = m_queueWidget->header();

    // Add up the sizes of all header sections. The last section is
    // stretched, so its size is relative to the size of the width;
    // instead of counting it, we count the size of its largest value.
    int width = 200;
    // int width = fontMetrics().horizontalAdvance(tr("Downloading") + "  ");
    for (auto i = 0; i < header->count() - 1; ++i) {
        width += header->sectionSize(i);
    }

    return QSize(width, QWidget::sizeHint().height());
}

/******************************************************************************
 ******************************************************************************/
QByteArray DownloadQueueView::saveState(int version) const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << VERSION_MARKER;
    stream << version;
    stream << columnWidths();
    return data;
}

bool DownloadQueueView::restoreState(const QByteArray &state, int version)
{
    if (state.isEmpty()) {
        return false;
    }
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    int marker;
    int v;
    stream >> marker;
    stream >> v;
    if (stream.status() != QDataStream::Ok || marker != VERSION_MARKER || v != version) {
        return false;
    }
    QList<int> widths;
    stream >> widths;
    setColumnWidths(widths);
    bool restored = true;
    return restored;
}

/******************************************************************************
 ******************************************************************************/
QList<int> DownloadQueueView::columnWidths() const
{
    QList<int> widths;
    for (int column = 0, count = m_queueWidget->columnCount(); column < count; ++column) {
        auto width = m_queueWidget->columnWidth(column);
        widths.append(width);
    }
    return widths;
}

static int defaultColumnWidth(int index)
{
    return index == 0 ? COLUMN_0_DEFAULT_WIDTH : COLUMN_DEFAULT_WIDTH;
}

void DownloadQueueView::setColumnWidths(const QList<int> &widths)
{
    for (int column = 0, count = m_queueWidget->columnCount(); column < count; ++column) {
        if (column < widths.count()) {
            auto width = widths.at(column);
            m_queueWidget->setColumnWidth(column, width);
        } else {
            m_queueWidget->setColumnWidth(column, defaultColumnWidth(column));
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::rename()
{
    if (!m_queueWidget->selectedItems().isEmpty()) {
        auto treeItem = m_queueWidget->selectedItems().first();
        m_queueWidget->setCurrentItem(treeItem, COL_0_FILE_NAME);
        m_queueWidget->editItem(treeItem, COL_0_FILE_NAME);
    }
}

/******************************************************************************
 ******************************************************************************/
DownloadManager *DownloadQueueView::engine() const
{
    return m_downloadManager;
}

void DownloadQueueView::setEngine(DownloadManager *downloadManager)
{
    struct Cx {
        const char *signal;
        const char *slot;
    };
    static const Cx connections[] = {
        { SIGNAL(jobAppended(DownloadRange)),
          SLOT(onJobAdded(DownloadRange)) },
        { SIGNAL(jobRemoved(DownloadRange)),
          SLOT(onJobRemoved(DownloadRange)) },
        { SIGNAL(jobStateChanged(AbstractDownloadItem*)),
          SLOT(onJobStateChanged(AbstractDownloadItem*)) },
        { SIGNAL(selectionChanged()),
          SLOT(onSelectionChanged()) },
        { SIGNAL(sortChanged()),
          SLOT(onSortChanged()) },
        { 0, 0 }
    };

    if (m_downloadManager == downloadManager) {
        return;
    }

    if (m_downloadManager) {
        for (auto cx = &connections[0]; cx->signal; cx++) {
            QObject::disconnect(m_downloadManager, cx->signal, this, cx->slot);
        }
    }
    m_downloadManager = downloadManager;
    if (m_downloadManager) {
        for (auto cx = &connections[0]; cx->signal; cx++) {
            QObject::connect(m_downloadManager, cx->signal, this, cx->slot);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    } else if (event->type() == QEvent::StyleChange) {
        restylizeUi();
    }
    QWidget::changeEvent(event);
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::retranslateUi()
{
    Q_ASSERT(m_queueWidget);
    QStringList headers;
    headers << tr("Download/Name")
            << tr("Domain")
            << tr("Progress")
            << tr("Percent")
            << tr("Size")
            << tr("Est. time")      /* Hidden by default */
            << tr("Speed")          /* Hidden by default */
               ;
    m_queueWidget->setHeaderLabels(headers);

    for (auto index = 0; index < m_queueWidget->topLevelItemCount(); ++index) {
        auto treeItem = m_queueWidget->topLevelItem(index);
        auto queueItem = dynamic_cast<QueueWidgetItem *>(treeItem);
        if (queueItem) {
            queueItem->updateItem();
        }
    }
}

void DownloadQueueView::restylizeUi()
{
    auto itemDelegate = static_cast<QueueWidgetItemDelegate*>(m_queueWidget->itemDelegate());
    if (itemDelegate) {
        itemDelegate->restylizeUi();
        m_queueWidget->update();
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::onJobAdded(const DownloadRange &range)
{
    for (auto item : range) {
        auto downloadItem = dynamic_cast<AbstractDownloadItem*>(item);
        auto queueItem = new QueueWidgetItem(downloadItem, m_queueWidget);
        m_queueWidget->addTopLevelItem(queueItem);
    }
}

void DownloadQueueView::onJobRemoved(const DownloadRange &range)
{
    for (auto item : range) {
        auto index = getIndex(item);
        if (index >= 0) {
            auto treeItem = m_queueWidget->takeTopLevelItem(index);
            auto queueItem = dynamic_cast<QueueWidgetItem*>(treeItem);
            Q_ASSERT(queueItem);
            if (queueItem) {
                queueItem->deleteLater();
            }
        }
    }
}

void DownloadQueueView::onJobStateChanged(AbstractDownloadItem *item)
{
    auto queueItem = getQueueItem(item);
    if (queueItem) {
        queueItem->updateItem();
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::onSelectionChanged()
{
    const QSignalBlocker blocker(m_downloadManager);
    m_downloadManager->beginSelectionChange();

    auto selection = m_downloadManager->selection();
    for (auto index = 0; index < m_queueWidget->topLevelItemCount(); ++index) {
        auto treeItem = m_queueWidget->topLevelItem(index);
        auto queueItem = dynamic_cast<const QueueWidgetItem *>(treeItem);
        auto isSelected = selection.contains(queueItem->downloadItem());
        treeItem->setSelected(isSelected);
    }

    m_downloadManager->endSelectionChange();
}

void DownloadQueueView::onSortChanged()
{
    // Save selection and current item
    auto currentItem = m_queueWidget->currentItem();
    auto selection = m_downloadManager->selection();

    auto items = m_downloadManager->downloadItems();
    for (auto i = 0; i < items.size(); ++i) {
        auto downloadItem = items.at(i);
        auto index = getIndex(downloadItem);
        if (index != -1) {
            // Rem: takeTopLevelItem() changes the selection
            auto treeItem =  m_queueWidget->takeTopLevelItem(index);
            if (treeItem) {
                m_queueWidget->insertTopLevelItem(static_cast<int>(i), treeItem);
            }
        }
    }
    // Restore selection and current item
    m_queueWidget->setCurrentItem(currentItem);
    m_downloadManager->setSelection(selection);
}

/******************************************************************************
 ******************************************************************************/
int DownloadQueueView::getIndex(AbstractDownloadItem *downloadItem) const
{
    for (auto index = 0; index < m_queueWidget->topLevelItemCount(); ++index) {
        auto treeItem = m_queueWidget->topLevelItem(index);
        if (treeItem->type() == QTreeWidgetItem::UserType) {
            auto queueItem = dynamic_cast<const QueueWidgetItem *>(treeItem);
            if (queueItem && downloadItem && queueItem->downloadItem() == downloadItem) {
                return index;
            }
        }
    }
    return -1;
}

QueueWidgetItem* DownloadQueueView::getQueueItem(AbstractDownloadItem *downloadItem)
{
    auto index = getIndex(downloadItem);
    if (index >= 0) {
        auto treeItem = m_queueWidget->topLevelItem(index);
        if (treeItem->type() == QTreeWidgetItem::UserType) {
            auto queueItem = dynamic_cast<QueueWidgetItem *>(treeItem);
            if (queueItem && queueItem->downloadItem() == downloadItem) {
                return queueItem;
            }
        }
    }
    return nullptr;
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::onQueueViewDoubleClicked(const QModelIndex &index)
{
    auto treeItem = m_queueWidget->itemFromIndex(index);
    auto queueItem = dynamic_cast<const QueueWidgetItem *>(treeItem);
    emit doubleClicked(queueItem->downloadItem());
}

/*!
 * Synchronize with the selection in the Engine.
 */
void DownloadQueueView::onQueueViewItemSelectionChanged()
{
    QList<AbstractDownloadItem *> selection;
    for (auto treeItem : m_queueWidget->selectedItems()) {
        auto queueItem = dynamic_cast<const QueueWidgetItem *>(treeItem);
        selection << queueItem->downloadItem();
    }
    m_downloadManager->setSelection(selection);
}

/*!
 * Update the Engine data.
 */
void DownloadQueueView::onQueueItemCommitData(QWidget *editor)
{
    auto lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) {
        auto newName = lineEdit->text();

        // remove extension from base name
        auto pos = newName.lastIndexOf('.');
        if (pos != -1) {
            newName = newName.left(pos);
        }

        auto treeItem = m_queueWidget->currentItem();
        auto queueItem = dynamic_cast<QueueWidgetItem *>(treeItem);
        auto downloadItem = queueItem->downloadItem();

        downloadItem->rename(newName);
        queueItem->updateItem();
    }
}

void DownloadQueueView::onQueueItemDropped(QueueWidgetItem *queueItem)
{
    if (queueItem) {
        QList<AbstractDownloadItem*> items;
        items << queueItem->downloadItem();
        m_downloadManager->remove(items);
    }
}

/******************************************************************************
 ******************************************************************************/
QMenu* DownloadQueueView::contextMenu() const
{
    return m_contextMenu;
}

void DownloadQueueView::setContextMenu(QMenu *contextMenu)
{
    m_contextMenu = contextMenu;
}

void DownloadQueueView::showContextMenu(const QPoint &pos)
{
    if (m_contextMenu) {
        m_contextMenu->exec(mapToGlobal(pos));
    }
}
