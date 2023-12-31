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

#ifndef WIDGETS_THEME_WIDGET_H
#define WIDGETS_THEME_WIDGET_H

#include <QtWidgets/QWidget>

namespace Ui {
class ThemeWidget;
}

class ThemeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ThemeWidget(QWidget *parent = nullptr);
    ~ThemeWidget() override;

    QMap<QString, QVariant> theme() const;
    void setTheme(const QMap<QString, QVariant> &map);

signals:
    void changed();

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void comboboxChanged(int value);

private:
    Ui::ThemeWidget *ui = nullptr;
    void retranslateComboBox();
};

#endif // WIDGETS_THEME_WIDGET_H
