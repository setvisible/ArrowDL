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

#ifndef WIDGETS_QUEUE_WIDGET_ITEM_DELEGATE_H
#define WIDGETS_QUEUE_WIDGET_ITEM_DELEGATE_H

#include <Core/AbstractDownloadItem>

#include <QtWidgets/QStyledItemDelegate>

class QPainter;

/*!
 * QueueWidgetItemDelegate is used to draw the progress bars and edit the filename.
 */
class QueueWidgetItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit QueueWidgetItemDelegate(QObject *parent = nullptr);

    // painting
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    // editing
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

    void restylizeUi();

private:
    QIcon m_idleIcon = {};
    QIcon m_resumeIcon = {};
    QIcon m_pauseIcon = {};
    QIcon m_stopIcon = {};
    QIcon m_completedIcon = {};

    QColor stateColor(AbstractDownloadItem::State state) const;
    QIcon stateIcon(AbstractDownloadItem::State state) const;
};

#endif // WIDGETS_QUEUE_WIDGET_ITEM_DELEGATE_H
