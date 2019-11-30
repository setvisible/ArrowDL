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

#ifndef IPC_CONSTANTS_H
#define IPC_CONSTANTS_H

#include <QtCore/QString>

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
 * QString msg;
 * msg = "[IPC_BEGIN] [OPEN_URL] https://www.example.org/2019/12/" [IPC_END]";
 * \endcode
 */

static const QString C_SHARED_MEMORY_KEY("org.example.QSharedMemory.DownloadManager");
static const QString C_SHARED_MEMORY_ACK_REPLY("0K3Y_B0Y");

static const QString C_PACKET_BEGIN("[IPC_BEGIN]");
static const QString C_PACKET_END("[IPC_END]");
static const QString C_PACKET_ERROR("[ERROR]");

static const QString C_KEYWORD_CURRENT_URL("[CURRENT_URL]");
static const QString C_KEYWORD_LINKS("[LINKS]");
static const QString C_KEYWORD_MEDIA("[MEDIA]");
static const QString C_KEYWORD_OPEN_URL("[OPEN_URL]");
static const QString C_KEYWORD_MANAGER("[MANAGER]");
static const QString C_KEYWORD_PREFS("[PREFS]");

static const std::string C_STR_CURRENT_URL(C_KEYWORD_CURRENT_URL.toStdString());
static const std::string C_STR_LINKS(C_KEYWORD_LINKS.toStdString());
static const std::string C_STR_OPEN_URL(C_KEYWORD_OPEN_URL.toStdString());
static const std::string C_STR_ERROR(C_PACKET_ERROR.toStdString());


#endif // IPC_CONSTANTS_H
