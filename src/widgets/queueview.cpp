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

#include "queueview.h"

#include <Constants>
#include <Core/AbstractJob>
#include <Core/MimeDatabase>
#include <Core/QueueModel>
#include <Core/Utils>
#include <Widgets/Globals>
#include <Widgets/QueueView>
#include <Widgets/QueueViewItemDelegate>

#include <QtCore/QDebug>
#include <QtCore/QEvent>
#include <QtCore/QFileInfo>
#include <QtCore/QIODevice>
#include <QtCore/QMimeData>
#include <QtGui/QDrag>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>

/*!
 * \class QueueView
 *
 * QueueView extends QTableView to allow drag and drop.
 */
QueueView::QueueView(QWidget *parent) : QTableView(parent)
{
    setItemDelegate(new QueueViewItemDelegate(this));

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSortingEnabled(false);
    setAlternatingRowColors(false);
    setMidLineWidth(3);
    setShowGrid(false);

    horizontalHeader()->setVisible(true);
    horizontalHeader()->setStretchLastSection(true);

    verticalHeader()->setVisible(false);
    verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setMinimumSectionSize(ROW_DEFAULT_HEIGHT);
    verticalHeader()->setMaximumSectionSize(ROW_DEFAULT_HEIGHT);
    verticalHeader()->setDefaultSectionSize(ROW_DEFAULT_HEIGHT);
    verticalHeader()->resizeSections(QHeaderView::Fixed);

    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // Edit with second click
    setEditTriggers(QAbstractItemView::SelectedClicked);

    // To enable the user to move the items around within the view,
    // we must set the list widget's dragDropMode:
    setDragDropMode(QAbstractItemView::DragOnly);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    setColumnWidths(QList<int>());

    connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClicked(QModelIndex)));
    connect(itemDelegate(), SIGNAL(commitData(QWidget*)), this, SLOT(onCommitData(QWidget*)));

    // Drag-n-Drop

    retranslateUi();
}

/******************************************************************************
 ******************************************************************************/
void QueueView::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);

    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(onRowsInserted(QModelIndex,int,int)));
    connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(onRowsRemoved(QModelIndex,int,int)));
    connect(model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(onRowsRemoved(QModelIndex,int,int,QModelIndex,int)));
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QList<int>)),
            this, SLOT(onDataChanged(QModelIndex,QModelIndex,QList<int>)));
    connect(model, SIGNAL(modelReset()), this, SLOT(onModelReset()));


    // While model() is null, selectionModel() is also null.
    // Thus the signal slot connections of selectionModel() only works after the model() is set.
    // For convenience the signal slot connections are set in setModel() directly.
    connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(onCurrentChanged(QModelIndex,QModelIndex)));
    connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onSelectionChanged(QItemSelection,QItemSelection)));
}

/******************************************************************************
 ******************************************************************************/
void QueueView::changeEvent(QEvent *event)
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
void QueueView::retranslateUi()
{
}

void QueueView::restylizeUi()
{
    auto delegate = static_cast<QueueViewItemDelegate*>(itemDelegate());
    if (delegate) {
        delegate->restylizeUi();
        update();
    }
}

/******************************************************************************
 ******************************************************************************/
QList<AbstractJob *> QueueView::selectedJobs() const
{
    QList<AbstractJob *> jobs;
    for (auto index : selectionModel()->selectedRows()) {
        auto job = getJobAtRow(index.row());
        jobs.append(job);
    }
    return jobs;
}

/******************************************************************************
 ******************************************************************************/
void QueueView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->pos();
    }
    QTableView::mousePressEvent(event);
}

void QueueView::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }
    if ((event->pos() - dragStartPosition).manhattanLength()
        < QApplication::startDragDistance()) {
        return;
    }
    auto jobs = selectedJobs();

    QPixmap pixmap;
    QList<QUrl> urls;
    for (auto job : jobs) {
        auto url = urlFrom(job);
        if (!url.isEmpty()) {
            if (pixmap.isNull()) {
                pixmap = MimeDatabase::fileIcon(url);
            }
            urls << url;
        }
    }
    if (urls.isEmpty()) {
        return;
    }

    auto drag = new QDrag(this);
    auto mimeData = new QMimeData;
    mimeData->setUrls(urls);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()/2));

    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
    if (dropAction == Qt::MoveAction) {
        removeSelected();
    }
}

