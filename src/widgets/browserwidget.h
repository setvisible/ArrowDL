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

#ifndef WIDGETS_BROWSER_WIDGET_H
#define WIDGETS_BROWSER_WIDGET_H

#include <QtWidgets/QWidget>

namespace Ui {
class BrowserWidget;
}

class BrowserWidget : public QWidget
{
    Q_OBJECT

public:
    enum Type { File, Directory };

    explicit BrowserWidget(QWidget *parent = 0);
    ~BrowserWidget();

    QString text() const;
    void setText(const QString &text);

    Type type() const;
    void setType(Type type);

    QString extensionType() const;
    void setExtensionType(const QString &extensionType);

    QString extensionName() const;
    void setExtensionName(const QString &extensionName);

signals:
    void textChanged(QString text);

private slots:
    void onBrowseButtonReleased();
    void onCurrentTextChanged(const QString &text);

private:
    Ui::BrowserWidget *ui;
    Type m_type;
    QString m_extensionType;
    QString m_extensionName;
};

#endif // WIDGETS_BROWSER_WIDGET_H
