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

#include "dummyjob.h"

#include <Core/ResourceItem>

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>

DummyJob::DummyJob(QObject *parent, ResourceItem *resource)
    : AbstractJob(parent, resource)
{
    m_resourceUrl = QUrl("https://www.example.com/myfolder/myimage.png");
    m_resourceLocalFileName = QString("myimage.png");
}

/******************************************************************************
 ******************************************************************************/
QUrl DummyJob::sourceUrl() const
{
    return m_resourceUrl;
}

void DummyJob::setSourceUrl(const QUrl &resourceUrl)
{
    m_resourceUrl = resourceUrl;
}

QString DummyJob::localFullFileName() const
{
    return m_resourceUrl.toLocalFile();
}

QString DummyJob::localFileName() const
{
    return m_resourceLocalFileName;
}

QString DummyJob::localFilePath() const
{
    const QFileInfo fi(m_resourceUrl.toLocalFile());
    return fi.absolutePath();
}

QUrl DummyJob::localFileUrl() const
{
    return m_resourceUrl;
}

QUrl DummyJob::localDirUrl() const
{
    return m_resourceUrl;
}
