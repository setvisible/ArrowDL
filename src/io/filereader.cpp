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

#include "filereader.h"

#include "format.h"

#include <Core/IScheduler>

#include <QtCore/QDebug>
#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>


FileReader::FileReader(QIODevice *device)
    : m_device(device)
{
}

FileReader::FileReader(const QString &fileName)
    : m_device(new QFile(fileName))
{
}

FileReader::~FileReader()
{
    delete m_device;
}

/******************************************************************************
 ******************************************************************************/
bool FileReader::initHandler()
{
    // check some preconditions
    if (!m_device) {
        m_fileReaderError = FileReader::DeviceError;
        m_errorString = FileReader::tr("Invalid device");
        return false;
    }
    // probe the file extension
    if (!m_device->isOpen() && !m_device->open(QIODevice::ReadOnly | QIODevice::Text)) {

        auto file = dynamic_cast<QFile*>(m_device);
        if (file->error() == QFileDevice::ResourceError) {
            // this is bad. we should abort the open attempt and note the failure.
            m_fileReaderError = FileReader::DeviceError;
            m_errorString = file->errorString();
            return false;
        }
        if (!m_device->isOpen()) {
            m_fileReaderError = FileReader::FileNotFoundError;
            m_errorString = FileReader::tr("File not found");
            return false;
        }
    }
    // assign a handler
    if (m_handler.isNull() && (m_handler = createReadHandlerHelper(m_device)).isNull()) {
        m_fileReaderError = FileReader::UnsupportedFormatError;
        m_errorString = FileReader::tr("Unsupported format");
        return false;
    }
    return true;
}

/******************************************************************************
 ******************************************************************************/
bool FileReader::read(IScheduler *scheduler)
{
    if (!scheduler) {
        qWarning("FileReader::read: cannot read into null pointer");
        return false;
    }
    if (m_handler.isNull() && !initHandler()) {
        return false;
    }
    try {
        const bool result = m_handler->read(scheduler);
        if (!result) {
            m_fileReaderError = InvalidDataError;
            m_errorString = FileReader::tr("Unable to read data");
            return false;
        }
    } catch (std::exception const& e) {
        m_fileReaderError = UnknownError;
        m_errorString = QString::fromUtf8(e.what());
        return false;
    }
    return true;
}

FileReader::FileReaderError FileReader::error() const
{
    return m_fileReaderError;
}

QString FileReader::errorString() const
{
    if (m_errorString.isEmpty())
        return FileReader::tr("Unknown error");
    return m_errorString;
}

/******************************************************************************
 ******************************************************************************/
IFileHandlerPtr FileReader::createReadHandlerHelper(QIODevice *device)
{
    IFileHandlerPtr handler;
    QByteArray suffix;

    if (auto file = qobject_cast<QFile *>(device)) {
        // device is a file
        suffix = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
    }
    // check if any built-in handlers can write the data
    if (!handler && !suffix.isEmpty()) {
        handler = Io::findHandlerFromSuffix(suffix);
    }

    /// \todo implement autoDetectFormat ?

    if (handler.isNull()) {
        // no handler: give up.
        return {};
    }
    if (!handler->canRead()) {
        return {};
    }
    handler->setDevice(device);
    return handler;
}

/******************************************************************************
 ******************************************************************************/
QString FileReader::supportedFormats()
{
    QString text;
    {
        QString allSuffixes;
        for (const Io::FileFormat *fmt = &Io::formats[0]; fmt->handler; fmt++) {
            if (!fmt->handler->canRead()) {
                continue;
            }
            if (!allSuffixes.isEmpty()) {
                allSuffixes.append(" ");
            }
            allSuffixes.append(QString("*.%0").arg(fmt->suffix));
        }
        text.append(tr("Any file (all types) (%0)").arg(allSuffixes));
    }
    for (const Io::FileFormat *fmt = &Io::formats[0]; fmt->handler; fmt++) {
        if (!fmt->handler->canRead()) {
            continue;
        }
        if (!text.isEmpty()) {
            text.append(";;");
        }
        text.append(QString("%0 (*.%1)").arg(fmt->tr_text(), fmt->suffix));
    }
    if (!text.isEmpty()) {
        text.append(";;");
    }
    text.append(tr("All files (%0)").arg(QString("*.*")));
    return text;
}
