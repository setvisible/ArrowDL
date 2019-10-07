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

#include <QtCore/QFile>
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
void tst_DownloadManager::appendJobPaused()
{
    // Given
    DownloadManager *target = new DownloadManager(this);

    qRegisterMetaType<DownloadItem*>();

    QSignalSpy spyJobFinished(target, SIGNAL(jobFinished(IDownloadItem*)));

    QString address =
            "https://raw.githubusercontent.com/setvisible/nastran-pch2csv/"
            "master/doc/320px-Blue-punch-card-front-horiz.png";

    DownloadItem *item = createDummyJob(target, address, "*name*.png");

    // When
    target->append(item, false);
    target->resume(item);

    // Then
    QVERIFY2(spyJobFinished.wait(5000),
             QString("\n\nConnection Timeout\nCan't reach:\n%0\n\n")
             .arg(address).toStdString().c_str()); // wait for 5 seconds max

    QCOMPARE(spyJobFinished.count(), 1);

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
