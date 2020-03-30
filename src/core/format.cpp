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

#include <QtCore/QtMath>
#include <QtCore/QDebug>
#include <QtCore/QTime>

/******************************************************************************
 ******************************************************************************/
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

/******************************************************************************
 ******************************************************************************/
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

/******************************************************************************
 ******************************************************************************/
/*!
 * \code
 * thisMethod(123456789); // returns "123,456,789"
 * \endcode
 */
QString Format::fileSizeThousandSeparator(qint64 size)
{
    auto number = QString::number(size);
    int i = number.count();
    while (i > 3) {
        i -= 3;
        number.insert(i, QLatin1Char(','));
    }
    return number;
}

/******************************************************************************
 ******************************************************************************/
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


/******************************************************************************
 ******************************************************************************/
static bool parseDouble(const QString &str, double &p)
{
    bool ok = false;
    auto val = str.toDouble(&ok);
    if (ok) {
        p = val;
    }
    return ok;
}

double Format::parsePercentDecimal(const QString &text)
{
    if (!text.contains('%')) {
        return -1.0;
    }
    QString percentString = text;
    percentString.remove('%');
    double percent = 0;
    if (!parseDouble(percentString, percent)) {
        return -1.0;
    }
    return percent;
}

/******************************************************************************
 ******************************************************************************/
qint64 Format::parseBytes(const QString &text)
{
    QString textwithoutTilde = text;
    textwithoutTilde.remove(QChar('~'));

    QString numberString = textwithoutTilde;
    numberString.remove(QRegExp("[a-zA-Z]*"));
    qreal decimal = 0;
    if (!parseDouble(numberString, decimal)) {
        return -1;
    }

    QString unitString = textwithoutTilde;
    unitString.remove(numberString);
    unitString = unitString.toUpper();

    qreal multiple = 0;
    if ( unitString == QLatin1String("B") ||
         unitString == QLatin1String("BYTE") ||
         unitString == QLatin1String("BYTES")) {
        multiple = 1;

    } else if (unitString == QLatin1String("KB")) {
        multiple = 1000;

    } else if (unitString == QLatin1String("MB")) {
        multiple = 1000000;

    } else if (unitString == QLatin1String("GB")) {
        multiple = 1000000000;

    } else if (unitString == QLatin1String("KIB")) {
        multiple = 1024;  // 1 KiB = 2^10 bytes = 1024 bytes

    } else if (unitString == QLatin1String("MIB")) {
        multiple = 1048576;  // 1 MiB = 2^20 bytes = 1048576 bytes

    } else if (unitString == QLatin1String("GIB")) {
        multiple = 1073741824;  // 1 GiB = 2^30 bytes = 1073741824 bytes

    } else {
        return -1;
    }
    /*
     * Remark: Can't use the QtMath's qCeil() here,
     * because returns "int" instead of "qint64",
     * causing erronous cast for big number.
     *
     * For 32-bit integer:
     *    -2^31            < x <    2^31-1
     *    -2,147,483,648   < x <    2,147,483,647
     *    -2.0 GiB         < x <    1.999 GiB
     *
     * => Issue when the filesize is greater than 1.999 GiB...
     */
    // qint64 bytes = qCeil(decimal * multiple);
    qint64 bytes = static_cast<qint64>(std::ceil(decimal * multiple));
    return bytes;
}
