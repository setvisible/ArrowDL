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

#ifndef IPC_CONSTANTS_H
#define IPC_CONSTANTS_H

#include <QtCore/QString>
#include <QtCore/QSharedMemory>

/*!
 * \class InterProcessCommunication
 * \brief Pass a message between the Launcher and the Application
 *
 * The message is a QString. The format described below.
 *
 * Each block is separated by char space (U+0020 or ' ').
 *
 * Block Header...:   "[IPC_BEGIN]"
 *
 * Block URL......:   "[CURRENT_URL]"
 *
 *                    <url>
 *
 * Block Links....:   "[LINKS]"
 *
 *                    <url>
 *                    <url>
 *                    ...
 *                    <url>
 *
 * Block Media....:   "[MEDIA]"
 *
 *                    <url>
 *                    <url>
 *                    ...
 *                    <url>
 *
 * Block EOF......:   "[IPC_END]"
 *
 *
 * Example 1
 * \code
 * QString msg;
 * msg = "[IPC_BEGIN] [CURRENT_URL] https://www.example.org/ [LINKS]"
 *       " https://www.example.org/image_01.png https://www.example.org/image_02.png"
 *       " https://www.example.org/image_03.png [MEDIA]"
 *       " https://www.example.org/image_01.png https://www.example.org/image_02.png [IPC_END]";
 * \endcode
 *
 * Example 2
 * \code
 * QLatin1StringView msg;
 * msg = "[IPC_BEGIN] [OPEN_URL] https://www.example.org/2019/12/" [IPC_END]";
 * \endcode
 */

static const QLatin1StringView C_SHARED_MEMORY_KEY        ("org.example.QSharedMemory.DownloadManager");
static const QLatin1StringView C_SHARED_MEMORY_ACK_REPLY  ("0K3Y_B0Y");

static const QLatin1StringView C_PACKET_BEGIN             ("[IPC_BEGIN]");
static const QLatin1StringView C_PACKET_END               ("[IPC_END]");
static const QLatin1StringView C_PACKET_ERROR             ("[ERROR]");

static const QLatin1StringView C_KEYWORD_CURRENT_URL      ("[CURRENT_URL]");
static const QLatin1StringView C_KEYWORD_LINKS            ("[LINKS]");
static const QLatin1StringView C_KEYWORD_MEDIA            ("[MEDIA]");
static const QLatin1StringView C_KEYWORD_OPEN_URL         ("[OPEN_URL]");
static const QLatin1StringView C_KEYWORD_DOWNLOAD_LINK    ("[DOWNLOAD_LINK]");
static const QLatin1StringView C_KEYWORD_MANAGER          ("[MANAGER]");
static const QLatin1StringView C_KEYWORD_PREFS            ("[PREFS]");

static const QLatin1StringView C_KEYWORD_QUICK_LINKS      ("[QUICK_LINKS]");
static const QLatin1StringView C_KEYWORD_QUICK_MEDIA      ("[QUICK_MEDIA]");
static const QLatin1StringView C_KEYWORD_STARTED_PAUSED   ("[STARTED_PAUSED]");

static const std::string C_STR_CURRENT_URL(     C_KEYWORD_CURRENT_URL.toString().toStdString());
static const std::string C_STR_LINKS(           C_KEYWORD_LINKS.toString().toStdString());
static const std::string C_STR_OPEN_URL(        C_KEYWORD_OPEN_URL.toString().toStdString());
static const std::string C_STR_DOWNLOAD_LINK(   C_KEYWORD_DOWNLOAD_LINK.toString().toStdString());
static const std::string C_STR_ERROR(           C_PACKET_ERROR.toString().toStdString());


static inline QString shm_read(QSharedMemory *sharedMemory)
{
    // Reads the shared memory.
    QByteArray bytes;
    if (sharedMemory->lock()) {
        bytes.setRawData((char*)sharedMemory->constData(), sharedMemory->size());
        sharedMemory->unlock();
    }

    // Qt5 to Qt6
    // QString::QString(const QByteArray &ba)
    // Note: : any null ('\0') bytes in the byte array will be included in this string,
    // converted to Unicode null characters (U+0000).
    // This behavior is different from Qt 5.x.
    auto real_size_excluding_null = bytes.indexOf(QChar::Null);
    bytes.truncate(real_size_excluding_null);
    return QString(bytes);
}

static inline void shm_write(QSharedMemory *sharedMemory, const QString &message)
{
    // Qt5 to Qt6
    // QString::QString(const QByteArray &ba)
    // Note: : any null ('\0') bytes in the byte array will be included in this string,
    // converted to Unicode null characters (U+0000).
    // This behavior is different from Qt 5.x.
    QByteArray bytes = message.toUtf8();
    bytes.append(QChar::Null);

    // Write message into shared memory.
    if (sharedMemory->lock()) {
        char *to = (char*)sharedMemory->data();
        const char *from = bytes.constData();
        qsizetype max_count = qMin(sharedMemory->size(), bytes.size());
        memcpy(to, from, max_count);
        sharedMemory->unlock();
    }
}

#endif // IPC_CONSTANTS_H
