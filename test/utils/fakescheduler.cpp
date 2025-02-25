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

#include "fakescheduler.h"

#include "fakejob.h"

#include <Core/ResourceItem>


FakeScheduler::FakeScheduler(QObject *parent) : Scheduler(parent)
{
}

FakeScheduler::~FakeScheduler()
{
}

AbstractJob* FakeScheduler::createJobFile(const QUrl &url)
{
    FakeJob *job = new FakeJob(this, new ResourceItem());
    job->setSourceUrl(url);
    return job;
}

void FakeScheduler::createFakeJobs(int count)
{
    QList<AbstractJob*> jobs;
    for (auto i = 0; i < count; ++i) {
        auto job = new FakeJob(this, new ResourceItem());
        jobs.append(job);
    }
    Scheduler::append(jobs, false);
}

void FakeScheduler::appendFakeJob(const QUrl &url)
{
    AbstractJob *job = createJobFile(url);

    QList<AbstractJob*> jobs;
    jobs.append(job);
    Scheduler::append(jobs, false);
}
