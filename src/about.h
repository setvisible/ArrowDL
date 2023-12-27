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

#ifndef ABOUT_H
#define ABOUT_H

#include "version.h"

#include <QtCore/QObject>
#include <QtCore/QString>


static QString buildAbout(const QString &paragraphDelimiter)
{
    return QString(QObject::tr(
           "%0 - %1 - version %2 - build %3"
           ).arg(STR_APPLICATION_NAME, STR_COMPILER_WORDSIZE, STR_APPLICATION_VERSION, STR_APPLICATION_BUILD) +
       paragraphDelimiter +
       QObject::tr(
           "Copyright (C) %0 %1. All rights reserved."
           ).arg(STR_APPLICATION_DATE, STR_APPLICATION_AUTHOR) +
       paragraphDelimiter +
       QObject::tr("GNU LGPL License") +
       paragraphDelimiter +
       QObject::tr(
           "Permission is hereby granted, free of charge, to any person obtaining a copy "
           "of this software and associated documentation files (the \"Software\"), to deal "
           "in the Software without restriction, including without limitation the rights "
           "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
           "copies of the Software, and to permit persons to whom the Software is "
           "furnished to do so, subject to the following conditions: ") +
       paragraphDelimiter +
       QObject::tr(
           "The above copyright notice and this permission notice shall be included in all "
           "copies or substantial portions of the Software. ") +
       paragraphDelimiter +
       QObject::tr(
           "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
           "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
           "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
           "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
           "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
           "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
           "SOFTWARE. "));
}

/**
 * @brief Classic "About" message.
 * The classic message can be shown from a console with classic:
 * \code
 *   myApp -v
 *   myApp --version
 * \endcode
 */
inline QString about()
{
    return buildAbout("\n\n");
}

inline QString aboutHtml()
{
    return QString("<h3>" + QObject::tr("About %0").arg(STR_APPLICATION_NAME) + "</h3>" +
                   "<p><img src=\"://resources/logo/icon128.png\" /></p>" +
                   "<p></p>" +
                   "<p>" + buildAbout("</p><p>") + "</p>");
}

#endif // ABOUT_H
