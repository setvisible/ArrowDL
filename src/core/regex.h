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

#ifndef CORE_REGEX_H
#define CORE_REGEX_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

class Regex
{
public:
    static bool hasBatchDescriptors(const QString &str);

    static QStringList interpret(const QUrl &url);
    static QStringList interpret(const QString &str);

};

#endif // CORE_REGEX_H
