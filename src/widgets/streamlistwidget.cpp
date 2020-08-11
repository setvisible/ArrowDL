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

#include "streamlistwidget.h"
#include "ui_streamlistwidget.h"

#include <Widgets/CheckableItemDelegate>

#include <QtCore/QDebug>
#include <QtGui/QMovie>


StreamListWidget::StreamListWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::StreamListWidget)
  , m_model(new StreamTableModel(this))
{
    ui->setupUi(this);

    ui->playlistTableView->setItemDelegate(new CheckableItemDelegate(ui->playlistTableView));
    ui->playlistTableView->setModel(m_model);

    adjustSize();

    connect(m_model, &CheckableTableModel::checkStateChanged,
            this, &StreamListWidget::onCheckStateChanged);

    connect(ui->playlistTableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(onSelectionChanged(const QItemSelection &, const QItemSelection &)));

    /* Fancy GIF animation */
    QMovie *movie = new QMovie(":/icons/menu/stream_wait_16x16.gif");
    ui->waitingIconLabel->setMovie(movie);
    movie->start();

    setEmpty();
    retranslateUi();
}

StreamListWidget::~StreamListWidget()
{
    delete ui;
}

void StreamListWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void StreamListWidget::retranslateUi()
{
    m_model->retranslateUi();
}

/******************************************************************************
 ******************************************************************************/
bool StreamListWidget::isValid() const
{
    return m_state == Normal;
}

void StreamListWidget::setEmpty()
{
    setState(Empty);
    m_model->clear();
}

void StreamListWidget::setWaitMessage()
{
    setState(Downloading);
    m_model->clear();
}

void StreamListWidget::setErrorMessage(QString errorMessage)
{
    setState(StreamListWidget::Error);
    ui->errorMessageLabel->setText(errorMessage);
    m_model->clear();
}

void StreamListWidget::setStreamInfoList(StreamInfoPtr streamInfo)
{
    setStreamInfoList(QList<StreamInfoPtr>() << streamInfo);
}

void StreamListWidget::setStreamInfoList(QList<StreamInfoPtr> streamInfoList)
{
    setState(StreamListWidget::Normal);
    m_model->setStreamInfoList(streamInfoList);
    if (!streamInfoList.isEmpty()) {
        // Check all the playlist
        for (int i = 0; i < streamInfoList.count(); ++i) {
            m_model->setData(m_model->index(i, 0), true, CheckableTableModel::CheckStateRole);
        }
        // Select the first entry
        ui->playlistTableView->selectRow(0);
    }
    // Eventually, hide the playlist panel
    ui->playlistPanelWidget->setVisible(streamInfoList.count() > 1);
}

StreamListWidget::State StreamListWidget::state() const
{
    return m_state;
}

void StreamListWidget::setState(State state)
{
    m_state = state;
    switch (m_state) {
    case Empty:       ui->stackedWidget->setCurrentWidget(ui->pageEmpty);       break;
    case Downloading: ui->stackedWidget->setCurrentWidget(ui->pageDownloading); break;
    case Normal:      ui->stackedWidget->setCurrentWidget(ui->pageNormal);      break;
    case Error:       ui->stackedWidget->setCurrentWidget(ui->pageError);       break;
    }
}

/******************************************************************************
 ******************************************************************************/
QList<int> StreamListWidget::columnWidths() const
{
    return ui->playlistTableView->columnWidths();
}

void StreamListWidget::setColumnWidths(const QList<int> &widths)
{
    ui->playlistTableView->setColumnWidths(widths);
}

/******************************************************************************
 ******************************************************************************/
void StreamListWidget::onSelectionChanged(const QItemSelection &, const QItemSelection &)
{
    auto selectedRows = ui->playlistTableView->selectionModel()->selectedRows(0);
    QSet<int> rows;
    foreach (auto index, selectedRows) {
        rows << index.row();
    }
    auto list = rows.toList();
    if (list.count() == 1) {
        StreamInfoPtr streamInfo = m_model->itemAt(list.first());
        ui->streamWidget->setStreamInfo(streamInfo);
        ui->streamWidget->setVisible(true);
    } else {
        ui->streamWidget->setVisible(false);
    }
}

void StreamListWidget::onCheckStateChanged()
{
    qDebug() << Q_FUNC_INFO;
    // selected items changed -> recalc le size total
}

/******************************************************************************
 ******************************************************************************/
QList<StreamInfoPtr> StreamListWidget::selection() const
{
    return m_model->selection();
}

/******************************************************************************
 ******************************************************************************/
StreamTableModel::StreamTableModel(QObject *parent) : CheckableTableModel(parent)
{
    retranslateUi();
}

void StreamTableModel::clear()
{
    beginResetModel();
    CheckableTableModel::clear();
    m_items.clear();
    endResetModel();
}

void StreamTableModel::retranslateUi()
{
    m_headers = QStringList()
            << QString() // checkbox
            << tr("#")
            << tr("Title");
}

void StreamTableModel::setStreamInfoList(QList<StreamInfoPtr> streamInfoList)
{
    clear();
    if (!streamInfoList.isEmpty()) {
        QModelIndex parent = QModelIndex(); // empty is always root
        beginInsertRows(parent, 0, streamInfoList.count());
        m_items = streamInfoList;
        endInsertRows();
        // emit dataChanged(index(0, 0), index(rowCount(), columnCount()), {Qt::DisplayRole});
    }
}

StreamInfoPtr StreamTableModel::itemAt(int row) const
{
    Q_ASSERT(row >= 0 && row < m_items.count());
    return m_items.at(row);
}

QList<StreamInfoPtr> StreamTableModel::selection() const
{
    QList<StreamInfoPtr> selection;
    foreach (int row, this->checkedRows()) {
        if (row >= 0 && row < m_items.count()) {
            selection << m_items.at(row);
        }
    }
    return selection;
}

int StreamTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_headers.count();
}

QVariant StreamTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section >= 0 && section < m_headers.count()) {
            return m_headers.at(section);
        } else {
            return QVariant();
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

int StreamTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.count();
}

QVariant StreamTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.row() >= rowCount() || index.row() < 0) {
        return QVariant();
    }
    StreamInfoPtr streamInfo = m_items.at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case  0: return QVariant();
        case  1: return streamInfo->playlist_index;
        case  2: return streamInfo->fulltitle;
        default:
            break;
        }
    }
    return CheckableTableModel::data(index, role);
}
