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

#ifndef WIDGETS_CUSTOM_STYLE_H
#define WIDGETS_CUSTOM_STYLE_H

#include <QtWidgets/QProxyStyle>
#include <QtGui/QPixmap>

class CustomStyle : public QProxyStyle
{
public:
    CustomStyle(QStyle *style = Q_NULLPTR);
    CustomStyle(const QString &key);
    ~CustomStyle() override = default;

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = Q_NULLPTR) const override;

private:
    QImage m_textureImage;
};

#endif // WIDGETS_CUSTOM_STYLE_H
