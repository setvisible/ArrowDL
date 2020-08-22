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

#include "checkabletablemodel.h"

#include <QtCore/QAbstractTableModel>

#include <algorithm> /* std::sort */

CheckableTableModel::CheckableTableModel(QObject *parent) : QAbstractTableModel(parent)
{
}

void CheckableTableModel::clear()
{
    m_checkedIndexes.clear();
}

QSet<QModelIndex> CheckableTableModel::checkedIndexes() const
{
    return m_checkedIndexes;
}

QList<int> CheckableTableModel::checkedRows() const
{
    QSet<int> rows;
    auto indexes = m_checkedIndexes.toList();
    foreach (auto index, indexes) {
        rows.insert(index.row());
    }
    auto list = rows.toList();
    std::sort(list.begin(), list.end());
    return list;
}

/******************************************************************************
 ******************************************************************************/
QVariant CheckableTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (role == CheckStateRole) {
        return m_checkedIndexes.contains(index);
    }
    return QVariant();
}

bool CheckableTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    if (index.column() == 0 && role == CheckStateRole) {
        const bool checked = value.toBool();
        if (m_checkedIndexes.contains(index) == checked) {
            return true; // Successful
        }
        if (checked) {
            m_checkedIndexes.insert(index);
        } else {
            m_checkedIndexes.remove(index);
        }
        emit checkStateChanged(index, checked);

        QModelIndex topLeft = index;
        QModelIndex bottomRight = this->index(index.row(), columnCount());
        emit dataChanged(topLeft, bottomRight);
        return true;
    }
    return false;
}

//Qt::ItemFlags CheckableTableModel::flags(const QModelIndex &index) const
//{
//    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
//    // We use the flags() function to ensure that views
//    // know that the model is read-only.
//    //    if (!index.isValid()) {
//    //        return 0;
//    //    }
//    //    if (index.column() == 0) {
//    //       flags |= Qt::ItemIsUserCheckable;
//    //    }
//    return flags;
//}
