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

#include "../../utils/fakedownloaditem.h"

#include <Core/DownloadManager>
#include <Core/DownloadFileItem>
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

Q_DECLARE_OPAQUE_POINTER(AbstractDownloadItem*)
Q_DECLARE_METATYPE(DownloadRange)


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
    void initTestCase();

    void append();

    void do_not_move();
    void moveCurrentTop();
    void moveCurrentUp();
    void moveCurrentDown();
    void moveCurrentBottom();

    // void appendJobPaused();

private:
    QTemporaryDir m_tempDir;

    inline DownloadFileItem *createDummyJob(QSharedPointer<DownloadManager> downloadManager, const QString url, const QString mask);
};

void tst_DownloadManager::initTestCase()
{
    qRegisterMetaType<AbstractDownloadItem*>("AbstractDownloadItem*");
    qRegisterMetaType<DownloadRange>("DownloadRange");
    qInstallMessageHandler(hideQDebugMessage);
}

DownloadFileItem *tst_DownloadManager::createDummyJob(
        QSharedPointer<DownloadManager> downloadManager,
        const QString url, const QString mask)
{
    Q_ASSERT(m_tempDir.isValid());
    // qDebug() << "Directory for tests: " << m_tempDir.path();
    ResourceItem* resource = new ResourceItem();
    resource->setUrl(url);
    resource->setDestination(m_tempDir.path());
    resource->setMask(mask);
    DownloadFileItem *item = new DownloadFileItem(downloadManager.data());
    item->setResource(resource);
    return item;
}

/******************************************************************************
 ******************************************************************************/
void tst_DownloadManager::append()
{
    // Given
    QScopedPointer<DownloadManager> target(new DownloadManager(this));

    QSignalSpy spyJobAppended(target.data(), SIGNAL(jobAppended(DownloadRange)));
    QSignalSpy spyJobRemoved(target.data(), &DownloadManager::jobRemoved);
    QSignalSpy spyJobStateChanged(target.data(), &DownloadManager::jobStateChanged);
    QSignalSpy spyJobFinished(target.data(), &DownloadManager::jobFinished);

    const qsizetype bytesTotal = 123*1024*1024;
    const qint64 timeIncrement = 150;
    const qint64 duration = 2500;

    FakeDownloadItem* item = new FakeDownloadItem(
                QUrl("http://www.example.com/favicon.png"), QLatin1String("favicon.png"),
                bytesTotal, timeIncrement, duration);

    QList<AbstractDownloadItem*> items;
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

    QCOMPARE(item->state(), AbstractDownloadItem::Completed);
    QCOMPARE(item->bytesReceived(), bytesTotal);
    QCOMPARE(item->bytesTotal(), bytesTotal);
}

/******************************************************************************
 ******************************************************************************/
