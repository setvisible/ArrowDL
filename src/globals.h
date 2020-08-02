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

#ifndef GLOBALS_H
#define GLOBALS_H

#include "builddefs.h"
#include "version.h"
#include <QtCore/QString>

const QString STR_APPLICATION_NAME("DownZemAll!");
const QLatin1String STR_APPLICATION_VERSION(APP_VERSION);
const QLatin1String STR_APPLICATION_DATE("2020");
const QString STR_APPLICATION_AUTHOR("Sébastien Vavassori");
const QString STR_APPLICATION_WEBSITE("https://github.com/setvisible/DownZemAll");
const QString STR_TUTORIAL_WEBSITE("https://setvisible.github.io/DownZemAll/category/tutorial.html");
const QString STR_GITHUB_REPO_ADDRESS = "https://github.com/setvisible/DownZemAll";

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


/* Compiler Infos */
const QString STR_COMPILER_WORDSIZE(QString("%0-bit").arg(QSysInfo::WordSize));
const QString STR_COMPILER_BUILD_ABI = QSysInfo::buildAbi();
const QString STR_COMPILER_BUILD_CPU = QSysInfo::buildCpuArchitecture();

const QString STR_COMPILER_NAME=
#if defined(Q_CC_BOR)
    "Borland/Turbo C++"
#elif defined(Q_CC_CDS)
    "Reliant C++"
#elif defined(Q_CC_COMEAU)
    "Comeau C++"
#elif defined(Q_CC_DEC)
    "DEC C++"
#elif defined(Q_CC_EDG)
    "Edison Design Group C++"
#elif defined(Q_CC_GHS)
    "Green Hills Optimizing C++ Compilers"
#elif defined(Q_CC_GNU)
    "GNU C++"
#elif defined(Q_CC_HIGHC)
    "MetaWare High C/C++"
#elif defined(Q_CC_HPACC)
    "HP aC++"
#elif defined(Q_CC_INTEL)
    "Intel C++ for Linux, Intel C++ for Windows"
#elif defined(Q_CC_KAI)
    "KAI C++"
#elif defined(Q_CC_MIPS)
    "MIPSpro C++"
#elif defined(Q_CC_MSVC)
    "Microsoft Visual C/C++, Intel C++ for Windows"
#elif defined(Q_CC_OC)
    "CenterLine C++"
#elif defined(Q_CC_PGI)
    "Portland Group C++"
#elif defined(Q_CC_SUN)
    "Forte Developer, or Sun Studio C++"
#elif defined(Q_CC_SYM)
    "Digital Mars C/C++ (used to be Symantec C++)"
#elif defined(Q_CC_USLC)
    "SCO OUDK and UDK"
#elif defined(Q_CC_WAT)
    "Watcom C++"
#else
    "UNKNOWN"
#endif
;

const QString STR_CURRENT_PLATFORM = QSysInfo::prettyProductName();
const QString STR_CURRENT_VERSION = QString("%0 (kernel: %1)")
        .arg(QSysInfo::productVersion())
        .arg(QSysInfo::kernelVersion());
const QString STR_CURRENT_CPU = QSysInfo::currentCpuArchitecture();
const bool IS_HOST_64BIT = STR_CURRENT_CPU.contains(QLatin1String("64"));


#endif // GLOBALS_H
