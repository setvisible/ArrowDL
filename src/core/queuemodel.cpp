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

#include "queuemodel.h"

#include <Constants>
#include <Core/AbstractJob>
#include <Core/ResourceItem>
#include <Core/Format>

#include <QtCore/QAbstractTableModel>
#include <QtCore/QDebug>
#include <QtGui/QFont>

QueueModel::QueueModel(QObject *parent) : QAbstractTableModel(parent)
{
}

/******************************************************************************
 ******************************************************************************/
int QueueModel::rowCount(const QModelIndex &parent) const
{
    return m_items.count();
}

int QueueModel::columnCount(const QModelIndex &parent) const
{
    return COL_COUNT;
}

QVariant QueueModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.row() < 0 || index.row() >= rowCount()) {
        return {};
    }

    const AbstractJob* item = m_items.at(index.row());

    if (role == DownloadItemRole) {
        QVariant variant;
        variant.setValue(item);
        return variant;

    } else if (role == CopyToClipboardRole) {
        return item->sourceUrl().toString();

    } else if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case COL_0_FILE_NAME:      return item->localFileName();
        case COL_1_WEBSITE_DOMAIN: return item->sourceUrl().host(); /// \todo domain only
        case COL_2_PROGRESS_BAR:   return {};
        case COL_3_PERCENT:        return QString("%0%").arg(qMax(0, item->progress()));
        case COL_4_FILE_SIZE:      return fileSize(item);
        case COL_5_ESTIMATED_TIME: return estimatedTime(item);
        case COL_6_SPEED:          return Format::currentSpeedToString(item->speed());
        // case C_COL_7_SEGMENTS:     return "Unknown";
        // todo etc...
        default:
            break;
        }

    } else if (role == Qt::EditRole) {
        if (index.column() == COL_0_FILE_NAME) {
            return item->localFileName();
        }

    } else if (role == Qt::TextAlignmentRole) {
        return int(Qt::AlignLeft | Qt::AlignVCenter);

    } else if (role == StateRole) {
        if (index.column() == COL_2_PROGRESS_BAR) {
            return item->state();
        }

    } else if (role == ProgressRole) {
        if (index.column() == COL_2_PROGRESS_BAR) {
            return item->progress();
        }
    }
    return {};
}

/******************************************************************************
 ******************************************************************************/
QVariant QueueModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (section < 0 || section >= columnCount()) {
            return {};
        }
        if (role == Qt::DisplayRole) {
            switch (section) {
            case COL_0_FILE_NAME:      return tr("Download/Name");
            case COL_1_WEBSITE_DOMAIN: return tr("Domain");
            case COL_2_PROGRESS_BAR:   return tr("Progress");
            case COL_3_PERCENT:        return tr("Percent");
            case COL_4_FILE_SIZE:      return tr("Size");
            case COL_5_ESTIMATED_TIME: return tr("Est. time");  /* Hidden by default */
            case COL_6_SPEED:          return tr("Speed");      /* Hidden by default */
            default:
                return {};
            }

        } else if (role == Qt::TextAlignmentRole) {
            return int(Qt::AlignLeft | Qt::AlignVCenter);

        }
        return {};
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

bool QueueModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    // nothing, just sends signals
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(value)
    Q_UNUSED(role)
    emit headerDataChanged(Qt::Horizontal, 0, COL_COUNT);
    return true;
}

/******************************************************************************
 ******************************************************************************/
bool QueueModel::insertRows(int row, int count, const QModelIndex &)
{
    if (count < 1 || row < 0 || row > rowCount()) {
        return false;
    }
    beginInsertRows(QModelIndex(), row, row + count - 1);
    AbstractJob *item = nullptr;
    for (int r = row; r < row + count; ++r) {
        m_items.insert(r, item);
    }
    endInsertRows();
    return true;
}

bool QueueModel::removeRows(int row, int count, const QModelIndex &)
{
    if (count < 1 || row < 0 || (row + count) > rowCount()) {
        return false;
    }
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    AbstractJob *item = nullptr;
    for (int r = row; r < row + count; ++r) {
        item = m_items.takeAt(row);
        if (item) {
            disconnectItem(item);
            delete item;
        }
    }
    endRemoveRows();
    return true;
}

bool QueueModel::moveRows(const QModelIndex &, int sourceRow, int count,
                          const QModelIndex &, int destinationChild)
{
    if (sourceRow < 0
        || sourceRow + count - 1 >= rowCount()
        || destinationChild < 0
        || destinationChild > rowCount()
        || sourceRow == destinationChild
        || sourceRow == destinationChild - 1
        || count <= 0) {
        return false;
    }
    if (!beginMoveRows(QModelIndex(), sourceRow, sourceRow + count - 1,
                       QModelIndex(), destinationChild)) {
        return false;
    }
    int fromRow = sourceRow;
    if (destinationChild < sourceRow) {
        fromRow += count - 1;
    } else {
        destinationChild--;
    }
    while (count--) {
        m_items.move(fromRow, destinationChild);
    }
    endMoveRows();
    return true;
}

/******************************************************************************
 ******************************************************************************/
Qt::ItemFlags QueueModel::flags(const QModelIndex &index) const
{
    // We use the flags() function to ensure that views
    // know that the model is read-only but filename is editable.
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (index.isValid() && index.column() == COL_0_FILE_NAME) {
        flags |= Qt::ItemIsEditable;
    }
    return flags;
}

/******************************************************************************
 ******************************************************************************/
inline QString QueueModel::fileSize(const AbstractJob *item) const
{
    if (item->bytesTotal() > 0) {
        return tr("%0 of %1")
            .arg(Format::fileSizeToString(item->bytesReceived()),
                 Format::fileSizeToString(item->bytesTotal()));
    }
    return tr("Unknown");
}

inline QString QueueModel::estimatedTime(const AbstractJob *item) const
{
    switch (item->state()) {
    case AbstractJob::Downloading:
        return Format::timeToString(item->remainingTime());
        break;
    case AbstractJob::NetworkError:
    case AbstractJob::FileError:
        return item->errorMessage();
        break;
    default:
        return item->stateToString();
        break;
    }
}

/******************************************************************************
 ******************************************************************************/
QList<AbstractJob *> QueueModel::items() const
{
    return m_items;
}

void QueueModel::append(const QList<AbstractJob*> &items)
{
    if (items.isEmpty()) {
        return;
    }
    const int first = rowCount();
    const int last = first + items.count();
    emit beginInsertRows(QModelIndex(), first, last);
    for (auto item : items) {
        connectItem(item);
    }
    m_items.append(items);
    emit endInsertRows();
}

/******************************************************************************
 ******************************************************************************/
void QueueModel::connectItem(const AbstractJob *item)
{
    connect(item, SIGNAL(changed()), this, SLOT(onItemChanged()));
}

void QueueModel::disconnectItem(const AbstractJob *item)
{
    disconnect(item, SIGNAL(changed()), this, SLOT(onItemChanged()));
}

void QueueModel::onItemChanged()
{
    auto item = qobject_cast<AbstractJob *>(sender());
    auto row = m_items.indexOf(item);
    auto topLeft = index(row, 0);
    auto bottomRight = index(row, COL_COUNT - 1);
    auto roles = QList<int>(); // All roles
    emit dataChanged(topLeft, bottomRight, roles);
}

