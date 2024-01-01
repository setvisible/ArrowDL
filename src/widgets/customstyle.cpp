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

#include "customstyle.h"

#include <Constants>
#include <Core/IDownloadItem>
#include <Widgets/CustomStyleOptionProgressBar>
#include <Widgets/Globals>

#include <QtCore/QtMath>
#include <QtCore/QBitArray>
#include <QtGui/QPainter>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QStyleOption>



/*!
 * \class CustomStyle
 * \brief The CustomStyle class wraps a QProxyStyle (the default system style)
 * for the purpose of dynamically overriding the application painting.
 *
 */

CustomStyle::CustomStyle(QStyle *style) : QProxyStyle(style)
{
    m_textureImage = QImage(s_xpm);
}

CustomStyle::CustomStyle(const QString &key)
    : CustomStyle(QStyleFactory::create(key))
{
}

void CustomStyle::drawControl(ControlElement element, const QStyleOption *opt,
                              QPainter *p, const QWidget *widget) const
{  
    switch (element) {
    case CE_ProgressBar:
        if (auto pb = qstyleoption_cast<const CustomStyleOptionProgressBar *>(opt)) {

            auto hasIcon = !pb->icon.isNull();

            QPalette::ColorGroup cg;
            QPalette::ColorRole cr;

            if ((pb->state & State_Enabled) == 0) {
                cg = QPalette::Disabled;
            } else if (pb->state & QStyle::State_Active) {
                cg = QPalette::Active;
            } else {
                cg = QPalette::Inactive;
            }

            if (pb->state & State_Selected) {
                cr = QPalette::Highlight;
            } else {
                cr = QPalette::Window;
            }

            /* Draw the selection background */
            auto bgColor = pb->palette.color(cg, cr);
            p->setPen(Qt::NoPen);
            p->setBrush(bgColor);
            p->drawRect(pb->rect);

            /* Draw the icon */
            if (hasIcon) {
                auto size = ICON_SIZE;
                auto margin = (qMax(size, pb->rect.height()) - size ) / 2;
                auto iconRect = QRect(pb->rect.x() + margin , pb->rect.y() + margin, size, size);
                p->drawPixmap(iconRect, pb->icon.pixmap(ICON_SIZE));
            }

            /* Draw the progress bar */
            {
                /* Draw the progress bar frame */
                const int marginV = 3;
                const int marginH = 5;
                QRect frameRect = pb->rect;
                frameRect.setTop(frameRect.top() + marginV);
                frameRect.setBottom(frameRect.bottom() - marginV);
                if (hasIcon) {
                    frameRect.setLeft(frameRect.left() + marginH + ICON_WIDTH);
                } else {
                    frameRect.setLeft(frameRect.left() + marginH);
                }
                frameRect.setRight(frameRect.right() - marginH);

                p->setPen(QPen(s_darkGrey, 1));
                p->setBrush(s_lightGrey);
                p->drawRect(frameRect);

                /* Draw the progress bar indicator */
                auto minimum = qint64(pb->minimum);
                auto maximum = qint64(pb->maximum);
                auto progress = qint64(pb->progress);

                auto color = pb->color;
                QBrush brush;

                const int margin = 2;
                auto indicatorRect = frameRect;
                indicatorRect.setLeft(indicatorRect.left() + margin);
                indicatorRect.setRight(indicatorRect.right() + 1 - margin);

                if (pb->hasSegments) {
                    const int indicatorBarHeight = 2;

                    // Top bar: Detailed segments
                    {
                        auto segmentRect = indicatorRect;
                        segmentRect.setTop(segmentRect.top() + margin);
                        segmentRect.setBottom(segmentRect.bottom() + 1 - margin - indicatorBarHeight);

                        auto segments = pb->segments;
                        auto size = segments.size();

                        QImage segmentImage(qMax(1, size), 1, QImage::Format_RGB32);
                        segmentImage.fill(s_lightGrey);
                        auto rgb = color.rgb();
                        for (auto i = 0; i < size; ++i) {
                            if (segments.testBit(i)) {
                                segmentImage.setPixel(i, 0, rgb);
                            }
                        }
                        auto segmentPixmap = QPixmap::fromImage(segmentImage);
                        auto scaled = segmentPixmap.scaled(
                                    segmentRect.size(),
                                    Qt::IgnoreAspectRatio,
                                    Qt::FastTransformation);

                        p->drawPixmap(segmentRect, scaled);
                    }

                    // Bottom bar: Progress indicator bar
                    auto bottom = indicatorRect.bottom() + 1 - margin;
                    indicatorRect.setTop(bottom - indicatorBarHeight);
                    indicatorRect.setBottom(bottom);

                } else {
                    indicatorRect.setTop(indicatorRect.top() + margin);
                    indicatorRect.setBottom(indicatorRect.bottom() + 1 - margin);
                }

                if (progress < 0 || (minimum == 0 && maximum == 0)) {
                    auto rgb = color.rgb();
                    auto textureImage = m_textureImage;
                    textureImage.setColor(1, rgb);
                    auto pixmap = QPixmap::fromImage(textureImage);
                    brush.setTexture(pixmap);

                } else {
                    auto p_v = qreal(progress - minimum);
                    auto t_s = (maximum - minimum) ? qreal(maximum - minimum) : qreal(1);
                    auto r = qreal(p_v / t_s);
                    auto w = static_cast<int>(qMax(qCeil(r * indicatorRect.width()), 1));
                    indicatorRect.setWidth(w);
                    brush.setStyle(Qt::SolidPattern);
                    brush.setColor(color);
                }
                p->setPen(Qt::NoPen);
                p->setBrush(brush);
                p->drawRect(indicatorRect);
            }

        } else {
            QProxyStyle::drawControl(element, opt, p, widget);
        }
        break;
    default:
        QProxyStyle::drawControl(element, opt, p, widget);
        break;
    }
}
