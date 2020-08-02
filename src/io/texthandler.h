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

#ifndef IO_TEXT_HANDLER_H
#define IO_TEXT_HANDLER_H

#include <Io/IFileHandler>

class TextHandler : public IFileHandler
{
public:
    explicit TextHandler() = default;

    bool canRead() const Q_DECL_OVERRIDE;
    bool canWrite() const Q_DECL_OVERRIDE;

    bool read(DownloadEngine *engine) Q_DECL_OVERRIDE;
    bool write(const DownloadEngine &engine) Q_DECL_OVERRIDE;

private:
};

#endif // IO_TEXT_HANDLER_H
