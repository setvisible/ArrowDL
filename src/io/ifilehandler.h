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

#ifndef IO_I_FILE_HANDLE_H
#define IO_I_FILE_HANDLE_H

#include <Core/DownloadEngine>

#include <QtCore/QSharedPointer>

class QIODevice;

class IFileHandler
{    
public:
    IFileHandler();
    virtual ~IFileHandler() noexcept = default; // IMPORTANT: virtual destructor

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    virtual bool canRead() const = 0;
    virtual bool canWrite() const = 0;

    /*!
     * \brief Read the internal device and add the content to the engine.
     * \param engine
     * \return true is the device was read.
     */
    virtual bool read(DownloadEngine *engine) = 0;

    /*!
     * \brief Write the engine content to the internal device. (Optional)
     * \param engine
     * \return true is the device was opened.
     */
    virtual bool write(const DownloadEngine &engine);


private:
    QIODevice *m_device = Q_NULLPTR;

    Q_DISABLE_COPY(IFileHandler)
};

typedef QSharedPointer<IFileHandler> IFileHandlerPtr;

#endif // IO_I_FILE_HANDLE_H
