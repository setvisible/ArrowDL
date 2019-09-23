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

#include "mask.h"

#include <QtCore/QUrl>
#include <QtCore/QFileInfo>
#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

static const QString NAME          = "*name*";
static const QString EXT           = "*ext*";
static const QString URL           = "*url*";
static const QString CURL          = "*curl*";
static const QString FLATURL       = "*flaturl*";
static const QString SUBDIRS       = "*subdirs*";
static const QString FLATSUBDIRS   = "*flatsubdirs*";
static const QString QSTRING       = "*qstring*";

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

QString Mask::interpret(const QUrl &url,
                        const QString &customFileName,
                        const QString &mask)
{
    if (!url.isValid()) {
        return QString();
    }
    QString decodedMask = QString("%0.%1").arg(NAME).arg(EXT);
    if (mask.isNull() || mask.isEmpty()) {
        decodedMask = QString("%0/%1/%2.%3").arg(URL).arg(SUBDIRS).arg(NAME).arg(EXT);
    } else {
        decodedMask = mask;
    }

    const QString host = url.host();
    const QString path = url.path();
    const QString filename = url.fileName();
    const QString query = url.query();

    QFileInfo fi(filename);
    QString basename = fi.completeBaseName();
    QString suffix = fi.suffix();

    if (!customFileName.isEmpty()) {
        basename = customFileName;
    }

    QString subdirs = path;
    subdirs.chop(filename.count());
    if (subdirs.startsWith(QChar('/'))) {
        subdirs.remove(0, 1);
    }
    if (subdirs.endsWith(QChar('/'))) {
        subdirs.chop(1);
    }

    QString fullUrl = host + path;

    QString flatUrl = fullUrl;
    flatUrl.replace(QChar('/'), QChar('-'));

    QString flatSubdirs = subdirs;
    flatSubdirs.replace(QChar('/'), QChar('-'));

    // Renaming Tags
    decodedMask.replace(QChar('\\'), QChar('/'));

    decodedMask.replace( NAME         , basename      );
    decodedMask.replace( EXT          , suffix        );
    decodedMask.replace( URL          , host          );
    decodedMask.replace( CURL         , fullUrl       );
    decodedMask.replace( FLATURL      , flatUrl       );
    decodedMask.replace( SUBDIRS      , subdirs       );
    decodedMask.replace( FLATSUBDIRS  , flatSubdirs   );
    decodedMask.replace( QSTRING      , query         );

    /* Remove the trailing '.' and duplicated '/' */
    decodedMask.replace(QRegExp("/+"), "/");
    decodedMask.replace(QRegExp("^/"), "");
    decodedMask.replace(QRegExp("[/\\.]*$"), "");

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
    if (tag == NAME        ) return QT_TRANSLATE_NOOP(Mask, "File name");
    if (tag == EXT         ) return QT_TRANSLATE_NOOP(Mask, "Extension");
    if (tag == URL         ) return QT_TRANSLATE_NOOP(Mask, "Base URL");
    if (tag == CURL        ) return QT_TRANSLATE_NOOP(Mask, "Full URL");
    if (tag == FLATURL     ) return QT_TRANSLATE_NOOP(Mask, "Flat full URL");
    if (tag == SUBDIRS     ) return QT_TRANSLATE_NOOP(Mask, "URL subdirectories");
    if (tag == FLATSUBDIRS ) return QT_TRANSLATE_NOOP(Mask, "Flat URL subdirectories");
    if (tag == QSTRING     ) return QT_TRANSLATE_NOOP(Mask, "Query string");
    return QString();
}
