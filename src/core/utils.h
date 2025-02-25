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

#ifndef CORE_UTILS_H
#define CORE_UTILS_H

#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QString>

class Utils
{
public:
    static QList<QPair<int, int> > transformToRanges(QList<int> nums, bool ascending);
};

#endif // CORE_UTILS_H
