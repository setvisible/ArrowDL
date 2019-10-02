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

#ifndef WIDGETS_COMBOBOX_H
#define WIDGETS_COMBOBOX_H

#include <QtWidgets/QComboBox>

typedef bool (*ColorizePtr)(QString);   /* function pointer */

class ComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit ComboBox(QWidget *parent = Q_NULLPTR);
    ~ComboBox();

    QStringList history() const;
    void setHistory(const QStringList &paths);

    void removePathfromHistory(const QString &path);

    QString currentText() const;

    void setColorizeErrorWhen(ColorizePtr functor);

public slots:
    void setStyleSheet(const QString& styleSheet);
    void setCurrentText(const QString &text);
    void clearHistory();

private slots:
    void onCurrentTextChanged(const QString &text);
    void showContextMenu(const QPoint &pos);

private:
    ColorizePtr m_colorizePtr;

    inline void colorizeErrors(const QString &text);
};

#endif // WIDGETS_COMBOBOX_H
