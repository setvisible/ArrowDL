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

#ifndef WIDGETS_TEXT_EDITOR_WIDGET_H
#define WIDGETS_TEXT_EDITOR_WIDGET_H

#include <QtWidgets/QWidget>

namespace Ui {
class TextEditorWidget;
}

class TextEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TextEditorWidget(QWidget *parent = nullptr);
    ~TextEditorWidget() override;

    void clear();
    void append(const QString &text);
    int count() const;
    QString at(int lineNumber) const;

    bool isModified();

signals:
    void textChanged();

public slots:
    void setModified(bool modified);

private slots:
    void onBlockModeToggled(bool checked);

private:
    Ui::TextEditorWidget *ui = nullptr;
    void propagateIcons();
};

#endif // WIDGETS_TEXT_EDITOR_WIDGET_H
