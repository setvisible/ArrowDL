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

#include "checkableitemdelegate.h"

#include <Core/CheckableTableModel>
#include <Widgets/Globals>

#include <QtCore/QDebug>
#include <QtCore/QModelIndex>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtWidgets/QApplication>

#define C_CHECKBOX_SIZE            12
#define C_CHECKBOX_WIDTH           16
#define C_THUMBNAIL_WIDTH          16


static QModelIndex getSiblingAtColumn(const QModelIndex &index, int acolumn)
{
#if QT_VERSION >= 0x051100
    return index.siblingAtColumn(acolumn);
#else
    return index.sibling(index.row(), acolumn);
#endif
}

/*!
 * CheckableItemDelegate is used to draw customized check-state icons.
 */
CheckableItemDelegate::CheckableItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    m_checkIcon.addPixmap(QPixmap(":/icons/menu/check_ok_16x16.png"), QIcon::Normal, QIcon::On);
    m_checkIcon.addPixmap(QPixmap(":/icons/menu/check_progress_16x16.png"), QIcon::Disabled, QIcon::On);
}

/******************************************************************************
 ******************************************************************************/
void CheckableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    initStyleOption(&myOption, index);

    auto sibling = getSiblingAtColumn(index, 0);
    const bool selected = index.model()->data(sibling, CheckableTableModel::CheckStateRole).toBool();

    if (selected) {
        painter->fillRect(option.rect, s_lightYellow);
        myOption.palette.setColor(QPalette::All, QPalette::Highlight, s_darkYellow);
    } else {
        myOption.palette.setColor(QPalette::All, QPalette::Highlight, s_lightBlue);
    }

    if (myOption.state & QStyle::State_Selected) {
        myOption.font.setBold(true);
    }

    if (index.column() == 0) {
        QStyleOptionButton button;
        button.rect = myOption.rect;
        button.palette = myOption.palette;
        button.iconSize = QSize(C_CHECKBOX_SIZE, C_CHECKBOX_SIZE);
        button.icon = m_checkIcon;
        button.features |= QStyleOptionButton::Flat;
        button.state |= selected ? QStyle::State_Enabled : QStyle::State_None;
        QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
    } else {
        QStyledItemDelegate::paint(painter, myOption, index);
    }
}

/******************************************************************************
 ******************************************************************************/
QSize CheckableItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

int CheckableItemDelegate::widthHint()
{
    return C_CHECKBOX_WIDTH;
}

int CheckableItemDelegate::thumbnailHint()
{
    return C_THUMBNAIL_WIDTH;
}

/******************************************************************************
 ******************************************************************************/
bool CheckableItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress && index.column() == 0) {
        const bool selected = index.model()->data(index, CheckableTableModel::CheckStateRole).toBool();
        model->setData(index, !selected, CheckableTableModel::CheckStateRole);
        return true;
    }
    return QStyledItemDelegate::editorEvent(event,model,option, index);
}
