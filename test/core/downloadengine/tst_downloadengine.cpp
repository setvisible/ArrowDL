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

#include "../../utils/fakedownloaditem.h"

#include <Core/IDownloadItem>
#include <Core/DownloadEngine>

#include <QtCore/QDebug>
#include <QtCore/QUrl>

#include <QtTest/QSignalSpy>
#include <QtTest/QtTest>


class tst_DownloadEngine : public QObject
{
    Q_OBJECT

private slots:
    void append();
};

/******************************************************************************
 ******************************************************************************/
void tst_DownloadEngine::append()
{
    // Given
    DownloadEngine *target = new DownloadEngine(this);

    qRegisterMetaType<FakeDownloadItem*>();

    QSignalSpy spyJobAppended(target, SIGNAL(jobAppended(IDownloadItem*)));
    QSignalSpy spyJobRemoved(target, SIGNAL(jobRemoved(IDownloadItem*)));
    QSignalSpy spyJobStateChanged(target, SIGNAL(jobStateChanged(IDownloadItem*)));
    QSignalSpy spyJobFinished(target, SIGNAL(jobFinished(IDownloadItem*)));

    const qint64 bytesTotal = 123*1024*1024;
    const qint64 timeIncrement = 150;
    const qint64 duration = 2500;

    FakeDownloadItem *item = new FakeDownloadItem(
                QUrl("http://www.example.com/favicon.png"), QLatin1String("favicon.png"),
                bytesTotal, timeIncrement, duration);

    // When
    target->append(item, false);

    // Then
    QCOMPARE(spyJobAppended.count(), 1);
    QCOMPARE(spyJobRemoved.count(), 0);
    QCOMPARE(spyJobStateChanged.count(), 1); // Paused
    QCOMPARE(spyJobFinished.count(), 0);

    // When
    target->resume(item);

    // Then
    QVERIFY(spyJobFinished.wait(5000)); // wait for 5 seconds max

    QCOMPARE(spyJobAppended.count(), 1);
    QCOMPARE(spyJobRemoved.count(), 0);
    QCOMPARE(spyJobFinished.count(), 1);

    QCOMPARE(item->state(), IDownloadItem::Completed);
    QCOMPARE(item->bytesReceived(), bytesTotal);
    QCOMPARE(item->bytesTotal(), bytesTotal);

    item->deleteLater();
    target->deleteLater();
}

/******************************************************************************
 ******************************************************************************/

/*
 * QSignalSpy::wait() requires QTEST_MAIN instead of QTEST_APPLESS_MAIN,
 * otherwise we get QEventLoop: Cannot be used without QApplication
 */
QTEST_MAIN(tst_DownloadEngine)
/* QTEST_APPLESS_MAIN(tst_DownloadEngine) */

#include "tst_downloadengine.moc"
