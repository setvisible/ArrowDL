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

#include "filewriter.h"

#include "format.h"

#include <QtCore/QDebug>
#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>


FileWriter::FileWriter(QIODevice *device)
    : m_device(device)
{
}

FileWriter::FileWriter(const QString &fileName)
    : m_device(new QFile(fileName))
{
}

FileWriter::~FileWriter()
{
    if (m_device)
        delete m_device;
}

/******************************************************************************
 ******************************************************************************/
bool FileWriter::canWriteHelper()
{
    if (!m_device) {
        m_fileWriterError = FileWriter::DeviceError;
        m_errorString = FileWriter::tr("Device is not set");
        return false;
    }
    if (!m_device->isOpen()) {
        if (!m_device->open(QIODevice::WriteOnly | QIODevice::Text)) {
            m_fileWriterError = FileWriter::DeviceError;
            m_errorString = FileWriter::tr("Cannot open device for writing: %1")
                    .arg(m_device->errorString());
            return false;
        }
    }
    if (!m_device->isWritable()) {
        m_fileWriterError = FileWriter::DeviceError;
        m_errorString = FileWriter::tr("Device not writable");
        return false;
    }
    if (m_handler.isNull() && (m_handler = createWriteHandlerHelper(m_device)).isNull()) {
        m_fileWriterError = FileWriter::UnsupportedFormatError;
        m_errorString = FileWriter::tr("Unsupported format");
        return false;
    }
    return true;
}

/******************************************************************************
 ******************************************************************************/
bool FileWriter::canWrite()
{
    if (auto file = qobject_cast<QFile *>(m_device)) {
        const bool remove = !file->isOpen() && !file->exists();
        const bool result = canWriteHelper();
        // This looks strange (why remove if it doesn't exist?) but the issue
        // here is that canWriteHelper will create the file in the process of
        // checking if the write can succeed. If it subsequently fails, we
        // should remove that empty file.
        if (!result && remove)
            file->remove();
        return result;
    }
    return canWriteHelper();
}

/******************************************************************************
 ******************************************************************************/
bool FileWriter::write(DownloadEngine *engine)
{
    // Do this before canWrite, so it doesn't create a file if this fails.
    if (Q_UNLIKELY(!engine)) {
        m_fileWriterError = FileWriter::InvalidFileError;
        m_errorString = FileWriter::tr("File is empty");
        return false;
    }
    if (!canWrite()) {
        return false;
    }
    if (!m_handler->write(*engine)) {
        return false;
    }
    if (auto file = qobject_cast<QFile *>(m_device)) {
        file->flush();
    }
    return true;
}

FileWriter::FileWriterError FileWriter::error() const
{
    return m_fileWriterError;
}

QString FileWriter::errorString() const
{
    return m_errorString;
}

/******************************************************************************
 ******************************************************************************/
IFileHandlerPtr FileWriter::createWriteHandlerHelper(QIODevice *device)
{
    IFileHandlerPtr handler;
    QByteArray suffix;

    if (device) {
        // if there's no format, see if \a device is a file, and if so, find
        // the file suffix and find support for that format among our plugins.
        // this allows plugins to override our built-in handlers.
        if (auto file = qobject_cast<QFile *>(device)) {
            suffix = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        }
    }
    // check if any built-in handlers can write the data
    if (!handler && !suffix.isEmpty()) {
        handler = Io::findHandlerFromSuffix(suffix);
    }
    if (!handler) {
        return IFileHandlerPtr();
    }
    if (!handler->canWrite()) {
        return IFileHandlerPtr();
    }
    handler->setDevice(device);
    return handler;
}

/******************************************************************************
 ******************************************************************************/
QString FileWriter::supportedFormats()
{
    QString text;
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
