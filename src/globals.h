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

#ifndef GLOBALS_H
#define GLOBALS_H

#include "builddefs.h"
#include "version.h"
#include <QtCore/QString>

const QString STR_APPLICATION_NAME("DownZemAll!");
const QLatin1String STR_APPLICATION_VERSION(APP_VERSION);
const QLatin1String STR_APPLICATION_DATE("2019");
const QString STR_APPLICATION_AUTHOR("Sébastien Vavassori");

/*
 * Remark: the "Application Organization Name"
 * is the string that appears in the Windows registry.
 * It should contain only ASCII characters for maximizing
 * compatibility with Windows 7/8/10 and following.
 */
const QString STR_APPLICATION_ORGANIZATION("Sebastien Vavassori");


/* Something like "2017-05-20_15:36:58" */
const QString STR_APPLICATION_BUILD =
        QString("%1-%2-%3_%4:%5:%6")
        .arg(BUILD_YEAR, 4, 10, QChar('0'))
        .arg(BUILD_MONTH, 2, 10, QChar('0'))
        .arg(BUILD_DAY, 2, 10, QChar('0'))
        .arg(BUILD_HOUR, 2, 10, QChar('0'))
        .arg(BUILD_MIN, 2, 10, QChar('0'))
        .arg(BUILD_SEC, 2, 10, QChar('0'));


#endif // GLOBALS_H
