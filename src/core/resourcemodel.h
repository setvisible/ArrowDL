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

#ifndef CORE_RESOURCE_MODEL_H
#define CORE_RESOURCE_MODEL_H

#include <Core/CheckableTableModel>

class ResourceItem;

class ResourceModel : public CheckableTableModel
{
    Q_OBJECT

public:
    explicit ResourceModel(QObject *parent);
    ~ResourceModel() Q_DECL_OVERRIDE = default;

    void clear() Q_DECL_OVERRIDE;

    QList<ResourceItem*> items() const;
    void add(ResourceItem *item);

    QList<ResourceItem*> selection() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    void retranslateUi();

signals:
    void resourceChanged();
    void selectionChanged();

public slots:
    void setDestination(const QString &destination);
    void setMask(const QString &mask);
    void select(const QRegularExpression &regex);

private slots:
    void onCheckStateChanged(QModelIndex index, bool checked);
    void onResourceChanged();

private:
    QStringList m_headers;
    QList<ResourceItem*> m_items;
};

#endif // CORE_RESOURCE_MODEL_H
