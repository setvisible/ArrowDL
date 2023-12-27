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

#include "checkabletableview.h"

#include <Constants>
#include <Core/CheckableTableModel>
#include <Widgets/CheckableItemDelegate>

#include <QtCore/QDebug>
#include <QtGui/QAction>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMenu>


CheckableTableView::CheckableTableView(QWidget *parent) : QTableView(parent) 
{
    setShowGrid(false);

    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAlternatingRowColors(false);
    setMidLineWidth(3);

    auto vHeader = verticalHeader();
    vHeader->setSectionResizeMode(QHeaderView::Fixed);
    vHeader->setDefaultSectionSize(VERTICAL_HEADER_WIDTH);
    vHeader->setVisible(false);

    auto hHeader = horizontalHeader();
    hHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hHeader->setHighlightSections(false);

    connect(hHeader, SIGNAL(sectionCountChanged(int,int)), this, SLOT(onSectionCountChanged(int,int)));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
}

/******************************************************************************
 ******************************************************************************/
void CheckableTableView::setContextMenuCallback(const std::function<void(QMenu*)> &callback)
{
    m_contextMenuCallback = callback;
}

/******************************************************************************
 ******************************************************************************/
QList<int> CheckableTableView::columnWidths() const
{
    QAbstractItemModel *model = this->model();
    Q_ASSERT(model);
    QList<int> widths;
    if (model) {
        for (auto column = 0; column < model->columnCount(); ++column) {
            auto width = columnWidth(column);
            widths.append(width);
        }
    }
    return widths;
}

void CheckableTableView::setColumnWidths(const QList<int> &widths)
{
    QAbstractItemModel *model = this->model();
    Q_ASSERT(model);
    if (model) {
        for (auto column = 0; column < model->columnCount(); ++column) {
            if (column == 0) {
                setColumnWidth(column, CheckableItemDelegate::widthHint());
            } else if (column > 0 && column < widths.count()) {
                auto width = widths.at(column);
                if (width < 0 || width > COLUMN_MAX_WIDTH) {
                    width =  COLUMN_DEFAULT_WIDTH;
                }
                setColumnWidth(column, width);
            } else {
                setColumnWidth(column, COLUMN_DEFAULT_WIDTH);
            }
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void CheckableTableView::onSectionCountChanged(int /*oldCount*/, int newCount)
{
    auto header = qobject_cast<QHeaderView *>(sender());
    if (newCount > 0) {
        header->setSectionResizeMode(0, QHeaderView::Fixed);
        auto parent = qobject_cast<QTableView *>(header->parent());
        if (parent) {
            parent->setColumnWidth(0, CheckableItemDelegate::widthHint());
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void CheckableTableView::showContextMenu(const QPoint &/*pos*/)
{
    auto contextMenu = new QMenu(this);

    QAction actionCheckSelected(tr("Check Selected Items"), contextMenu);
    actionCheckSelected.setIcon(QIcon::fromTheme("check-ok"));
    connect(&actionCheckSelected, SIGNAL(triggered()), this, SLOT(checkSelected()));

    QAction actionUncheckSelected(tr("Uncheck Selected Items"), contextMenu);
    actionUncheckSelected.setIcon(QIcon::fromTheme("check-nok"));
    connect(&actionUncheckSelected, SIGNAL(triggered()), this, SLOT(uncheckSelected()));

    QAction actionToggleCheck(tr("Toggle Check for Selected Items"), contextMenu);
    actionToggleCheck.setIcon(QIcon::fromTheme("check-progress"));
    connect(&actionToggleCheck, SIGNAL(triggered()), this, SLOT(toggleCheck()));
    // --
    QAction actionSelectAll(tr("Select All"), contextMenu);
    actionSelectAll.setIcon(QIcon::fromTheme("select-all"));
    actionSelectAll.setShortcut(QKeySequence::SelectAll);
    connect(&actionSelectAll, SIGNAL(triggered()), this, SLOT(selectAll()));

    QAction actionSelectFiltered(tr("Select Filtered"), contextMenu);
    actionSelectFiltered.setIcon(QIcon::fromTheme("select-completed"));
    actionSelectFiltered.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
    connect(&actionSelectFiltered, SIGNAL(triggered()), this, SLOT(selectFiltered()));

    QAction actionInvertSelection(tr("Invert Selection"), contextMenu);
    actionInvertSelection.setIcon(QIcon::fromTheme("select-invert"));
    actionInvertSelection.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
    connect(&actionInvertSelection, SIGNAL(triggered()), this, SLOT(invertSelection()));

    contextMenu->addAction(&actionCheckSelected);
    contextMenu->addAction(&actionUncheckSelected);
    contextMenu->addAction(&actionToggleCheck);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionSelectAll);
    contextMenu->addAction(&actionSelectFiltered);
    contextMenu->addAction(&actionInvertSelection);

    if (m_contextMenuCallback) {
        m_contextMenuCallback(contextMenu);
    }

    contextMenu->exec(QCursor::pos());
    contextMenu->deleteLater();
}

/******************************************************************************
 ******************************************************************************/
void CheckableTableView::checkSelected()
{
    for (auto index: selectedIndexesAtColumn(0)) {
        auto model = const_cast<QAbstractItemModel*>(index.model());
        model->setData(index, true, CheckableTableModel::CheckStateRole);
    }
}

void CheckableTableView::uncheckSelected()
{
    for (auto index: selectedIndexesAtColumn(0)) {
        auto model = const_cast<QAbstractItemModel*>(index.model());
        model->setData(index, false, CheckableTableModel::CheckStateRole);
    }
}

void CheckableTableView::toggleCheck()
{
    for (auto index: selectedIndexesAtColumn(0)) {
        auto selected = index.model()->data(index, CheckableTableModel::CheckStateRole).toBool();
        auto model = const_cast<QAbstractItemModel*>(index.model());
        model->setData(index, !selected, CheckableTableModel::CheckStateRole);
    }
}

void CheckableTableView::selectFiltered()
{
    selectionModel()->clearSelection();

    auto rowCount = model()->rowCount();
    auto colCount = model()->columnCount();
    for (auto i = 0; i < rowCount; ++i) {

        auto index = model()->index(i, 0);
        auto selected = index.model()->data(index, CheckableTableModel::CheckStateRole).toBool();

        if (selected) {
            for (auto j = 0; j < colCount; ++j) {
                auto selectedIndex = model()->index(i, j);
                selectionModel()->select(selectedIndex, QItemSelectionModel::Select);
            }
        }
    }
}

void CheckableTableView::invertSelection()
{
    auto rowCount = model()->rowCount();
    auto colCount = model()->columnCount();
    for (auto i = 0; i < rowCount; ++i) {
        for (auto j = 0; j < colCount; ++j) {
            auto index = model()->index(i, j);
            selectionModel()->select(index, QItemSelectionModel::Toggle);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
QModelIndexList CheckableTableView::selectedIndexesAtColumn(int column)
{
    QModelIndexList indexes;
    for (auto index: selectionModel()->selectedIndexes()) {
        if (index.column() == column) {
            indexes.append(index);
        }
    }
    return indexes;
}
