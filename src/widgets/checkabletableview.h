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

#ifndef WIDGETS_CHECKABLE_TABLE_VIEW_H
#define WIDGETS_CHECKABLE_TABLE_VIEW_H

#include <QtWidgets/QTableView>

#include <functional> /* std::function */

class QAction;

class CheckableTableView : public QTableView
{
    Q_OBJECT
public:
    explicit CheckableTableView(QWidget *parent);
    ~CheckableTableView() Q_DECL_OVERRIDE;

    void setContextMenuCallback(std::function<void (QMenu*)> callback);

    QList<int> columnWidths() const;
    void setColumnWidths(const QList<int> &widths);

    QModelIndexList selectedIndexesAtColumn(int column);

public slots:
    void checkSelected();
    void uncheckSelected();
    void toggleCheck();
    void selectFiltered();
    void invertSelection();

private slots:
    void onSectionCountChanged(int oldCount, int newCount);
    void showContextMenu(const QPoint &pos);

private:
    std::function<void (QMenu*)> m_contextMenuCallback;
};

#endif // WIDGETS_CHECKABLE_TABLE_VIEW_H
