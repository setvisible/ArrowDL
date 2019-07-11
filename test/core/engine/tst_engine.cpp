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

#include <Core/Engine>
#include <Core/JobClient>
#include <Core/Mask>
#include <Core/ResourceItem>

#include <QtCore/QThread>
#include <QtCore/QCoreApplication>
#include <QtCore/QTemporaryDir>

#include <QtTest/QSignalSpy>
#include <QtTest/QtTest>
#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

class tst_Engine : public QObject
{
    Q_OBJECT

private slots:
    void appendJob();
    void appendJobPaused();

private:
    JobClient *createDummyJob(const QString url, const QString mask);
    QTemporaryDir m_tempDir;
};

/******************************************************************************
 ******************************************************************************/
JobClient *tst_Engine::createDummyJob(const QString url, const QString mask)
{
    Q_ASSERT(m_tempDir.isValid());
    qDebug() << "Directory for tests: " << m_tempDir.path();
    ResourceItem* resource1 = new ResourceItem();
    resource1->setUrl(url);
    resource1->setDestination(m_tempDir.path());
    resource1->setMask(mask);
    JobClient *job = new JobClient(this);
    job->setResource(resource1);
    return job;
}

/******************************************************************************
 ******************************************************************************/
void tst_Engine::appendJob()
{
    // Given
    Engine *target = new Engine( this);

    qRegisterMetaType<JobClient*>();
    QSignalSpy spyJobAppended(target, SIGNAL(jobAppended(JobClient*)));
    QSignalSpy spyJobRemoved(target, SIGNAL(jobRemoved(JobClient*)));
    QSignalSpy spyJobStateChanged(target, SIGNAL(jobStateChanged(JobClient*)));
    QSignalSpy spyDownloadFinished(target, SIGNAL(downloadFinished(bool)));

    JobClient *job = createDummyJob(
                "https://avatars3.githubusercontent.com/u/20563751?s=460&v=4",
                "*name*.png");

    // When
    target->append(job, true);

    // Then
    QVERIFY(spyDownloadFinished.wait(5000)); // wait for 5 seconds max

    QCOMPARE(spyJobAppended.count(), 1);
    QCOMPARE(spyJobRemoved.count(), 0);
    QCOMPARE(spyJobStateChanged.count(), 6);
    QCOMPARE(spyDownloadFinished.count(), 1);

    QCOMPARE(job->state(), JobClient::Completed);
    QCOMPARE(job->bytesReceived(), 35493);
    QCOMPARE(job->bytesReceived(), 35493);

    QFile localFile(job->localFullFileName());
    QVERIFY(localFile.exists());
    QCOMPARE(localFile.size(), 35493);

    job->deleteLater();
    target->deleteLater();
}

/******************************************************************************
 ******************************************************************************/
void tst_Engine::appendJobPaused()
{
    // Given
    Engine *target = new Engine( this);

    qRegisterMetaType<JobClient*>();
    QSignalSpy spyJobAppended(target, SIGNAL(jobAppended(JobClient*)));
    QSignalSpy spyJobRemoved(target, SIGNAL(jobRemoved(JobClient*)));
    QSignalSpy spyJobStateChanged(target, SIGNAL(jobStateChanged(JobClient*)));
    QSignalSpy spyDownloadFinished(target, SIGNAL(downloadFinished(bool)));

    JobClient *job = createDummyJob(
                "https://raw.githubusercontent.com/setvisible/nastran-pch2csv/"
                "master/doc/320px-Blue-punch-card-front-horiz.png",
                "*name*.png");

    // When
    target->append(job, false);

    // Then
    QCOMPARE(spyJobAppended.count(), 1);
    QCOMPARE(spyJobRemoved.count(), 0);
    QCOMPARE(spyJobStateChanged.count(), 0);
    QCOMPARE(spyDownloadFinished.count(), 0);

    // When
    target->resume(job);

    // Then
    QVERIFY(spyDownloadFinished.wait(5000)); // wait for 5 seconds max

    QCOMPARE(spyJobAppended.count(), 1);
    QCOMPARE(spyJobRemoved.count(), 0);
    QCOMPARE(spyJobStateChanged.count(), 7);
    QCOMPARE(spyDownloadFinished.count(), 1);

    QCOMPARE(job->state(), JobClient::Completed);
    QCOMPARE(job->bytesReceived(), 89588);
    QCOMPARE(job->bytesReceived(), 89588);

    QFile localFile(job->localFullFileName());
    QVERIFY(localFile.exists());
    QCOMPARE(localFile.size(), 89588);

    job->deleteLater();
    target->deleteLater();
}

/******************************************************************************
 ******************************************************************************/

/*
 * QSignalSpy::wait() requires QTEST_MAIN instead of QTEST_APPLESS_MAIN,
 * otherwise we get QEventLoop: Cannot be used without QApplication
 */
QTEST_MAIN(tst_Engine)
/* QTEST_APPLESS_MAIN(tst_Engine) */

#include "tst_engine.moc"
