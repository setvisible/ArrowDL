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

#include "checkableitemdelegate.h"

#include <Constants>
#include <Core/CheckableTableModel>
#include <Widgets/Globals>

#include <QtCore/QDebug>
#include <QtCore/QModelIndex>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtWidgets/QApplication>



static QModelIndex getSiblingAtColumn(const QModelIndex &index, int acolumn)
{
    return index.siblingAtColumn(acolumn);
}

/*!
 * CheckableItemDelegate is used to draw customized check-state icons.
 */
CheckableItemDelegate::CheckableItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    m_checkIcon.addPixmap(QIcon::fromTheme("check-ok").pixmap(QSize(16, 16)), QIcon::Normal, QIcon::On);
    m_checkIcon.addPixmap(QIcon::fromTheme("check-progress").pixmap(QSize(16, 16)), QIcon::Disabled, QIcon::On);
}

/******************************************************************************
 ******************************************************************************/
void CheckableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    initStyleOption(&myOption, index);

    auto palette = qApp->palette();
    myOption.palette.setColor(QPalette::All, QPalette::Window, palette.color(QPalette::Base));
    myOption.palette.setColor(QPalette::All, QPalette::WindowText, palette.color(QPalette::WindowText));
    myOption.palette.setColor(QPalette::All, QPalette::Highlight, palette.color(QPalette::Highlight));
    myOption.palette.setColor(QPalette::All, QPalette::HighlightedText, palette.color(QPalette::HighlightedText));

    auto sibling = getSiblingAtColumn(index, 0);
    auto checked = index.model()->data(sibling, CheckableTableModel::CheckStateRole).toBool();
    if (checked) {
        auto bgColor = myOption.palette.color(QPalette::Active, QPalette::Window);
        auto isDarkMode = bgColor.lightness() < 128;

        auto lightYellow = isDarkMode ? s_dark_lightYellow : s_lightYellow;
        auto darkYellow = isDarkMode ? s_dark_darkYellow : s_darkYellow;

        myOption.palette.setColor(QPalette::All, QPalette::Window, lightYellow);
        myOption.palette.setColor(QPalette::All, QPalette::Highlight, darkYellow);
        painter->fillRect(myOption.rect, lightYellow); // hack
    }

    if (myOption.state & QStyle::State_Selected) {
        myOption.font.setBold(true);
    }

    /* Inactive palette keep same colors as Active */
    myOption.palette.setColor(QPalette::Inactive, QPalette::Base, myOption.palette.color(QPalette::Active, QPalette::Base));
    myOption.palette.setColor(QPalette::Inactive, QPalette::Highlight, myOption.palette.color(QPalette::Active, QPalette::Highlight));
    myOption.palette.setColor(QPalette::Inactive, QPalette::HighlightedText, myOption.palette.color(QPalette::Active, QPalette::HighlightedText));
    myOption.palette.setColor(QPalette::Inactive, QPalette::Text, myOption.palette.color(QPalette::Active, QPalette::Text));

    if (index.column() == 0) {
        QStyleOptionButton button;
        button.rect = myOption.rect;
        button.palette = myOption.palette;
        button.iconSize = QSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
        button.icon = m_checkIcon;
        button.features |= QStyleOptionButton::Flat;
        button.state |= checked ? QStyle::State_Enabled : QStyle::State_None; // hack
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
    return CHECKBOX_WIDTH;
}

int CheckableItemDelegate::thumbnailHint()
{
    return THUMBNAIL_WIDTH;
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
