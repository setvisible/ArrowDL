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

#include <Core/CheckableTableModel>
#include <Widgets/CheckableItemDelegate>

#include <QtCore/QDebug>
#include <QtWidgets/QAction>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMenu>

#define C_VERTICAL_HEADER_WIDTH    22
#define C_COLUMN_DEFAULT_WIDTH    100
#define C_COLUMN_MAX_WIDTH       1000


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

    QHeaderView *vHeader = verticalHeader();
    vHeader->setSectionResizeMode(QHeaderView::Fixed);
    vHeader->setDefaultSectionSize(C_VERTICAL_HEADER_WIDTH);
    vHeader->setVisible(false);

    QHeaderView *hHeader = horizontalHeader();
    hHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hHeader->setHighlightSections(false);

    connect(hHeader, SIGNAL(sectionCountChanged(int,int)),
            this, SLOT(onSectionCountChanged(int,int)));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));
}

CheckableTableView::~CheckableTableView()
{
}

/******************************************************************************
 ******************************************************************************/
void CheckableTableView::setContextMenuCallback(std::function<void(QMenu*)> callback)
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
        for (int column = 0; column < model->columnCount(); ++column) {
            const int width = columnWidth(column);
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
        for (int column = 0; column < model->columnCount(); ++column) {
            if (column == 0) {
                setColumnWidth(column, CheckableItemDelegate::widthHint());
            } else if (column > 0 && column < widths.count()) {
                int width = widths.at(column);
                if (width < 0 || width > C_COLUMN_MAX_WIDTH) {
                    width =  C_COLUMN_DEFAULT_WIDTH;
                }
                setColumnWidth(column, width);
            } else {
                setColumnWidth(column, C_COLUMN_DEFAULT_WIDTH);
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
    actionCheckSelected.setIcon(QIcon(":/icons/menu/check_ok_16x16.png"));
    connect(&actionCheckSelected, SIGNAL(triggered()), this, SLOT(checkSelected()));

    QAction actionUncheckSelected(tr("Uncheck Selected Items"), contextMenu);
    actionUncheckSelected.setIcon(QIcon(":/icons/menu/check_nok_16x16.png"));
    connect(&actionUncheckSelected, SIGNAL(triggered()), this, SLOT(uncheckSelected()));

    QAction actionToggleCheck(tr("Toggle Check for Selected Items"), contextMenu);
    actionToggleCheck.setIcon(QIcon(":/icons/menu/check_progress_16x16.png"));
    connect(&actionToggleCheck, SIGNAL(triggered()), this, SLOT(toggleCheck()));
    // --
    QAction actionSelectAll(tr("Select All"), contextMenu);
    actionSelectAll.setIcon(QIcon(":/icons/menu/select_all_32x32.png"));
    actionSelectAll.setShortcut(QKeySequence::SelectAll);
    connect(&actionSelectAll, SIGNAL(triggered()), this, SLOT(selectAll()));

    QAction actionSelectFiltered(tr("Select Filtered"), contextMenu);
    actionSelectFiltered.setIcon(QIcon(":/icons/menu/select_completed_32x32.png"));
    actionSelectFiltered.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
    connect(&actionSelectFiltered, SIGNAL(triggered()), this, SLOT(selectFiltered()));

    QAction actionInvertSelection(tr("Invert Selection"), contextMenu);
    actionInvertSelection.setIcon(QIcon(":/icons/menu/select_invert_32x32.png"));
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
    foreach (auto index, selectedIndexesAtColumn(0)) {
        auto model = const_cast<QAbstractItemModel*>(index.model());
        model->setData(index, true, CheckableTableModel::CheckStateRole);
    }
}

void CheckableTableView::uncheckSelected()
{
    foreach (auto index, selectedIndexesAtColumn(0)) {
        auto model = const_cast<QAbstractItemModel*>(index.model());
        model->setData(index, false, CheckableTableModel::CheckStateRole);
    }
}

void CheckableTableView::toggleCheck()
{
    foreach (auto index, selectedIndexesAtColumn(0)) {
        const bool selected = index.model()->data(index, CheckableTableModel::CheckStateRole).toBool();
        auto model = const_cast<QAbstractItemModel*>(index.model());
        model->setData(index, !selected, CheckableTableModel::CheckStateRole);
    }
}

void CheckableTableView::selectFiltered()
{
    selectionModel()->clearSelection();

    const int rowCount = model()->rowCount();
    const int colCount = model()->columnCount();
    for (int i = 0; i < rowCount; ++i) {

        const QModelIndex &index = model()->index(i, 0);
        const bool selected = index.model()->data(index, CheckableTableModel::CheckStateRole).toBool();

        if (selected) {
            for (int j = 0; j < colCount; ++j) {
                const QModelIndex &selectedIndex = model()->index(i, j);
                selectionModel()->select(selectedIndex, QItemSelectionModel::Select);
            }
        }
    }
}

void CheckableTableView::invertSelection()
{
    const int rowCount = model()->rowCount();
    const int colCount = model()->columnCount();
    for (int i = 0; i < rowCount; ++i) {
        for (int j = 0; j < colCount; ++j) {
            const QModelIndex &index = model()->index(i, j);
            selectionModel()->select(index, QItemSelectionModel::Toggle);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
QModelIndexList CheckableTableView::selectedIndexesAtColumn(int column)
{
    QModelIndexList indexes;
    foreach (auto index, selectionModel()->selectedIndexes()) {
        if (index.column() == column) {
            indexes.append(index);
        }
    }
    return indexes;
}
