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

#include <Core/DownloadManager>
#include <Core/DownloadItem>
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

class tst_DownloadManager : public QObject
{
    Q_OBJECT

private slots:
    void appendJob();
    void appendJobPaused();

private:
    DownloadItem *createDummyJob(
            DownloadManager *downloadManager, const QString url, const QString mask);
    QTemporaryDir m_tempDir;
};

/******************************************************************************
 ******************************************************************************/
DownloadItem *tst_DownloadManager::createDummyJob(
        DownloadManager *downloadManager, const QString url, const QString mask)
{
    Q_ASSERT(m_tempDir.isValid());
    qDebug() << "Directory for tests: " << m_tempDir.path();
    ResourceItem* resource1 = new ResourceItem();
    resource1->setUrl(url);
    resource1->setDestination(m_tempDir.path());
    resource1->setMask(mask);
    DownloadItem *item = new DownloadItem(downloadManager);
    item->setResource(resource1);
    return item;
}

/******************************************************************************
 ******************************************************************************/
void tst_DownloadManager::appendJob()
{
    // Given
    DownloadManager *target = new DownloadManager( this);

    qRegisterMetaType<DownloadItem*>();
    QSignalSpy spyJobAppended(target, SIGNAL(jobAppended(DownloadItem*)));
    QSignalSpy spyJobRemoved(target, SIGNAL(jobRemoved(DownloadItem*)));
    QSignalSpy spyJobStateChanged(target, SIGNAL(jobStateChanged(DownloadItem*)));
    QSignalSpy spyDownloadFinished(target, SIGNAL(downloadFinished(bool)));

    DownloadItem *item = createDummyJob(
                target,
                "https://avatars3.githubusercontent.com/u/20563751?s=460&v=4",
                "*name*.png");

    // When
    target->append(item, true);

    // Then
    QVERIFY(spyDownloadFinished.wait(5000)); // wait for 5 seconds max

    QCOMPARE(spyJobAppended.count(), 1);
    QCOMPARE(spyJobRemoved.count(), 0);
    QCOMPARE(spyJobStateChanged.count(), 6);
    QCOMPARE(spyDownloadFinished.count(), 1);

    QCOMPARE(item->state(), DownloadItem::Completed);
    QCOMPARE(item->bytesReceived(), 35493);
    QCOMPARE(item->bytesReceived(), 35493);

    QFile localFile(item->localFullFileName());
    QVERIFY(localFile.exists());
    QCOMPARE(localFile.size(), 35493);

    item->deleteLater();
    target->deleteLater();
}

/******************************************************************************
 ******************************************************************************/
void tst_DownloadManager::appendJobPaused()
{
    // Given
    DownloadManager *target = new DownloadManager( this);

    qRegisterMetaType<DownloadItem*>();
    QSignalSpy spyJobAppended(target, SIGNAL(jobAppended(DownloadItem*)));
    QSignalSpy spyJobRemoved(target, SIGNAL(jobRemoved(DownloadItem*)));
    QSignalSpy spyJobStateChanged(target, SIGNAL(jobStateChanged(DownloadItem*)));
    QSignalSpy spyDownloadFinished(target, SIGNAL(downloadFinished(bool)));

    DownloadItem *item = createDummyJob(
                target,
                "https://raw.githubusercontent.com/setvisible/nastran-pch2csv/"
                "master/doc/320px-Blue-punch-card-front-horiz.png",
                "*name*.png");

    // When
    target->append(item, false);

    // Then
    QCOMPARE(spyJobAppended.count(), 1);
    QCOMPARE(spyJobRemoved.count(), 0);
    QCOMPARE(spyJobStateChanged.count(), 0);
    QCOMPARE(spyDownloadFinished.count(), 0);

    // When
    target->resume(item);

    // Then
    QVERIFY(spyDownloadFinished.wait(5000)); // wait for 5 seconds max

    QCOMPARE(spyJobAppended.count(), 1);
    QCOMPARE(spyJobRemoved.count(), 0);
    QCOMPARE(spyJobStateChanged.count(), 7);
    QCOMPARE(spyDownloadFinished.count(), 1);

    QCOMPARE(item->state(), DownloadItem::Completed);
    QCOMPARE(item->bytesReceived(), 89588);
    QCOMPARE(item->bytesReceived(), 89588);

    QFile localFile(item->localFullFileName());
    QVERIFY(localFile.exists());
    QCOMPARE(localFile.size(), 89588);

    item->deleteLater();
    target->deleteLater();
}

/******************************************************************************
 ******************************************************************************/

/*
 * QSignalSpy::wait() requires QTEST_MAIN instead of QTEST_APPLESS_MAIN,
 * otherwise we get QEventLoop: Cannot be used without QApplication
 */
QTEST_MAIN(tst_DownloadManager)
/* QTEST_APPLESS_MAIN(tst_DownloadManager) */

#include "tst_downloadmanager.moc"
