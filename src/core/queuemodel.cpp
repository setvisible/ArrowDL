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
    return m_jobs.count();
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

    const AbstractJob *job = m_jobs.at(index.row());

    if (role == JobRole) {
        QVariant variant;
        variant.setValue(job);
        return variant;

    } else if (role == CopyToClipboardRole) {
        return job->sourceUrl().toString();

    } else if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case COL_0_FILE_NAME:      return job->localFileName();
        case COL_1_WEBSITE_DOMAIN: return job->sourceUrl().host(); /// \todo domain only
        case COL_2_PROGRESS_BAR:   return {};
        case COL_3_PERCENT:        return QString("%0%").arg(qMax(0, job->progress()));
        case COL_4_FILE_SIZE:      return fileSize(job);
        case COL_5_ESTIMATED_TIME: return estimatedTime(job);
        case COL_6_SPEED:          return Format::currentSpeedToString(job->speed());
        // case C_COL_7_SEGMENTS:     return "Unknown";
        // todo etc...
        default:
            break;
        }

    } else if (role == Qt::EditRole) {
        if (index.column() == COL_0_FILE_NAME) {
            return job->localFileName();
        }

    } else if (role == Qt::TextAlignmentRole) {
        return int(Qt::AlignLeft | Qt::AlignVCenter);

    } else if (role == StateRole) {
        if (index.column() == COL_2_PROGRESS_BAR) {
            return job->state();
        }

    } else if (role == ProgressRole) {
        if (index.column() == COL_2_PROGRESS_BAR) {
            return job->progress();
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
    AbstractJob *job = nullptr;
    for (int r = row; r < row + count; ++r) {
        m_jobs.insert(r, job);
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
    AbstractJob *job = nullptr;
    for (int r = row; r < row + count; ++r) {
        job = m_jobs.takeAt(row);
        if (job) {
            disconnectJob(job);
            delete job;
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
        m_jobs.move(fromRow, destinationChild);
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
inline QString QueueModel::fileSize(const AbstractJob *job) const
{
    if (job->bytesTotal() > 0) {
        return tr("%0 of %1")
            .arg(Format::fileSizeToString(job->bytesReceived()),
                 Format::fileSizeToString(job->bytesTotal()));
    }
    return tr("Unknown");
}

inline QString QueueModel::estimatedTime(const AbstractJob *job) const
{
    switch (job->state()) {
    case AbstractJob::Downloading:
        return Format::timeToString(job->remainingTime());
        break;
    case AbstractJob::NetworkError:
    case AbstractJob::FileError:
        return job->errorMessage();
        break;
    default:
        return job->stateToString();
        break;
    }
}

/******************************************************************************
 ******************************************************************************/
QList<AbstractJob *> QueueModel::jobs() const
{
    return m_jobs;
}

void QueueModel::append(const QList<AbstractJob*> &jobs)
{
    if (jobs.isEmpty()) {
        return;
    }
    const int first = rowCount();
    const int last = first + jobs.count();
    emit beginInsertRows(QModelIndex(), first, last);
    for (auto job : jobs) {
        connectJob(job);
    }
    m_jobs.append(jobs);
    emit endInsertRows();
}

/******************************************************************************
 ******************************************************************************/
void QueueModel::connectJob(const AbstractJob *job)
{
    connect(job, SIGNAL(changed()), this, SLOT(onJobChanged()));
}

void QueueModel::disconnectJob(const AbstractJob *job)
{
    disconnect(job, SIGNAL(changed()), this, SLOT(onJobChanged()));
}

void QueueModel::onJobChanged()
{
    auto job = qobject_cast<AbstractJob *>(sender());
    auto row = m_jobs.indexOf(job);
    auto topLeft = index(row, 0);
    auto bottomRight = index(row, COL_COUNT - 1);
    auto roles = QList<int>(); // All roles
    emit dataChanged(topLeft, bottomRight, roles);
}

