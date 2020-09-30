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

#ifndef IO_FILE_READER_H
#define IO_FILE_READER_H

#include <Io/IFileHandler>

#include <QtCore/QCoreApplication>

class QIODevice;

class FileReader
{
    Q_DECLARE_TR_FUNCTIONS(FileReader)

public:
    enum FileReaderError {
        UnknownError,
        FileNotFoundError,
        DeviceError,
        UnsupportedFormatError,
        InvalidDataError
    };

    FileReader() = default;
    explicit FileReader(QIODevice *device);
    explicit FileReader(const QString &fileName);
    ~FileReader();

    bool read(DownloadEngine *engine);

    FileReaderError error() const;
    QString errorString() const;

    static QString supportedFormats();

private:
    /* Device */
    QIODevice *m_device{Q_NULLPTR};
    IFileHandlerPtr m_handler;

    bool initHandler();
    IFileHandlerPtr createReadHandlerHelper(QIODevice *device);

    /* Error */
    FileReader::FileReaderError m_fileReaderError = UnknownError;
    QString m_errorString;

    Q_DISABLE_COPY(FileReader)
};

#endif // IO_FILE_READER_H
