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

#ifndef WIDGETS_FILTER_WIDGET_H
#define WIDGETS_FILTER_WIDGET_H

#include <QtWidgets/QWidget>

namespace Ui {
class FilterWidget;
}

class FilterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FilterWidget(QWidget *parent);
    ~FilterWidget();

    uint state() const;
    void setState(uint code);

    QString text() const;
    void setText(const QString &text);

signals:
    void filterChanged(QString text);

private slots:
    void onFilterChanged(int);
    void onFilterChanged(const QString &);

private:
    Ui::FilterWidget *ui;

    QString filter() const;
};

#endif // WIDGETS_FILTER_WIDGET_H
