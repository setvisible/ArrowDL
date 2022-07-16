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

#include <Core/DownloadManager>
#include <Core/DownloadItem>
#include <Core/Mask>
#include <Core/ResourceItem>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QThread>
#include <QtCore/QCoreApplication>
#include <QtCore/QTemporaryDir>
#include <QtTest/QSignalSpy>
#include <QtTest/QtTest>

Q_DECLARE_OPAQUE_POINTER(IDownloadItem*)


void hideQDebugMessage(QtMsgType, const QMessageLogContext &, const QString &)
{
    /*
     * Do nothing: just hide QDebug messages,
     * to diminish visual pollution in the test
     */
}

class tst_DownloadManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        qRegisterMetaType<IDownloadItem*>("IDownloadItem*");
        qInstallMessageHandler(hideQDebugMessage);
    }

    void appendJobPaused();

private:
    QTemporaryDir m_tempDir;

    inline DownloadItem *createDummyJob(QSharedPointer<DownloadManager> downloadManager,
                                        const QString url, const QString mask);
};

/******************************************************************************
 ******************************************************************************/
DownloadItem *tst_DownloadManager::createDummyJob(
        QSharedPointer<DownloadManager> downloadManager,
        const QString url, const QString mask)
{
    Q_ASSERT(m_tempDir.isValid());
    qDebug() << "Directory for tests: " << m_tempDir.path();
    ResourceItem* resource = new ResourceItem();
    resource->setUrl(url);
    resource->setDestination(m_tempDir.path());
    resource->setMask(mask);
    DownloadItem *item = new DownloadItem(downloadManager.data());
    item->setResource(resource);
    return item;
}

/******************************************************************************
 ******************************************************************************/
void tst_DownloadManager::appendJobPaused()
{
    // Given
    QSharedPointer<DownloadManager> target(new DownloadManager(this));

    QSignalSpy spyJobFinished(target.data(), SIGNAL(jobFinished(IDownloadItem*)));

    /* The most permanent url in the whole universe */
    // QString address = "https://setvisible.github.io/styles.css"; // css not png

    /// \todo fix IDownloadItem::NetworkError with "3xx Unknown redirect error" with SSL url

    /* Non SSL url */
    QString address = "http://www.example.com/index.html"; // 'http' instead of 'https'

    QList<IDownloadItem*> items;
    DownloadItem *item = createDummyJob(target, address, "*name*.png");
    items.append(item);

    // When
    target->append(items, false);
    target->resume(item);

    // Then
    QVERIFY2(spyJobFinished.wait(5000),
             QString("\n\nConnection Timeout\nCan't reach:\n%0\n\n")
             .arg(address).toStdString().c_str()); // wait for 5 seconds max

    QCOMPARE(spyJobFinished.count(), 1);

    QCOMPARE(item->state(), DownloadItem::Completed);
    QCOMPARE(item->bytesReceived(), 1256);
    QCOMPARE(item->bytesTotal(), 1256);

    QFile localFile(item->localFullFileName());
    QVERIFY(localFile.exists());
    QCOMPARE(localFile.size(), 1256);
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
