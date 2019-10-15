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

#ifndef IO_FORMAT_H
#define IO_FORMAT_H

#include <Io/IFileHandler>
#include <Io/JsonHandler>
#include <Io/TextHandler>

#include <QtCore/QString>

namespace Io {

struct FileFormat {
    const char *suffix;
    const char *text;
    const IFileHandler* handler;
};

static const FileFormat formats[] = {
    { "txt", "Text Files", new TextHandler() },
    { "json", "Json Files", new JsonHandler() },
    { 0, 0, 0 }
};

static IFileHandler* findHandlerFromSuffix(const QString &suffix)
{
    IFileHandler *handler = Q_NULLPTR;
    for (const FileFormat *fmt = &formats[0]; fmt->handler; fmt++) {
        if (suffix == fmt->suffix) {            
            handler = const_cast<IFileHandler *>(fmt->handler);
            break;
        }
    }
    return handler;
}

}

#endif // IO_FORMAT_H
