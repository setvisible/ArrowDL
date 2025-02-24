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

#ifndef DUMMY_SCHEDULER_H
#define DUMMY_SCHEDULER_H

#include <Core/IScheduler>

#include <QtCore/QObject>

class DummyScheduler : public QObject, public IScheduler
{
    Q_OBJECT
public:
    explicit DummyScheduler(QObject *parent = nullptr);
    ~DummyScheduler() override = default;

    void append(const QList<AbstractJob *> &jobs, bool started = false) override;
    void remove(const QList<AbstractJob *> &jobs);

    QList<AbstractJob *> jobs() const override;

    AbstractJob* createJobFile(const QUrl &url) override;
    AbstractJob* createJobTorrent(const QUrl &url) override;

    // Utility
    void createFakeJobs(int count = 100);
    void appendFakeJob(const QUrl &url);

private:
    QList<AbstractJob *> m_jobs;
};

#endif // DUMMY_SCHEDULER_H
