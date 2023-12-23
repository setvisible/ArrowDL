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

#include "resourcemodel.h"

#include <Core/ResourceItem>

#include <QtCore/QRegularExpression>

using namespace Qt::Literals::StringLiterals;

ResourceModel::ResourceModel(QObject *parent) : CheckableTableModel(parent)
{
    connect(this, SIGNAL(checkStateChanged(QModelIndex , bool)), this, SLOT(onCheckStateChanged(QModelIndex , bool)));
    connect(this, SIGNAL(resourceChanged()), this, SLOT(onResourceChanged()));

    retranslateUi();
}

/******************************************************************************
 ******************************************************************************/
void ResourceModel::clear()
{
    beginResetModel();
    CheckableTableModel::clear();
    m_items.clear();
    endResetModel();
    emit resourceChanged();
}

/******************************************************************************
 ******************************************************************************/
QList<ResourceItem*> ResourceModel::items() const
{
    return m_items;
}

void ResourceModel::add(ResourceItem *item)
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

QList<ResourceItem*> ResourceModel::selection() const
{
    QList<ResourceItem*> selection;
    foreach (int row, this->checkedRows()) {
        if (row >= 0 && row < m_items.count()) {
            selection << m_items.at(row);
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
void ResourceModel::select(const QRegularExpression &regex)
{
    beginResetModel();
    QSignalBlocker blocker(this);
    for (int i = 0; i < m_items.count(); ++i) {
        auto item = m_items.at(i);
        auto url = item->url();
        auto isChecked = (regex.isValid() && regex.match(url).hasMatch());
        this->setData(this->index(i, 0), isChecked, CheckableTableModel::CheckStateRole);
    }
    blocker.unblock();
    endResetModel();
    emit selectionChanged();
}

/******************************************************************************
 ******************************************************************************/
void ResourceModel::onCheckStateChanged(QModelIndex /*index*/, bool /*checked*/)
{
    emit selectionChanged();
}

void ResourceModel::onResourceChanged()
{
    // just ensure dataChanged() too;
    QModelIndex topLeft = this->index(0, 0);
    QModelIndex bottomRight = this->index(rowCount(), columnCount());
    emit dataChanged(topLeft, bottomRight);
}

/******************************************************************************
 ******************************************************************************/
void ResourceModel::retranslateUi()
{
    m_headers = QStringList()
            << ""_L1 // checkbox
            << tr("Download")
            << tr("Resource Name")
            << tr("Description")
            << tr("Mask");
}

/******************************************************************************
 ******************************************************************************/
int ResourceModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_headers.count();
}

QVariant ResourceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section >= 0 && section < m_headers.count()) {
            return m_headers.at(section);
        }
        return QVariant();
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

int ResourceModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.count();
}

QVariant ResourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    ResourceItem *item = m_items.at(index.row());
    if (role == Qt::DisplayRole) {
        switch ( index.column()) {
        case 0: return QVariant();
        case 1: return item->url();
        case 2: return item->customFileName();
        case 3: return item->description();
        case 4: return item->mask();
        default:
            break;
        }
    }
    return CheckableTableModel::data(index, role);
}
