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

#ifndef WIDGETS_LINK_WIDGET_H
#define WIDGETS_LINK_WIDGET_H

#include <QtWidgets/QWidget>

class Model;
class CheckableTableView;

class QMenu;

namespace Ui {
class LinkWidget;
}

class LinkWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LinkWidget(QWidget *parent);
    ~LinkWidget() Q_DECL_OVERRIDE;

    Model* model() const;
    void setModel(Model *model);

    QList<int> columnWidths() const;
    void setColumnWidths(const QList<int> &widths);

    void contextMenuCallback(QMenu *contextMenu);

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onCurrentTabChanged(int index);
    void onResourceChanged();
    void onSectionResized(int logicalIndex, int oldSize, int newSize);

    void customizeMask();
    void copyLinks();
    void open();

private:
    Ui::LinkWidget *ui;
    Model *m_model;

    void setup(CheckableTableView *view);
    void resizeSection(CheckableTableView *view, int logicalIndex, int newSize);

    inline QString textForOpenAction() const;
    inline CheckableTableView *currentTableView() const;
};

#endif // WIDGETS_LINK_WIDGET_H
