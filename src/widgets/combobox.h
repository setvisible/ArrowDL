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

#ifndef WIDGETS_COMBOBOX_H
#define WIDGETS_COMBOBOX_H

#include <QtWidgets/QComboBox>

using InputValidityPtr = bool(*)(const QString &); /* function pointer */

class ComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit ComboBox(QWidget *parent = Q_NULLPTR);
    ~ComboBox() Q_DECL_OVERRIDE = default;

    QStringList history() const;
    void setHistory(const QStringList &history);

    void addToHistory(const QString &text);
    void removeFromHistory(const QString &text);

    QString currentText() const;

    bool isInputValid() const;
    void setInputIsValidWhen(InputValidityPtr functor);

public slots:
    void setStyleSheet(const QString& styleSheet);
    void setCurrentText(const QString &text);
    void clearHistory();

private slots:
    void onCurrentTextChanged(const QString &text);
    void onCurrentIndexChanged(int index);
    void showContextMenu(const QPoint &pos);

private:
    InputValidityPtr m_inputValidityPtr = Q_NULLPTR;

    inline void colorize();
};

#endif // WIDGETS_COMBOBOX_H