QUrl QueueView::urlFrom(const AbstractJob *job) const
{
    if (!job)
        return {};

    const QFileInfo fi(job->localFullFileName());
    if (!fi.exists())
        return {};

    return QUrl::fromLocalFile(job->localFullFileName());
}

/******************************************************************************
 ******************************************************************************/
QSize QueueView::sizeHint() const
{
    // Add up the sizes of all header sections. The last section is
    // stretched, so its size is relative to the size of the width;
    // instead of counting it, we count the size of its largest value.
    int width = 200;
    // int width = fontMetrics().horizontalAdvance(tr("Downloading") + "  ");
    for (auto i = 0; i < horizontalHeader()->count() - 1; ++i) {
        width += horizontalHeader()->sectionSize(i);
    }

    return QSize(width, QWidget::sizeHint().height());
}

/******************************************************************************
 ******************************************************************************/
QByteArray QueueView::saveState(int version) const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << VERSION_MARKER;
    stream << version;
    stream << columnWidths();
    return data;
}

bool QueueView::restoreState(const QByteArray &state, int version)
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
QList<int> QueueView::columnWidths() const
{
    QList<int> widths;
    for (int column = 0, count = model()->columnCount(); column < count; ++column) {
        auto width = columnWidth(column);
        widths.append(width);
    }
    return widths;
}

static int defaultColumnWidth(int index)
{
    return index == 0 ? COLUMN_0_DEFAULT_WIDTH : COLUMN_DEFAULT_WIDTH;
}

void QueueView::setColumnWidths(const QList<int> &widths)
{
    if (!model()) {
        return;
    }
    for (int column = 0, count = model()->columnCount(); column < count; ++column) {
        if (column < widths.count()) {
            auto width = widths.at(column);
            setColumnWidth(column, width);
        } else {
            setColumnWidth(column, defaultColumnWidth(column));
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void QueueView::rename()
{
    auto indexes = selectionModel()->selectedRows(COL_0_FILE_NAME);
    if (indexes.isEmpty()) {
        return;
    }
    auto index = indexes.first();
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    edit(index);
}

/******************************************************************************
 ******************************************************************************/
void QueueView::selectAll()
{
    QTableView::selectAll();
}

void QueueView::selectNone()
{
    selectionModel()->clearSelection();
    selectionModel()->clearCurrentIndex();
}

void QueueView::invertSelection()
{
    selectionModel()->clearCurrentIndex();
    auto topLeft = model()->index(0, 0);
    auto bottomRight = model()->index(model()->rowCount() - 1, 0);
    QItemSelection selection(topLeft, bottomRight);
    selectionModel()->select(selection, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
}

void QueueView::selectCompleted()
{
    selectNone();
    for (int row = 0, count = model()->rowCount(); row < count; ++row) {
        auto index = model()->index(row, 0);
        auto job = getJobAtRow(row);
        if (job->state() == AbstractJob::Completed) {
            selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
}

QString QueueView::selectionToString() const
{
    QString ret;
    auto jobs = selectedJobs();
    int count = 0;
    for (auto job : jobs) {
        ret += job->localFileName();
        ret += "\n";
        count++;
        if (count > SELECTION_DISPLAY_LIMIT) {
            ret += tr("... (%0 others)").arg(jobs.count() - SELECTION_DISPLAY_LIMIT);
            break;
        }
    }
    return ret;
}

QString QueueView::selectionToClipboard() const
{
    QString ret;
    for (auto index : selectionModel()->selectedRows()) {
        ret += model()->data(index, QueueModel::CopyToClipboardRole).toString();
        ret += "\n";
    }
    return ret;
}

/******************************************************************************
 ******************************************************************************/
void QueueView::removeCompleted()
{
    auto selection = selectionModel()->selection();
    selectCompleted();
    removeSelected();
    selectionModel()->clearSelection();
    selectionModel()->clearCurrentIndex();
    selectionModel()->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void QueueView::removeSelected()
{
    QList<int> rows;
    for (auto index : selectionModel()->selectedRows()) {
        rows.append(index.row());
    }
    if (rows.isEmpty()) {
        return;
    }
    // Sort in descending order
    std::sort(rows.begin(), rows.end());
    std::reverse(rows.begin(), rows.end());

    // To avoid emitting multiple signals, remove by groups instead of one by one
    // Don't do this:
    // for (auto row : rows) {
    //     model()->removeRow(row);
    // }
    auto ranges = Utils::transformToRanges(rows, false);
    for (auto range : ranges) {
        int row = range.second;
        int count = range.first - range.second + 1;
        model()->removeRows(row, count);
    }

    /// \todo improve this
    selectionModel()->clearSelection();
    selectionModel()->clearCurrentIndex();
    //selectionModel()->select(model()->index(rows.last(), 0), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

void QueueView::removeAll()
{
    model()->removeRows(0, model()->rowCount());
    selectionModel()->clearSelection();
    selectionModel()->clearCurrentIndex();
}

void QueueView::moveSelectionToTrash()
{
    auto jobs = selectedJobs();
    for (auto job : jobs) {
        job->moveToTrash();
    }
    removeSelected();
}

/******************************************************************************
 ******************************************************************************/
void QueueView::move(Direction direction)
{
    QList<int> rows;
    for (auto index : selectionModel()->selectedRows()) {
        rows.append(index.row());
    }
    if (rows.isEmpty()) {
        return;
    }
    // Sort because selectedRows() returns by order of selection, not by order in the queue
    std::sort(rows.begin(), rows.end());

    int destinationChild = 0;
    switch (direction) {
    case Up:
        destinationChild = qMax(0, rows.first() - 1);
        break;
    case Top:
        destinationChild = 0;
        break;
    case Down:
        destinationChild = qMin(model()->rowCount(), rows.last() + 2);
        break;
    case Bottom:
        destinationChild = model()->rowCount();
        break;
    default:
        break;
    }

    if (direction == Down || direction == Bottom) {
        // Inverse order
        std::reverse(rows.begin(), rows.end());
    }
    for (auto sourceRow : rows) {
        model()->moveRow(QModelIndex(), sourceRow, QModelIndex(), destinationChild);
        switch (direction) {
        case Up:
        case Top:
            destinationChild++;
            break;
        case Down:
        case Bottom:
            destinationChild--;
            break;
        default:
            break;
        }
    }

    switch (direction) {
    case Up:
        scrollTo(model()->index(destinationChild - rows.count(), 0));
        break;
    case Top:
        scrollToTop();
        break;
    case Down:
        scrollTo(model()->index(destinationChild + rows.count(), 0));
        break;
    case Bottom:
        scrollToBottom();
        break;
    default:
        break;
    }
}

void QueueView::moveUp()
{
    move(Up);
}

void QueueView::moveTop()
{
    move(Top);
}

void QueueView::moveDown()
{
    move(Down);
}

void QueueView::moveBottom()
{
    move(Bottom);
}

/******************************************************************************
 ******************************************************************************/
void QueueView::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)
    emit dataChanged();
}

void QueueView::onRowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)
    emit dataChanged();
}

void QueueView::onRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                            const QModelIndex &destinationParent, int destinationRow)
{
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destinationParent)
    Q_UNUSED(destinationRow)
    emit dataChanged();
}

void QueueView::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles)
{
    Q_UNUSED(topLeft)
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)
    emit dataChanged();
}

