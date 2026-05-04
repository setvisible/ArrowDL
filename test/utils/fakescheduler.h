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

#ifndef FAKE_DOWNLOAD_MANAGER_H
#define FAKE_DOWNLOAD_MANAGER_H

#include <Core/Scheduler>

class FakeScheduler : public Scheduler
{
    Q_OBJECT

public:
    explicit FakeScheduler(QObject *parent = nullptr);
    ~FakeScheduler();

    AbstractJob* createJobFile(const QUrl &url);

    /* Utility */
    void createFakeJobs(int count = 100);
    void appendFakeJob(const QUrl &url);

};

#endif // FAKE_DOWNLOAD_MANAGER_H
