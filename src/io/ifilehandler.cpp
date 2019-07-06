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

#include "ifilehandler.h"

#include <QtCore/QIODevice>

/*!
 * \class IFileHandler
 * \brief The IFileHandler class defines the common file I/O interface
 * for all file formats in DZA.
 */

IFileHandler::IFileHandler()
    : m_device(0)
{
}

void IFileHandler::setDevice(QIODevice *device)
{
    m_device = device;
}

QIODevice *IFileHandler::device() const
{
    return m_device;
}

bool IFileHandler::write(const DownloadEngine &engine)
{
    Q_UNUSED(engine);
    return false;
}
