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

#ifndef CORE_CHECKABLE_TABLE_MODEL_H
#define CORE_CHECKABLE_TABLE_MODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QSet>

class CheckableTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum {
        CheckStateRole = Qt::UserRole + 1
    };

    explicit CheckableTableModel(QObject *parent);
    ~CheckableTableModel() Q_DECL_OVERRIDE = default;

    virtual void clear();

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    // Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

signals:
    void checkStateChanged();

protected:
    QList<int> checkedRows() const;
    QSet<QModelIndex> checkedIndexes() const;

private:
    QSet<QModelIndex> m_checkedIndexes;
};

#endif // CORE_CHECKABLE_TABLE_MODEL_H
