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

#ifndef IPC_INTER_PROCESS_COMMUNICATION_H
#define IPC_INTER_PROCESS_COMMUNICATION_H

#include <QtCore/QString>

class Model;

class InterProcessCommunication
{
    enum Mode {None, Link, Media, Other};

public:
    enum Option {
        NoOptions = 0x0,
        QuickLinks = 0x1,
        QuickMedia = 0x2,
        StartPaused = 0x4
    };
    Q_DECLARE_FLAGS(Options, Option)

    static QString readMessageFromLauncher();

    static QString clean(const QString &message);

    static bool isSingleUrl(const QString &message);
    static bool isCommandOpenManager(const QString &message);
    static bool isCommandShowPreferences(const QString &message);
    static bool isCommandOpenUrl(const QString &message);
    static QString getCurrentUrl(const QString &message);

    static bool isCommandDownloadLink(const QString &message);
    static QString getDownloadLink(const QString &message);

    static void parseMessage(const QString &message, Model *model, Options *options = nullptr);

};

Q_DECLARE_OPERATORS_FOR_FLAGS(InterProcessCommunication::Options)

#endif // IPC_INTER_PROCESS_COMMUNICATION_H
