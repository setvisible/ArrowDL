/* - DownZemAll! - Copyright (C) 2019 Sebastien Vavassori
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

#include "resourcemodel.h"

#include <core/ResourceItem>

ResourceModel::ResourceModel(QObject *parent) : QAbstractTableModel(parent)
{
}

ResourceModel::~ResourceModel()
{
    qDeleteAll(m_items);
}

/******************************************************************************
 ******************************************************************************/
void ResourceModel::clear()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
    emit resourceChanged();
}

/******************************************************************************
 ******************************************************************************/
void ResourceModel::addResource(ResourceItem *item)
{
    foreach (auto itemOld, m_items) {
        if (itemOld->url() == item->url()) {
            return;
        }
    }
    beginResetModel();
    m_items << item;
    endResetModel();
    emit resourceChanged();
}

QList<ResourceItem*> ResourceModel::resourceItems() const
{
    return m_items;
}

/******************************************************************************
 ******************************************************************************/
QList<ResourceItem*> ResourceModel::selectedResourceItems() const
{
    QList<ResourceItem*> selection;
    foreach (auto item, m_items) {
        if (item->isSelected()) {
            selection << item;
        }
    }
    return selection;
}

/******************************************************************************
 ******************************************************************************/
void ResourceModel::setDestination(const QString &destination)
{
    foreach (auto item, m_items) {
        item->setDestination(destination);
    }
    emit resourceChanged();
}

void ResourceModel::setMask(const QString &mask)
{
    foreach (auto item, m_items) {
        item->setMask(mask);
    }
    emit resourceChanged();
}


/******************************************************************************
 ******************************************************************************/
int ResourceModel::rowCount(const QModelIndex &) const
{
    return m_items.count();
}

int ResourceModel::columnCount(const QModelIndex &) const
{
    return 5;
}

/******************************************************************************
 ******************************************************************************/
QVariant ResourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    ResourceItem *item = m_items.at(index.row());
    const int col = index.column();

    // Shows the checkbox
    if (role == Qt::UserRole) {
        return m_items.at(index.row())->isSelected();
    }

    if (role == Qt::DisplayRole) {
        if (col == 0) {
            return QVariant();
        } else if (col == 1) {
            return item->url();
        } else if (col == 2) {
            return item->customFileName();
        } else if (col == 3) {
            return item->description();
        } else if (col == 4) {
            return item->mask();
        }
    }
    return QVariant();
}

bool ResourceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    if (role == Qt::UserRole) {
        const bool selected = value.toBool();

        auto item = m_items.at(index.row());
        item->setSelected(selected);

        QModelIndex topLeft = index;
        QModelIndex bottomRight = index.model()->index(index.row(), index.model()->columnCount());
        emit dataChanged(topLeft, bottomRight);
        return true;
    }
    return false;
}

/******************************************************************************
 ******************************************************************************/
QVariant ResourceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("");
        } else if (section == 1) {
            return tr("Download");
        } else if (section == 2) {
            return tr("Resource Name");
        } else if (section == 3) {
            return tr("Description");
        } else if (section == 4) {
            return tr("Mask");
        }
    }
    return QVariant();
}

/******************************************************************************
 ******************************************************************************/
Qt::ItemFlags ResourceModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    // We use the flags() function to ensure that views
    // know that the model is read-only.
    //    if (!index.isValid()) {
    //        return 0;
    //    }
    //    if (index.column() == 0) {
    //       flags |= Qt::ItemIsUserCheckable               ;
    //    }
    return flags;
}
