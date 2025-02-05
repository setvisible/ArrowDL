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

#include "mask.h"

#include <Constants>

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QUrl>

using namespace Qt::Literals::StringLiterals;


static const QStringList s_tags
{
    NAME        ,
    EXT         ,
    URL         ,
    CURL        ,
    FLATURL     ,
    SUBDIRS     ,
    FLATSUBDIRS ,
    QSTRING
};


static QString decodePercentEncoding(const QString &input)
{
    /*
     * Replace Percent-encoded characters to UTF-8
     * when the URL is misformed and contains a mix
     * of ASCII and UTF-8 encoding.
     */
    auto decoded = input;
    decoded.replace("%0A", "\r");
    decoded.replace("%0D", "\n");
    decoded.replace("%20", " ");
    decoded.replace("%22", "\"");
    decoded.replace("%25", "%");
    decoded.replace("%2D", "-");
    decoded.replace("%2E", ".");
    decoded.replace("%3C", "<");
    decoded.replace("%3E", ">");
    decoded.replace("%5C", "\\");
    decoded.replace("%5E", "^");
    decoded.replace("%5F", "_");
    decoded.replace("%60", "`");
    decoded.replace("%7B", "{");
    decoded.replace("%7C", "|");
    decoded.replace("%7D", "}");
    decoded.replace("%7E", "~");
    return decoded;
}

QString Mask::decodeMagnetEncoding(const QString &s)
{
    /*
     * See  std::string unescape_string(string_view s, error_code& ec)
     * in
     * libtorrent/src/escape_string.cpp
     */
    QString ret;
    for (auto i = s.begin(); i != s.end(); ++i) {
        if (*i == '+') {
            ret += ' ';
        } else if (*i != '%') {
            ret += *i;
        } else {
            ++i;
            if (i == s.end()) {
                // ec = errors::invalid_escaped_string;
                return ret;
            }

            int high = 0;
            if (*i >= '0' && *i <= '9') high = (*i).toLatin1() - '0';
            else if (*i >= 'A' && *i <= 'F') high = (*i).toLatin1() + 10 - 'A';
            else if (*i >= 'a' && *i <= 'f') high = (*i).toLatin1() + 10 - 'a';
            else {
                // ec = errors::invalid_escaped_string;
                return ret;
            }

            ++i;
            if (i == s.end()) {
                // ec = errors::invalid_escaped_string;
                return ret;
            }

            int low = 0;
            if(*i >= '0' && *i <= '9') low = (*i).toLatin1() - '0';
            else if(*i >= 'A' && *i <= 'F') low = (*i).toLatin1() + 10 - 'A';
            else if(*i >= 'a' && *i <= 'f') low = (*i).toLatin1() + 10 - 'a';
            else {
                // ec = errors::invalid_escaped_string;
                return ret;
            }

            ret += char(high * 16 + low);
        }
    }
    return ret;
}


/******************************************************************************
 ******************************************************************************/
QUrl Mask::fromUserInput(const QString &input)
{
    auto cleaned = decodePercentEncoding(input);
    cleaned = cleaned.trimmed();
    auto url = QUrl::fromUserInput(cleaned);
    if (url.isEmpty()) {
        url = QUrl::toPercentEncoding(cleaned);
    }

    /*
     * REMARK
     * When the input has no query and no fragment, replace:
     * - "?" with "%3F"
     * - "#" with "%23"
     * in order to treat these characters as literal.
     *
     * Examples:
     * - Query:           myfile.txt?id=123&t=abc
     * - Fragment:        faq.html?#answer
     * - Not a query:     myfile_(?).txt
     * - Not a fragment:  myfile#.txt
     */
    if (url.hasQuery()) {
        auto query = url.query();
        if ( query.isEmpty() ||
             query.contains('=') ||
             query.contains('#') ||
             url.hasFragment()) {
            // valid query, just continue.
        } else {
            auto encoded = cleaned;
            encoded.replace("?", "%3F");
            encoded.replace("#", "%23");
            url = QUrl::fromUserInput(encoded);
            if (url.isEmpty()) {
                url = QUrl::toPercentEncoding(cleaned);
            }
        }
    } else {
        if (url.hasFragment()) {
            auto encoded = cleaned;
            encoded.replace("?", "%3F");
            encoded.replace("#", "%23");
            url = QUrl::fromUserInput(encoded);
            if (url.isEmpty()) {
                url = QUrl::toPercentEncoding(cleaned);
            }
        }
    }
    return url;
}

