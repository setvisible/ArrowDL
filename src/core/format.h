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

#ifndef CORE_FORMAT_H
#define CORE_FORMAT_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTime>

/*
 * Do not use namespace here, to enable tr("...") perimeter.
 */
class Format : public QObject
{
    Q_OBJECT
public:
    static QString infinity();

    static QString timeToString(const QTime time);
    static QString timeToString(const qint64 seconds);

    static QString currentSpeedToString(qreal speed, bool showInfiniteSymbol = false);
    static QString fileSizeToString(qsizetype size);
    static QString fileSizeThousandSeparator(qsizetype size);

    static QString yesOrNo(bool yes);

    static qreal parsePercentDecimal(const QString &text);
    static qsizetype parseBytes(const QString &text);

    static QString toHtmlMark(const QUrl &url, bool wrap = false);
    static QString wrapText(const QString &text, int blockLength = 50);

    static QString boolToHtml(bool value);
    static QString sizeToHtml(int size);
    static QString markDownToHtml(const QString &markdown);

};

#endif // CORE_FORMAT_H