void QueueView::onModelReset()
{
    emit dataChanged();
}

void QueueView::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    emit selectionChanged();
}

void QueueView::onCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(current)
    Q_UNUSED(previous)
    emit selectionChanged();
}

/******************************************************************************
 ******************************************************************************/
AbstractJob* QueueView::getJobAtRow(const int row) const
{
    auto index = model()->index(row, 0);
    AbstractJob* job = model()->data(index, QueueModel::JobRole).value<AbstractJob*>();
    return job;
}

/******************************************************************************
 ******************************************************************************/
void QueueView::onDoubleClicked(const QModelIndex &index)
{
    auto job = getJobAtRow(index.row());
    emit doubleClicked(job);
}

void QueueView::onCommitData(QWidget *editor)
{
    auto lineEdit = qobject_cast<QLineEdit*>(editor);
    if (!lineEdit) {
        return;
    }

    // Remove extension from base name
    auto newName = lineEdit->text();
    auto pos = newName.lastIndexOf('.');
    if (pos != -1) {
        newName = newName.left(pos);
    }
    auto index = selectionModel()->currentIndex();
    auto job = getJobAtRow(index.row());
    job->rename(newName);
}

/******************************************************************************
 ******************************************************************************/
QMenu* QueueView::contextMenu() const
{
    return m_contextMenu;
}

void QueueView::setContextMenu(QMenu *contextMenu)
{
    m_contextMenu = contextMenu;
}

void QueueView::showContextMenu(const QPoint &pos)
{
    if (m_contextMenu) {
        m_contextMenu->exec(mapToGlobal(pos));
    }
}
