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

#include "format.h"

#include <QtCore/QDebug>
#include <QtCore/QTime>

/*!
 * \brief Returns a string formatting the given time.
 */
QString Format::remaingTimeToString(QTime time)
{
    if (!time.isValid()) {
        return QLatin1String("--:--");
    }
    if (time < QTime(0, 0, 1, 0)) {
        return time.toString("00:01");
    }
    if (time < QTime(1, 0, 0, 0)) {
        return time.toString("mm:ss");
    }
    if (time < QTime(24, 0, 0, 0)) {
        return time.toString("hh:mm:ss");
    }
    return QLatin1String("--:--");
}

/*!
 * \brief Returns a string formatting the given size, in bytes.
 */
QString Format::fileSizeToString(qint64 size)
{
    if (size < 0 || size >= INT64_MAX) {
        return QObject::tr("Unknown");
    }
    if (size == 0) {
        return QObject::tr("0 bytes");
    }
    if (size == 1) {
        return "1 byte";
    }
    if (size < 1000) {
        return QString::number(size) + " bytes";
    }
    double correctSize = size / 1024.0; // KB
    if (correctSize < 1000) {
        return QString::number(correctSize > 0 ? correctSize : 0, 'f', 0) + " KB";
    }
    correctSize /= 1024; // MB
    if (correctSize < 1000) {
        return QString::number(correctSize, 'f', 1) + " MB";
    }
    correctSize /= 1024; // GB
    if (correctSize < 1000) {
        return QString::number(correctSize, 'f', 2) + " GB";
    }
    correctSize /= 1024; // TB
    return QString::number(correctSize, 'f', 3) + " TB";
}

/*!
 * \brief Returns a string formatting the given speed, in bytes per second.
 */
QString Format::currentSpeedToString(qreal speed)
{
    if (speed < 0 || !qIsFinite(speed)) {
        return QLatin1String("-");
    }
    speed /= 1024.0; // KB
        if (speed < 1000) {
        return QString::number(speed > 0.5 ? speed : 0, 'f', 0) + " KB/s";
    }
    speed /= 1024; // MB
    if (speed < 1000) {
        return QString::number(speed, 'f', 1) + " MB/s";
    }
    speed /= 1024; // GB
    return QString::number(speed, 'f', 2) + " GB/s";
}

