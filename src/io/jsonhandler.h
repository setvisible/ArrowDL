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

#ifndef IO_JSON_HANDLER_H
#define IO_JSON_HANDLER_H

#include <Core/IScheduler>
#include <Io/IFileHandler>

class JsonHandler : public IFileHandler
{
public:
    explicit JsonHandler() = default;

    bool canRead() const override;
    bool canWrite() const override;

    bool read(IScheduler *scheduler) override;
    bool write(const IScheduler &scheduler) override;

private:
};

#endif // IO_JSON_HANDLER_H
