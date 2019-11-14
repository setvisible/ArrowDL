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

Q_DECLARE_OPAQUE_POINTER(IDownloadItem*)
Q_DECLARE_METATYPE(DownloadRange)

class tst_DownloadEngine : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        qRegisterMetaType<IDownloadItem*>("IDownloadItem*");
        qRegisterMetaType<DownloadRange>("DownloadRange");
    }
    void append();
};

/******************************************************************************
 ******************************************************************************/
void tst_DownloadEngine::append()
{
    // Given
    QScopedPointer<DownloadEngine> target(new DownloadEngine(this));

    QSignalSpy spyJobAppended(target.data(), SIGNAL(jobAppended(DownloadRange)));
    QSignalSpy spyJobRemoved(target.data(), &DownloadEngine::jobRemoved);
    QSignalSpy spyJobStateChanged(target.data(), &DownloadEngine::jobStateChanged);
    QSignalSpy spyJobFinished(target.data(), &DownloadEngine::jobFinished);

    const qint64 bytesTotal = 123*1024*1024;
    const qint64 timeIncrement = 150;
    const qint64 duration = 2500;

    FakeDownloadItem* item = new FakeDownloadItem(
                QUrl("http://www.example.com/favicon.png"), QLatin1String("favicon.png"),
                bytesTotal, timeIncrement, duration);

    QList<IDownloadItem*> items;
    items.append(item);

    // When
    target->append(items, false);

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
