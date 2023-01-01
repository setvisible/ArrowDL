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

#ifndef VERSION_H
#define VERSION_H

#include <libtorrent/version.hpp>

/* CMake */
#ifndef APP_VERSION
#  include "config.h"
#endif

#ifndef BOOST_VERSION_STR
#  include "config_3rd.h"
#endif

#define GOOGLE_GUMBO_VERSION_STR      "0.10.1"

#define LIBTORRENT_VERSION_STR LIBTORRENT_VERSION

#endif // VERSION_H
