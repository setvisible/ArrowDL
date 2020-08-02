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

#ifndef WIDGETS_MASK_WIDGET_H
#define WIDGETS_MASK_WIDGET_H

#include <QtWidgets/QWidget>

namespace Ui {
class MaskWidget;
}

class MaskWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MaskWidget(QWidget *parent);
    ~MaskWidget() Q_DECL_OVERRIDE;

    QString currentMask() const;
    void setCurrentMask(const QString &text);

signals:
    void currentMaskChanged(QString mask);

private slots:
    void onCurrentTextChanged(const QString &text);
    void onTipButtonReleased();
    void onTipButtonLinkActivated(const QString& link);

private:
    Ui::MaskWidget *ui;
};

#endif // WIDGETS_MASK_WIDGET_H
