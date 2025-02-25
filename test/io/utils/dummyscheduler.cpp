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

#include "dummyjob.h"
#include "dummyscheduler.h"

#include <Core/AbstractJob>
#include <Core/ResourceItem>

DummyScheduler::DummyScheduler(QObject *parent) : QObject(parent)
{
}

void DummyScheduler::append(const QList<AbstractJob *> &jobs, bool started)
{
    m_jobs.append(jobs);
}

void DummyScheduler::remove(const QList<AbstractJob *> &jobs)
{
    for (auto job : jobs) {
        m_jobs.removeAll(job);
    }
}

QList<AbstractJob *> DummyScheduler::jobs() const
{
    return m_jobs;
}

AbstractJob* DummyScheduler::createJobFile(const QUrl &url)
{
    auto job = new DummyJob(this, new ResourceItem());
    job->setSourceUrl(url);
    return job;
}

AbstractJob* DummyScheduler::createJobTorrent(const QUrl &url)
{
    Q_UNUSED(url);
    return nullptr;
}

void DummyScheduler::createFakeJobs(int count)
{
    QList<AbstractJob*> jobs;
    for (auto i = 0; i < count; ++i) {
        auto job = new DummyJob(this, new ResourceItem());
        jobs.append(job);
    }
    append(jobs, false);
}

void DummyScheduler::appendFakeJob(const QUrl &url)
{
    AbstractJob *job = createJobFile(url);

    QList<AbstractJob*> jobs;
    jobs.append(job);
    append(jobs, false);
}
