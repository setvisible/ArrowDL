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

#include <Core/TorrentBaseContext>

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_TorrentBaseContext : public QObject
{
    Q_OBJECT

private slots:
    void computePriority_data();
    void computePriority();
};

/******************************************************************************
******************************************************************************/
void tst_TorrentBaseContext::computePriority_data()
{
    QTest::addColumn<int>("fileIndex");
    QTest::addColumn<int>("fileCount");
    QTest::addColumn<int>("expected");

    QTest::newRow("unique item") << 0 << 1 << static_cast<int>(TorrentFileInfo::Normal);

    QTest::newRow("2 items (item 0)") << 0 << 2 << static_cast<int>(TorrentFileInfo::Normal);
    QTest::newRow("2 items (item 1)") << 1 << 2 << static_cast<int>(TorrentFileInfo::Normal);

    QTest::newRow("9 items (item 0)") << 0 << 9 << static_cast<int>(TorrentFileInfo::High);
    QTest::newRow("9 items (item 1)") << 1 << 9 << static_cast<int>(TorrentFileInfo::High);
    QTest::newRow("9 items (item 2)") << 2 << 9 << static_cast<int>(TorrentFileInfo::High);
    QTest::newRow("9 items (item 3)") << 3 << 9 << static_cast<int>(TorrentFileInfo::Normal);
    QTest::newRow("9 items (item 4)") << 4 << 9 << static_cast<int>(TorrentFileInfo::Normal);
    QTest::newRow("9 items (item 5)") << 5 << 9 << static_cast<int>(TorrentFileInfo::Normal);
    QTest::newRow("9 items (item 6)") << 6 << 9 << static_cast<int>(TorrentFileInfo::Low);
    QTest::newRow("9 items (item 7)") << 7 << 9 << static_cast<int>(TorrentFileInfo::Low);
    QTest::newRow("9 items (item 8)") << 8 << 9 << static_cast<int>(TorrentFileInfo::Low);
}

void tst_TorrentBaseContext::computePriority()
{
    // Given
    QFETCH(int, fileIndex);
    QFETCH(int, fileCount);
    QFETCH(int, expected);

    // When
    TorrentBaseContext target;
    TorrentFileInfo::Priority actual = target.computePriority(fileIndex, fileCount);

    // Then
    QCOMPARE(actual, static_cast<TorrentFileInfo::Priority>(expected));
}

/******************************************************************************
******************************************************************************/
QTEST_APPLESS_MAIN(tst_TorrentBaseContext)

#include "tst_torrentbasecontext.moc"
