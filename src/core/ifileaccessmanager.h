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

#ifndef CORE_I_FILE_ACCESS_MANAGER_H
#define CORE_I_FILE_ACCESS_MANAGER_H

#include <Core/Settings>

class IFileAccessManager
{
public:
    IFileAccessManager() = default;
    virtual ~IFileAccessManager() noexcept = default; /* Pure virtual interface */

    virtual Settings* settings() const = 0;

    virtual ExistingFileOption aboutToModify(const QString &filename) = 0;
};

#endif // CORE_I_FILE_ACCESS_MANAGER_H
