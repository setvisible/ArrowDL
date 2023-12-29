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

#ifndef WIDGETS_FILTER_TIP_H
#define WIDGETS_FILTER_TIP_H

#include <QtWidgets/QFrame>


namespace Ui {
class FilterTip;
}

class FilterTip : public QFrame
{
    Q_OBJECT

public:
    explicit FilterTip(QWidget *parent);
    ~FilterTip() override;

    void add(const QString &text, const QString &link);

signals:
    void linkActivated(const QString& link);

private slots:
    void onLinkActivated(const QString& link);

private:
    Ui::FilterTip *ui;

};

#endif // WIDGETS_FILTER_TIP_H
