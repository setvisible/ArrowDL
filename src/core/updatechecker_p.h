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

#ifndef CORE_UPDATE_CHECKER_PRIVATE_H
#define CORE_UPDATE_CHECKER_PRIVATE_H

#include <functional> /* std::function */

#include <QtCore/QString>

namespace UpdateCheckerNS
{
inline std::function<bool (const QString &)> addressMatcher(bool isHost64Bit)
{
#if defined _WIN32
    if (isHost64Bit) {
        const auto addressMatcher64 = [](const QString& address) {
            return address.contains("DownZemAll_x64_Setup.exe");
        };
        return addressMatcher64;
    } else {
        const auto addressMatcher32 = [](const QString& address) {
            return address.contains("DownZemAll_x86_Setup.exe");
        };
        return addressMatcher32;
    }
#elif defined __APPLE__
    const auto noMatch = [](const QString& /*address*/) { return false; };
    return noMatch;
#else
    const auto noMatch = [](const QString& /*address*/) { return false; };
    return noMatch;
#endif
}
} // end namespace

#endif // CORE_UPDATE_CHECKER_PRIVATE_H
