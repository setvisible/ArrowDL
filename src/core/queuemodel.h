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

#ifndef CORE_QUEUE_MODEL_H
#define CORE_QUEUE_MODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

class AbstractDownloadItem;
class ResourceItem;
class Settings;

class QueueModel: public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Role {
        StateRole = Qt::UserRole + 1,
        ProgressRole,
        CopyToClipboardRole,
        DownloadItemRole
    };

    explicit QueueModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                  const QModelIndex &destinationParent, int destinationChild) override;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    // API to add/remove items
    QList<AbstractDownloadItem *> items() const;
    void append(const QList<AbstractDownloadItem*> &items);

protected slots:
    void onItemChanged();

private:
    QList<AbstractDownloadItem *> m_items = {};

    QList<AbstractDownloadItem *> m_selectedItems = {};
    bool m_selectionAboutToChange = false;

    inline QString fileSize(const AbstractDownloadItem *item) const;
    inline QString estimatedTime(const AbstractDownloadItem *item) const;

    void connectItem(const AbstractDownloadItem *item);
    void disconnectItem(const AbstractDownloadItem *item);
};


#endif // CORE_QUEUE_MODEL_H
