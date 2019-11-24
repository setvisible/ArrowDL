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

#include "interprocesscommunication.h"

#include <QtCore/QDebug>
#include <QtCore/QSharedMemory>

static const QString C_SHARED_MEMORY_KEY("org.example.QSharedMemory.DownloadManager");
static const QString C_SHARED_MEMORY_ACK_REPLY("0K3Y_B0Y");

QString InterProcessCommunication::readMessage()
{
    QString message;

    QSharedMemory sharedMemory;
    sharedMemory.setKey(C_SHARED_MEMORY_KEY);

    if (sharedMemory.attach(QSharedMemory::ReadWrite)) {

        QByteArray bytes;

        if (sharedMemory.lock()) {
            // Reads the shared memory
            const char* ptr = static_cast<const char*>(sharedMemory.constData());
            uint n = static_cast<uint>(sharedMemory.size());
            bytes.setRawData(ptr, n);
            sharedMemory.unlock();
        }

        message += QString(bytes);
        message += QChar::Space;

        if (sharedMemory.lock()) {
            // Replies the ACK message
            bytes = C_SHARED_MEMORY_ACK_REPLY.toUtf8();

            const char *from = bytes.constData();
            void *to = sharedMemory.data();
            size_t size = static_cast<size_t>(qMin(bytes.size() + 1, sharedMemory.size()));
            memcpy(to, from, size);

            char *d_ptr = static_cast<char*>(sharedMemory.data());
            d_ptr[sharedMemory.size() - 1] = '\0';

            sharedMemory.unlock();
        }

        sharedMemory.detach();

    } else {
        /*
         * Unable to attach to the shared memory segment.
         *
         * Use sharedMemory.error() and sharedMemory.errorString()
         * to investigate the error.
         */
        message = "[ERROR]";
    }
    return message;
}

