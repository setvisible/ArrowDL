/* - DownZemAll! - Copyright (C) 2019 Sebastien Vavassori
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
#include <QtWidgets/QStyledItemDelegate>

class Model;
class QTableView;

namespace Ui {
class LinkWidget;
}

class LinkWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LinkWidget(QWidget *parent);
    ~LinkWidget();

    Model* model() const;
    void setModel(Model *model);

protected:
    virtual void resizeEvent(QResizeEvent *event);

public slots:

private slots:
    void onCurrentTabChanged(int index);
    void onResourceChanged();

private:
    Ui::LinkWidget *ui;
    Model *m_model;

    void setup(QTableView *view);
    void resize();
    void resize(QTableView *view);
};

#endif // WIDGETS_LINK_WIDGET_H
