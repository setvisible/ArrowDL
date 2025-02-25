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

class AbstractJob;
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
        JobRole
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

    // API to add/remove jobs
    QList<AbstractJob *> jobs() const;
    void append(const QList<AbstractJob *> &jobs);

protected slots:
    void onJobChanged();

private:
    QList<AbstractJob *> m_jobs = {};

    inline QString fileSize(const AbstractJob *job) const;
    inline QString estimatedTime(const AbstractJob *job) const;

    void connectJob(const AbstractJob *job);
    void disconnectJob(const AbstractJob *job);
};

#endif // CORE_QUEUE_MODEL_H
