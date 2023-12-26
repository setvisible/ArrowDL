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
    void initTestCase();

    void append();

    void do_not_move();
    void moveCurrentTop();
    void moveCurrentUp();
    void moveCurrentDown();
    void moveCurrentBottom();
};

void tst_DownloadEngine::initTestCase()
{
    qRegisterMetaType<IDownloadItem*>("IDownloadItem*");
    qRegisterMetaType<DownloadRange>("DownloadRange");
}

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

    const qsizetype bytesTotal = 123*1024*1024;
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
static void VERIFY_ORDER(const QScopedPointer<DownloadEngine> &engine, QList<int> indexes)
{
    auto items = engine->downloadItems();
    if (engine->downloadItems().size() != indexes.size()) {
        QFAIL("Sizes must be the same");
    }
    for (auto i = 0; i < indexes.size(); ++i) {
        auto expected = QString("item %0").arg(indexes.at(i));
        auto actual = items.at(i)->localFileName();
        if (actual != expected) {
            auto message = QString("Items at index %0 are different: expected=<%1> actual=<%2>")
                    .arg(i).arg(expected, actual);
            QFAIL(message.toUtf8());
        }
    }
    QVERIFY(true);
}

static QList<IDownloadItem*> createDummyList()
{
    QList<IDownloadItem*> items;
    for (int i = 0; i < 10; ++i) {
        auto item = new FakeDownloadItem(QString("item %0").arg(i));
        items.append(item);
    }
    return items;
}

static void select(const QScopedPointer<DownloadEngine> &engine, QList<int> indexes)
{
    Q_ASSERT(!engine.isNull());
    QList<IDownloadItem*> selection;
    for (auto i = 0; i < indexes.size(); ++i) {
        auto index = indexes.at(i);
        selection.append(engine->downloadItems().at(index));
    }
    engine->setSelection(selection);
}

/******************************************************************************
 ******************************************************************************/
/*
 * This test verifies that the static test methods hereabove have no bug.
 */
void tst_DownloadEngine::do_not_move()
{
    // Given, When
    QScopedPointer<DownloadEngine> target(new DownloadEngine(this));
    target->append(createDummyList(), false);
    select(target, QList<int>({7, 4, 2, 6}));

    // Then
    VERIFY_ORDER(target, QList<int>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

/******************************************************************************
 ******************************************************************************/
void tst_DownloadEngine::moveCurrentTop()
{
    // Given
    QScopedPointer<DownloadEngine> target(new DownloadEngine(this));
    target->append(createDummyList(), false);
    select(target, QList<int>({7, 4, 2, 6}));

    // When
    target->moveCurrentTop();

    // Then
    VERIFY_ORDER(target, QList<int>({2, 4, 6, 7, 0, 1, 3, 5, 8, 9}));
}

void tst_DownloadEngine::moveCurrentUp()
{
    // Given
    QScopedPointer<DownloadEngine> target(new DownloadEngine(this));
    target->append(createDummyList(), false);
    select(target, QList<int>({7, 4, 2, 6}));

    // When
    target->moveCurrentUp();

    // Then
    VERIFY_ORDER(target, QList<int>({0, 2, 4, 6, 7, 1, 3, 5, 8, 9}));
}

void tst_DownloadEngine::moveCurrentDown()
{
    // Given
    QScopedPointer<DownloadEngine> target(new DownloadEngine(this));
    target->append(createDummyList(), false);
    select(target, QList<int>({7, 4, 2, 6}));

    // When
    target->moveCurrentDown();

    // Then
    VERIFY_ORDER(target, QList<int>({0, 1, 3, 5, 8, 2, 4, 6, 7, 9}));
}

void tst_DownloadEngine::moveCurrentBottom()
{
    // Given
    QScopedPointer<DownloadEngine> target(new DownloadEngine(this));
    target->append(createDummyList(), false);
    select(target, QList<int>({7, 4, 2, 6}));

    // When
    target->moveCurrentBottom();

    // Then
    VERIFY_ORDER(target, QList<int>({0, 1, 3, 5, 8, 9, 2, 4, 6, 7}));
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
