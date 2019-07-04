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

#include "resourceitem.h"

#include <Core/Mask>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QUrl>

ResourceItem::ResourceItem()
    :  m_url(QString())
    , m_destination(QString())
    , m_mask(QString())
    , m_customFileName(QString())
    , m_referringPage(QString())
    , m_description(QString())
    , m_checkSum(QString())
    , m_isSelected(false)
{
}

ResourceItem::~ResourceItem()
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
    const QString dirPath = m_destination;
    const QString fileName = Mask::interpret(m_url, m_customFileName, m_mask);
    const QString path = QDir(dirPath).filePath(fileName);
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
bool ResourceItem::isSelected() const
{
    return m_isSelected;
}

void ResourceItem::setSelected(const bool isSelected)
{
    m_isSelected = isSelected;
}
