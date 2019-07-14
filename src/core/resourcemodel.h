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

#ifndef CORE_RESOURCE_MODEL_H
#define CORE_RESOURCE_MODEL_H

#include <QtCore/QAbstractTableModel>

class ResourceItem;

class ResourceModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ResourceModel(QObject *parent);
    ~ResourceModel();

    void clear();

    void addResource(ResourceItem *item);
    QList<ResourceItem*> resourceItems() const;

    QList<ResourceItem*> selectedResourceItems() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

public slots:
    void setDestination(const QString &destination);
    void setMask(const QString &mask);

signals:
    void resourceChanged();

private:
    QList<ResourceItem*> m_items;
};

#endif // CORE_RESOURCE_MODEL_H
