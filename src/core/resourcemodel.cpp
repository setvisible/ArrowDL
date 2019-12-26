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

#include <Core/ResourceItem>

ResourceModel::ResourceModel(QObject *parent) : QAbstractTableModel(parent)
{
    connect(this, SIGNAL(resourceChanged()), this, SLOT(onResourceChanged()));
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
void ResourceModel::select(const QRegExp &regex)
{
    beginResetModel();
    foreach (auto item, m_items) {
        item->setSelected(!regex.isEmpty() && regex.indexIn(item->url(), 0) != -1);
    }
    endResetModel();
    emit selectionChanged();
}

/******************************************************************************
 ******************************************************************************/
void ResourceModel::onResourceChanged()
{
    // just ensure dataChanged() too;
    QModelIndex topLeft = this->index(0, 0);
    QModelIndex bottomRight = this->index(rowCount(), columnCount());
    emit dataChanged(topLeft, bottomRight);
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

    if (role == IsSelectedRole) {
        return item->isSelected();
    }

    if (role == Qt::DisplayRole) {
        const int col = index.column();
        if (col == 0) {
            return QVariant();
        }
        if (col == 1) {
            return item->url();
        }
        if (col == 2) {
            return item->customFileName();
        }
        if (col == 3) {
            return item->description();
        }
        if (col == 4) {
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
    if (index.column() == 0 && role == IsSelectedRole) {
        const bool selected = value.toBool();

        auto item = m_items.at(index.row());
        item->setSelected(selected);

        emit selectionChanged();

        QModelIndex topLeft = index;
        QModelIndex bottomRight = this->index(index.row(), columnCount());
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
        }
        if (section == 1) {
            return tr("Download");
        }
        if (section == 2) {
            return tr("Resource Name");
        }
        if (section == 3) {
            return tr("Description");
        }
        if (section == 4) {
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
