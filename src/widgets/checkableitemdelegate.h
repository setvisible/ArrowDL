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

#ifndef WIDGETS_CHECKABLE_ITEM_DELEGATE_H
#define WIDGETS_CHECKABLE_ITEM_DELEGATE_H

#include <QtWidgets/QStyledItemDelegate>

class CheckableItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CheckableItemDelegate(QObject *parent = Q_NULLPTR);

    // painting
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    static int widthHint();
    static int thumbnailHint();

    // editing
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) Q_DECL_OVERRIDE;

private:
    QIcon m_checkIcon;
};


#endif // WIDGETS_CHECKABLE_ITEM_DELEGATE_H
