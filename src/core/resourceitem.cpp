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

#include <Core/FileUtils>
#include <Core/Mask>
#include <Core/Stream>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRegularExpression>
#include <QtCore/QUrl>

static const QLatin1StringView s_regular("regular");
static const QLatin1StringView s_stream("stream");
static const QLatin1StringView s_torrent("torrent");


ResourceItem::Type ResourceItem::type() const
{
    return m_type;
}

void ResourceItem::setType(Type type)
{
    m_type = type;
}

QString ResourceItem::toString(Type type)
{
    switch (type) {
    case Type::Stream:  return s_stream;
    case Type::Torrent: return s_torrent;
    default:
        return s_regular;
    }
}

ResourceItem::Type ResourceItem::fromString(const QString &str)
{
    if (str.toLower() == s_stream)  return Type::Stream;
    if (str.toLower() == s_torrent) return Type::Torrent;
    return Type::Regular;
}

/******************************************************************************
 ******************************************************************************/
QString ResourceItem::url() const
{
    return m_url;
}

QUrl ResourceItem::url_TODO() const
{
    return QUrl(m_url);
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
    auto path = localFilePath(m_customFileName);
    return QUrl::fromLocalFile(path);
}

QString ResourceItem::fileName() const
{
    auto url = localFileUrl();
    if (!url.isEmpty() && url.isValid()) {
        return url.fileName();
    }
    return {};
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
qsizetype ResourceItem::streamFileSize() const
{
    return m_streamFileSize;
}

void ResourceItem::setStreamFileSize(qsizetype streamFileSize)
{
    m_streamFileSize = streamFileSize;
}

/******************************************************************************
 ******************************************************************************/
StreamObject::Config ResourceItem::streamConfig() const
{
    return m_streamConfig;
}

void ResourceItem::setStreamConfig(const StreamObject::Config &config)
{
    m_streamConfig = config;
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
    if (m_type == Type::Stream) {
        return localStreamFile(customFileName);
    }
    return localFile(m_destination, m_url, customFileName, m_mask);
}

inline QString ResourceItem::localStreamFile(const QString &customFileName) const
{
    QString url = QUrl(m_url).host() % "/" % m_streamFileName;
    auto fileName = Mask::interpret(url, customFileName, m_mask);
    fileName = FileUtils::validateFileName(fileName, true);
    return QDir(m_destination).filePath(fileName);
}

inline QString ResourceItem::localMagnetFile(const QString &customFileName) const
{
    auto displayName = parseMagnetUrl(m_url);
    auto fileName = customFileName.isEmpty() ? displayName : customFileName;
    fileName += ".torrent";

    // Remark: No mask for magnets
    // const QString fileName = Mask::interpret(url, customFileName, m_mask);

    fileName = FileUtils::validateFileName(fileName, false);
    return QDir(m_destination).filePath(fileName);
}

inline QString ResourceItem::parseMagnetUrl(const QString &url) const
{
    /// todo move to Mask::interpretMagnet ?
    static QRegularExpression regex(
        "^"
        % QRegularExpression::escape("magnet:?")
        % ".*"% QRegularExpression::escape("&") % "?"
        % QRegularExpression::escape("dn=")
        // *********************
        // captured group #1
        // rem: [^A] means anything not A
        % "([^" % QRegularExpression::escape("&") % "]*)"
        // *********************
        % "(" % QRegularExpression::escape("&") % ".*)?"
        % "$");

    auto match = regex.match(url);
    if (match.hasMatch()) {
        auto displayName = match.captured(1);
        displayName = Mask::decodeMagnetEncoding(displayName);
        return displayName;
    }
    return QString("[Wait, downloading metadata]");
}

/******************************************************************************
 ******************************************************************************/
inline QString ResourceItem::localFile(const QString &destination, const QUrl &url,
                                       const QString &customFileName, const QString &mask)
{
    auto fileName = Mask::interpret(url, customFileName, mask);
    fileName = FileUtils::validateFileName(fileName, true);
    return QDir(destination).filePath(fileName);
}
