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

#include "resourceitem.h"

#include <Core/Mask>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QUrl>

ResourceItem::ResourceItem()
    : m_url(QString())
    , m_destination(QString())
    , m_mask(QString())
    , m_customFileName(QString())
    , m_referringPage(QString())
    , m_description(QString())
    , m_checkSum(QString())
    , m_isStreamEnabled(false)
    , m_streamFileName(QString())
    , m_streamFormatId(QString())
    , m_streamFileSize(0)
    , m_isTorrentEnabled(false)
{
}

/******************************************************************************
 ******************************************************************************/
QString ResourceItem::url() const
{
    return m_url;
}

void ResourceItem::setUrl(const QString &url)
{
    m_url = url;
}

QUrl ResourceItem::distantFileUrl() const
{
    return m_url;
}

/******************************************************************************
 ******************************************************************************/
QString ResourceItem::destination() const
{
    return m_destination;
}

void ResourceItem::setDestination(const QString &destination)
{
    m_destination = destination;
}

/******************************************************************************
 ******************************************************************************/
QString ResourceItem::mask() const
{
    return m_mask;
}

void ResourceItem::setMask(const QString &mask)
{
    m_mask = mask;
}

/******************************************************************************
 ******************************************************************************/
QString ResourceItem::customFileName() const
{
    return m_customFileName;
}

void ResourceItem::setCustomFileName(const QString &customFileName)
{
    m_customFileName = customFileName;
}

/******************************************************************************
 ******************************************************************************/
QUrl ResourceItem::localFileUrl() const
{
    const QString path = localFilePath(m_customFileName);
    return QUrl::fromLocalFile(path);
}

QString ResourceItem::fileName() const
{
    const QUrl url = localFileUrl();
    if (!url.isEmpty() && url.isValid()) {
        return url.fileName();
    }
    return QString();
}

QString ResourceItem::localFileFullPath(const QString &customFileName) const
{
    return localFilePath(customFileName);
}

/******************************************************************************
 ******************************************************************************/
/**
 * URL of the HTML page where the URL was found, if so.
 */
QString ResourceItem::referringPage() const
{
    return m_referringPage;
}

void ResourceItem::setReferringPage(const QString &referringPage)
{
    m_referringPage = referringPage;
}

/******************************************************************************
 ******************************************************************************/
/** Closest alt='' attribute or title='' attribute or just the content
 * found in the HTML page.
 *
 * Ex:
 * <img src="../pic.png" alt="Blah" />
 *                            ^^^^
 * Ex2:
 * <a href="../pic.png">Blah</a>
 *                      ^^^^
 * Ex3:
 * <head>
 *  <title>Blah</title>
 *         ^^^^
 * </head>
 *
 */
QString ResourceItem::description() const
{
    return m_description;
}

void ResourceItem::setDescription(const QString &description)
{
    m_description = description;
}

/******************************************************************************
 ******************************************************************************/
QString ResourceItem::checkSum() const
{
    return m_checkSum;
}

void ResourceItem::setCheckSum(const QString &checkSum)
{
    m_checkSum = checkSum;
}

/******************************************************************************
 ******************************************************************************/
bool ResourceItem::isStreamEnabled() const
{
    return m_isStreamEnabled;
}

void ResourceItem::setStreamEnabled(bool enabled)
{
    m_isStreamEnabled = enabled;
}

/******************************************************************************
 ******************************************************************************/
QString ResourceItem::streamFileName() const
{
    return m_streamFileName;
}

void ResourceItem::setStreamFileName(const QString &streamFileName)
{
    m_streamFileName = streamFileName;
}

/******************************************************************************
 ******************************************************************************/
QString ResourceItem::streamFormatId() const
{
    return m_streamFormatId;
}

void ResourceItem::setStreamFormatId(const QString &streamFormatId)
{
    m_streamFormatId = streamFormatId;
}

/******************************************************************************
 ******************************************************************************/
qint64 ResourceItem::streamFileSize() const
{
    return m_streamFileSize;
}

void ResourceItem::setStreamFileSize(qint64 streamFileSize)
{
    m_streamFileSize = streamFileSize;
}

/******************************************************************************
 ******************************************************************************/
bool ResourceItem::isTorrentEnabled() const
{
    return m_isTorrentEnabled;
}

void ResourceItem::setTorrentEnabled(bool enabled)
{
    m_isTorrentEnabled = enabled;
}

/******************************************************************************
 ******************************************************************************/
QString ResourceItem::torrentPreferredFilePriorities() const
{
    return m_torrentPreferredFilePriorities;
}

void ResourceItem::setTorrentPreferredFilePriorities(const QString &priorities)
{
    m_torrentPreferredFilePriorities = priorities;
}

/******************************************************************************
 ******************************************************************************/
inline QString ResourceItem::localFilePath(const QString &customFileName) const
{
    if (QUrl(m_url).scheme() == "magnet") {
        return localMagnetFile(customFileName);
    }
    if (m_isStreamEnabled) {
        return localStreamFile(customFileName);
    }
    return localFile(m_destination, m_url, customFileName, m_mask);
}

inline QString ResourceItem::localStreamFile(const QString &customFileName) const
{
    QString url = QUrl(m_url).host() + "/" + m_streamFileName;
    const QString fileName = Mask::interpret(url, customFileName, m_mask);
    return QDir(m_destination).filePath(fileName);
}

inline QString ResourceItem::localMagnetFile(const QString &customFileName) const
{
    QString displayName = parseMagnetUrl(m_url);
    QString fileName = customFileName.isEmpty() ? displayName : customFileName;
    fileName += ".torrent";

    // Remark: No mask for magnets
    // const QString fileName = Mask::interpret(url, customFileName, m_mask);
    return QDir(m_destination).filePath(fileName);
}

inline QString ResourceItem::parseMagnetUrl(const QString &url) const
{
    /// todo move to Mask::interpretMagnet ?
    QRegExp regex("^"
                  + QRegExp::escape("magnet:?")
                  + ".*"+ QRegExp::escape("&") + "?"
                  + QRegExp::escape("dn=")
                  // *********************
                  // captured group #1
                  // rem: [^A] means anything not A
                  + "([^" + QRegExp::escape("&") + "]*)"
                  // *********************
                  + "(" + QRegExp::escape("&") + ".*)?"
                  + "$"
                  );
    if (regex.indexIn(url) != -1) {
        QString displayName = regex.cap(1);
        displayName = Mask::decodeMagnetEncoding(displayName);
        return displayName;
    }
    return QString("[Wait... Downloading metadata...]");
}

/******************************************************************************
 ******************************************************************************/
inline QString ResourceItem::localFile(const QString &destination, const QUrl &url,
                                       const QString &customFileName, const QString &mask)
{
    const QString fileName = Mask::interpret(url, customFileName, mask);
    return QDir(destination).filePath(fileName);
}
