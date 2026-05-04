/* - ArrowDL - Copyright (C) 2019-present Sebastien Vavassori
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * Version 3, 29 June 2007, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CORE_SESSION_H
#define CORE_SESSION_H

#include <QtCore/QList>
#include <QtCore/QString>

class AbstractJob;
class Scheduler;

class Session
{
public:
    Session() = default;

    static void read(QList<AbstractJob *> &jobs,
                     const QString &filename,
                     Scheduler *scheduler);

    static void write(const QList<AbstractJob *> &jobs,
                      const QString &filename);
};

#endif // CORE_SESSION_H
