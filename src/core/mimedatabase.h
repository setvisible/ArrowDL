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

#ifndef CORE_MIME_DATABASE_H
#define CORE_MIME_DATABASE_H


#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QPixmap>

/*!
 * \class MimeDatabase
 * \brief Internal GUI helper methods related to file mime types.
 */
class MimeDatabase
{
public:
    static QPixmap fileIcon(const QString &url, int extend = 32);

};

#endif // CORE_MIME_DATABASE_H
