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

#include "../../utils/fakejob.h"

#include <Core/AbstractJob>
#include <Core/JobFile>
#include <Core/Scheduler>
#include <Core/Mask>
#include <Core/ResourceItem>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QThread>
#include <QtCore/QCoreApplication>
#include <QtCore/QTemporaryDir>
#include <QtCore/QUrl>

#include <QtTest/QSignalSpy>
#include <QtTest/QtTest>


class tst_Scheduler : public QObject
{
    Q_OBJECT

private slots:
    void append();

private:
    QTemporaryDir m_tempDir;

    inline JobFile *createDummyJob(QSharedPointer<Scheduler> scheduler,
                                   const QString url,
                                   const QString mask);
};

JobFile *tst_Scheduler::createDummyJob(QSharedPointer<Scheduler> scheduler,
                                       const QString url,
                                       const QString mask)
{
    Q_ASSERT(m_tempDir.isValid());
    // qDebug() << "Directory for tests: " << m_tempDir.path();
    ResourceItem* resource = new ResourceItem();
    resource->setUrl(url);
    resource->setDestination(m_tempDir.path());
    resource->setMask(mask);
    JobFile *job = new JobFile(scheduler.data(), resource);
    return job;
}

/******************************************************************************
 ******************************************************************************/
void tst_Scheduler::append()
{
    // Given
    QScopedPointer<Scheduler> target(new Scheduler(this));

    QSignalSpy spyJobFinished(target.data(), &Scheduler::jobFinished);

    const qsizetype bytesTotal = 123*1024*1024;
    const qint64 timeIncrement = 150;
    const qint64 duration = 2500;

    FakeJob* job = new FakeJob(
        QUrl("http://www.example.com/favicon.png"), QLatin1String("favicon.png"),
        bytesTotal, timeIncrement, duration);

    QList<AbstractJob*> jobs;
    jobs.append(job);

    // When
    target->append(jobs, false);

    // Then
    QCOMPARE(spyJobFinished.count(), 0);

    // When
    target->resume(job);

    // Then
    QVERIFY(spyJobFinished.wait(5000)); // wait for 5 seconds max

    QCOMPARE(spyJobFinished.count(), 1);

    QCOMPARE(job->state(), AbstractJob::Completed);
    QCOMPARE(job->bytesReceived(), bytesTotal);
    QCOMPARE(job->bytesTotal(), bytesTotal);
}

/******************************************************************************
 ******************************************************************************/

/*
 * QSignalSpy::wait() requires QTEST_MAIN instead of QTEST_APPLESS_MAIN,
 * otherwise we get QEventLoop: Cannot be used without QApplication
 */
QTEST_MAIN(tst_Scheduler)
/* QTEST_APPLESS_MAIN(tst_Scheduler) */

#include "tst_scheduler.moc"