QString Mask::interpret(const QString &input, const QString &customFileName, const QString &mask)
{
    auto url = fromUserInput(input);
    return interpret(url, customFileName, mask);
}

QString Mask::interpret(const QUrl &url, const QString &customFileName, const QString &mask)
{
    if (!url.isValid()) {
        return {};
    }
    auto decodedMask = mask; //.trimmed();
    if (decodedMask.isEmpty()) {
        decodedMask = QString("%0/%1/%2.%3").arg(URL, SUBDIRS, NAME, EXT);
    }

    auto host = url.host();
    auto path = url.path();
    auto filename = url.fileName();
    auto query = url.query();

    QFileInfo fi(filename);
    auto basename = fi.completeBaseName();
    auto suffix = fi.suffix();

    auto subdirs = path;
    subdirs.chop(filename.count());
    if (subdirs.startsWith(QChar('/'))) {
        subdirs.remove(0, 1);
    }
    if (subdirs.endsWith(QChar('/'))) {
        subdirs.chop(1);
    }

    auto fullUrl = host + path;

    auto flatUrl = fullUrl;
    flatUrl.replace(QChar('/'), QChar('-'));

    auto flatSubdirs = subdirs;
    flatSubdirs.replace(QChar('/'), QChar('-'));

    // Renaming Tags
    decodedMask.replace(QChar('\\'), QChar('/'));

    // BUGFIX
    if (!customFileName.isEmpty()) {
        QString temp = "";
        auto parts = decodedMask.split(QChar('/'));
        for (QString part : parts) {
            if (part.contains(NAME)) {
                if (part.contains(EXT)) {
                    part = customFileName;
                    part.append(".").append(suffix);
                } else {
                    part = customFileName;
                }
            }
            temp.append(part).append(QChar('/'));
        }
        temp.removeLast();
        decodedMask = temp;
    }

    decodedMask.replace( NAME         , basename      );
    decodedMask.replace( EXT          , suffix        );
    decodedMask.replace( URL          , host          );
    decodedMask.replace( CURL         , fullUrl       );
    decodedMask.replace( FLATURL      , flatUrl       );
    decodedMask.replace( SUBDIRS      , subdirs       );
    decodedMask.replace( FLATSUBDIRS  , flatSubdirs   );
    decodedMask.replace( QSTRING      , query         );

    /* Remove the trailing '.' and duplicated '/' */
    static QRegularExpression reDuplicateSlash("/+");
    static QRegularExpression reLeadingSlash("^/");
    static QRegularExpression reTrailingSlashAndDot("[/\\.]*$");
    decodedMask.replace(reDuplicateSlash, "/");
    decodedMask.replace(reLeadingSlash, "");
    decodedMask.replace(reTrailingSlashAndDot, "");

    /* Replace reserved characters */
    cleanNameForWindows(decodedMask);

    return decodedMask;
}


/******************************************************************************
 ******************************************************************************/
/*!
 * List of all the available tags.
 */
QStringList Mask::tags()
{
    return s_tags;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Description of the given tag
 */
QString Mask::description(const QString &tag)
{
    if (tag == NAME        ) {return tr("File name");}
    if (tag == EXT         ) {return tr("Extension");}
    if (tag == URL         ) {return tr("Base URL");}
    if (tag == CURL        ) {return tr("Full URL");}
    if (tag == FLATURL     ) {return tr("Flat full URL");}
    if (tag == SUBDIRS     ) {return tr("URL subdirectories");}
    if (tag == FLATSUBDIRS ) {return tr("Flat URL subdirectories");}
    if (tag == QSTRING     ) {return tr("Query string");}
    return {};
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Replace reserved characters by '_' w.r.t to most strict standard (Windows).
 *
 * \remark Recommended naming for Windows:
 * https://docs.microsoft.com/fr-fr/windows/win32/fileio/naming-a-file?redirectedfrom=MSDN
 */
void Mask::cleanNameForWindows(QString &input)
{
    /* Chars must be part of ANSI charset (ASCII + extended 128-255) (0x00-0xFF)  */

    /* Replace reserved characters */
    input.replace(QRegularExpression("[<>:\"|?#*]"), "_");

    /* and more */
    input.replace(QRegularExpression("[#]"), "_");
}