static void VERIFY_ORDER(const QScopedPointer<DownloadManager> &downloadManager, QList<int> indexes)
{
    auto items = downloadManager->downloadItems();
    if (downloadManager->downloadItems().size() != indexes.size()) {
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

static QList<AbstractDownloadItem*> createDummyList()
{
    QList<AbstractDownloadItem*> items;
    for (int i = 0; i < 10; ++i) {
        auto item = new FakeDownloadItem(QString("item %0").arg(i));
        items.append(item);
    }
    return items;
}

static void select(const QScopedPointer<DownloadManager> &downloadManager, QList<int> indexes)
{
    Q_ASSERT(!downloadManager.isNull());
    QList<AbstractDownloadItem*> selection;
    for (auto i = 0; i < indexes.size(); ++i) {
        auto index = indexes.at(i);
        selection.append(downloadManager->downloadItems().at(index));
    }
    downloadManager->setSelection(selection);
}

/******************************************************************************
 ******************************************************************************/
/*
 * This test verifies that the static test methods hereabove have no bug.
 */
void tst_DownloadManager::do_not_move()
{
    // Given, When
    QScopedPointer<DownloadManager> target(new DownloadManager(this));
    target->append(createDummyList(), false);
    select(target, QList<int>({7, 4, 2, 6}));

    // Then
    VERIFY_ORDER(target, QList<int>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

/******************************************************************************
 ******************************************************************************/
void tst_DownloadManager::moveCurrentTop()
{
    // Given
    QScopedPointer<DownloadManager> target(new DownloadManager(this));
    target->append(createDummyList(), false);
    select(target, QList<int>({7, 4, 2, 6}));

    // When
    target->moveCurrentTop();

    // Then
    VERIFY_ORDER(target, QList<int>({2, 4, 6, 7, 0, 1, 3, 5, 8, 9}));
}

void tst_DownloadManager::moveCurrentUp()
{
    // Given
    QScopedPointer<DownloadManager> target(new DownloadManager(this));
    target->append(createDummyList(), false);
    select(target, QList<int>({7, 4, 2, 6}));

    // When
    target->moveCurrentUp();

    // Then
    VERIFY_ORDER(target, QList<int>({0, 2, 4, 6, 7, 1, 3, 5, 8, 9}));
}

void tst_DownloadManager::moveCurrentDown()
{
    // Given
    QScopedPointer<DownloadManager> target(new DownloadManager(this));
    target->append(createDummyList(), false);
    select(target, QList<int>({7, 4, 2, 6}));

    // When
    target->moveCurrentDown();

    // Then
    VERIFY_ORDER(target, QList<int>({0, 1, 3, 5, 8, 2, 4, 6, 7, 9}));
}

void tst_DownloadManager::moveCurrentBottom()
{
    // Given
    QScopedPointer<DownloadManager> target(new DownloadManager(this));
    target->append(createDummyList(), false);
    select(target, QList<int>({7, 4, 2, 6}));

    // When
    target->moveCurrentBottom();

    // Then
    VERIFY_ORDER(target, QList<int>({0, 1, 3, 5, 8, 9, 2, 4, 6, 7}));
}

/******************************************************************************
 ******************************************************************************/
/// \todo this is a test for sending a real http(s) connection:
/// hard to setup, thus has been commented out for now.
// void tst_DownloadManager::appendJobPaused()
// {
//     // Given
//     QSharedPointer<DownloadManager> target(new DownloadManager(this));
//
//     QSignalSpy spyJobFinished(target.data(), SIGNAL(jobFinished(AbstractDownloadItem*)));
//
//     /* The most permanent url in the whole universe */
//     // QString address = "https://www.arrow-dl.com/favicon.ico"; // ico, not png
//
//     /// \todo fix AbstractDownloadItem::NetworkError with "3xx Unknown redirect error" with SSL url
//
//     /* Non SSL url */
//     QString address = "http://www.example.com/index.html"; // 'http' instead of 'https'
//
//     QList<AbstractDownloadItem*> items;
//     DownloadFileItem *item = createDummyJob(target, address, "*name*.png");
//     items.append(item);
//
//     // When
//     target->append(items, false);
//     target->resume(item);
//
//     // Then
//     QVERIFY2(spyJobFinished.wait(5000),
//              QString("\n\nConnection Timeout\nCan't reach:\n%0\n\n")
//              .arg(address).toStdString().c_str()); // wait for 5 seconds max
//
//     QCOMPARE(spyJobFinished.count(), 1);
//
//     QCOMPARE(item->state(), DownloadFileItem::Completed);
//     QCOMPARE(item->bytesReceived(), qsizetype(1256));
//     QCOMPARE(item->bytesTotal(), qsizetype(1256));
//
//     QFile localFile(item->localFullFileName());
//     QVERIFY(localFile.exists());
//     QCOMPARE(localFile.size(), qsizetype(1256));
// }

/******************************************************************************
 ******************************************************************************/

/*
 * QSignalSpy::wait() requires QTEST_MAIN instead of QTEST_APPLESS_MAIN,
 * otherwise we get QEventLoop: Cannot be used without QApplication
 */
QTEST_MAIN(tst_DownloadManager)
/* QTEST_APPLESS_MAIN(tst_DownloadManager) */

#include "tst_downloadmanager.moc"
