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

#include <Torrent/WorkerThread>

#include <QtCore/QDebug>
#include <QtTest/QtTest>

using namespace Qt::Literals::StringLiterals;

class tst_WorkerThread : public QObject
{
    Q_OBJECT

private slots:
    void dump_invalid();
};

//class FriendlyWorkerThread : public WorkerThread
//{
//    friend class tst_WorkerThread;
//public:
//    explicit FriendlyWorkerThread(QObject *parent) : WorkerThread(parent) {}
//};
//
/******************************************************************************
 ******************************************************************************/
void tst_WorkerThread::dump_invalid()
{
    // Given
    auto torrentFile("./data/ill-formed.torrent"_L1);
    QVERIFY(QFileInfo::exists(torrentFile));

    TorrentInitialMetaInfo expected;

    // When
    //FriendlyWorkerThread target(this);
    WorkerThread target(this);
    TorrentInitialMetaInfo actual = target.dump(torrentFile);

    // Then
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/
QTEST_APPLESS_MAIN(tst_WorkerThread)

#include "tst_workerthread.moc"
