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

#include "format.h"

#include <Constants>

#include <QtCore/QtMath>
#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>
#include <QtCore/QTime>
#include <QtCore/QUrl>

using namespace Qt::Literals::StringLiterals;


QString Format::infinity()
{
    return SYMBOL_INFINITE;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Returns a string formatting the given time.
 */
QString Format::timeToString(const QTime time)
{
    if (!time.isValid()) {
        return "--:--"_L1;
    }
    if (time < QTime(0, 0, 1, 0)) {
        return time.toString("00:01"_L1);
    }
    if (time < QTime(1, 0, 0, 0)) {
        return time.toString("mm:ss"_L1);
    }
    return time.toString("hh:mm:ss"_L1);
}

QString Format::timeToString(const qint64 seconds)
{
    if (seconds < 0) {
        return "--:--"_L1;
    }
    if (seconds >= ONE_DAY_IN_SECONDS) { // More than one day
        return SYMBOL_INFINITE;
    }
    QTime time(0, 0, 0, 0);
    time = time.addSecs(static_cast<int>(seconds));
    return timeToString(time);
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Returns a string formatting the given size, in bytes.
 */
QString Format::fileSizeToString(qsizetype size)
{
    if (size < 0 || size >= PTRDIFF_MAX) {
        return tr("Unknown");
    }
    if (size == 0) {
        return tr("0 byte");
    }
    if (size == 1) {
        return tr("1 byte");
    }
    if (size < 1000) {
        return tr("%0 bytes").arg(QString::number(size));
    }
    qreal correctSize = qreal(size) / 1024; // KB
    if (correctSize < 1000) {
        return tr("%0 KB").arg(QString::number(correctSize > 0 ? correctSize : 0, 'f', 0));
    }
    correctSize /= 1024; // MB
    if (correctSize < 1000) {
        return tr("%0 MB").arg(QString::number(correctSize, 'f', 1));
    }
    correctSize /= 1024; // GB
    if (correctSize < 1000) {
        return tr("%0 GB").arg(QString::number(correctSize, 'f', 2));
    }
    correctSize /= 1024; // TB
    return tr("%0 TB").arg(QString::number(correctSize, 'f', 3));
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \code
 * fileSizeThousandSeparator(123456789); // returns "123,456,789"
 * \endcode
 */
QString Format::fileSizeThousandSeparator(qsizetype size)
{
    auto number = QString::number(size);
    auto i = number.count();
    while (i > 3) {
        i -= 3;
        number.insert(i, ","_L1);
    }
    return number;
}


/******************************************************************************
 ******************************************************************************/
QString Format::yesOrNo(bool yes)
{
    return yes ? tr("Yes") : tr("No");
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Returns a string formatting the given speed, in bytes per second.
 */
QString Format::currentSpeedToString(qreal speed, bool showInfiniteSymbol)
{
    if (speed < 0 || !qIsFinite(speed)) {
        return showInfiniteSymbol ? SYMBOL_INFINITE : QLatin1String("-");
    }
    speed /= 1024.0; // KB
    if (speed < 1000) {
        return tr("%0 KB/s").arg(QString::number(speed > 0.5 ? speed : 0, 'f', 0));
    }
    speed /= 1024; // MB
    if (speed < 1000) {
        return tr("%0 MB/s").arg(QString::number(speed, 'f', 1));
    }
    speed /= 1024; // GB
    if (speed < 1000) {
        return tr("%0 GB/s").arg(QString::number(speed, 'f', 2));
    }
    speed /= 1024; // TB
    return tr("%0 TB/s").arg(QString::number(speed, 'f', 2));
}


/******************************************************************************
 ******************************************************************************/
static bool parseDouble(const QString &str, qreal &p)
{
    bool ok = false;
    qreal val = str.toDouble(&ok);
    if (ok) {
        p = val;
    }
    return ok;
}

qreal Format::parsePercentDecimal(const QString &text)
{
    if (!text.contains('%')) {
        return -1.0;
    }
    QString percentString = text;
    percentString.remove('%');
    qreal percent = 0;
    if (!parseDouble(percentString, percent)) {
        return -1.0;
    }
    return percent;
}

/******************************************************************************
 ******************************************************************************/
qsizetype Format::parseBytes(const QString &text)
{
    QString textWithoutTilde = text;
    textWithoutTilde.remove('~'_L1);

    QString numberString = textWithoutTilde;

    static QRegularExpression reLetters("[a-zA-Z]*");
    numberString.remove(reLetters);
    qreal decimal = 0;
    if (!parseDouble(numberString, decimal)) {
        return -1;
    }

    QString unitString = textWithoutTilde;
    unitString.remove(numberString);
    unitString = unitString.toUpper();

    qreal multiple = 0;
    if ( unitString == "B"_L1 ||
         unitString == "BYTE"_L1 ||
         unitString == "BYTES"_L1) {
        multiple = 1;

    } else if (unitString == "KB"_L1) {
        multiple = 1000;

    } else if (unitString == "MB"_L1) {
        multiple = 1000000;

    } else if (unitString == "GB"_L1) {
        multiple = 1000000000;

    } else if (unitString == "TB"_L1) {
        multiple = 1000000000000;

    } else if (unitString == "KIB"_L1) {
        multiple = 1024;  // 1 KiB = 2^10 bytes = 1024 bytes

    } else if (unitString == "MIB"_L1) {
        multiple = 1048576;  // 1 MiB = 2^20 bytes = 1048576 bytes

    } else if (unitString == "GIB"_L1) {
        multiple = 1073741824;  // 1 GiB = 2^30 bytes = 1073741824 bytes

    } else if (unitString == "TIB"_L1) {
        multiple = 1099511627776;  // 1 TiB = 2^40 bytes = 1099511627776 bytes

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
    qsizetype bytes = static_cast<qsizetype>(decimal * multiple);
    return bytes;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Return the given \a url as a HTML mark, composed by
 * the hypertext link inside the anchor tag "<a></a>".
 *
 * If \a wrap is on, the displayed text of the HTML mark will contains
 * a whitespace every 100 characters, so that QLabel can word-wrap it.
 *
 * This is a work-around for QLabel that doesn't allow Qt::WrapAnywhere.
 */
QString Format::toHtmlMark(const QUrl &url, bool wrap)
{
    QString urlDisplay= url.toString();
    if (wrap) {
        urlDisplay = Format::wrapText(urlDisplay);
    }
    return QString("<a href=\"%0\">%1</a>").arg(url.toString(), urlDisplay);
}

/******************************************************************************
 ******************************************************************************/
QString Format::wrapText(const QString &text, int blockLength)
{
    QString out;
    int blockCounter = 0; // counter since the last whitespace
    for (auto ch : text) {
        out += ch;
        blockCounter++;
        if (ch.isSpace()) {
            blockCounter = 0;
            continue;
        }
        if (blockCounter < blockLength) {
            continue;
        }
        if (ch.isPunct() || ch.isSymbol()) {
            // Add some whitespaces to ease word wrap.
            out += QChar::Space;
            blockCounter = 0;
        }
    }
    return out.trimmed();
}

/******************************************************************************
 ******************************************************************************/
QString Format::boolToHtml(bool value)
{
    return value ? "True"_L1 : "False"_L1;
}

QString Format::sizeToHtml(int size)
{
    /// \todo Add non breaking space?
    return Format::fileSizeToString(size);
}

QString Format::markDownToHtml(const QString &markdown)
{
    /// \todo Replace this with QTextDocument::setMarkDown() introduced in Qt 5.14.
    QString html = markdown;
    html = html.replace("\n"_L1, "<br>\n"_L1);
    return html;
}

