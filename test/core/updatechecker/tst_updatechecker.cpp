/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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

#include "../../../src/core/updatechecker_p.h"

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_UpdateChecker : public QObject
{
    Q_OBJECT

private slots:
    void addressMatcher_data();
    void addressMatcher();
};

/******************************************************************************
******************************************************************************/
void tst_UpdateChecker::addressMatcher_data()
{
    QTest::addColumn<bool>("isHost64bit");
    QTest::addColumn<bool>("expected");
    QTest::addColumn<QString>("url");

    QTest::newRow("null") << false << false << QString();
    QTest::newRow("null") << true << false << QString();
    QTest::newRow("empty") << false << false << "";
    QTest::newRow("empty") << true << false << "";

#if defined _WIN32
    /* 32-bit */
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/myimage.png";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/1ds3tr1f2dfg";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownRightNow_chromium_v1.6.1.zip";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownRightNow_firefox_v1.6.1.xpi";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_v1.6.1_windows-mingw-x64.zip";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_v1.6.1_windows-mingw-x86.zip";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_v1.6.1_windows-msvc-x64.zip";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_v1.6.1_windows-msvc-x86.zip";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_v1.6.1_x86_64-linux-gnu.tar.gz";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_x64_Setup.exe";
    QTest::newRow("") << false << true  << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_x86_Setup.exe";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/archive/v1.6.1.zip";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/archive/v1.6.1.tar.gz";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/DownRightNow_chromium_v1.6.1.zip";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/DownRightNow_firefox_v1.6.1.xpi";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/DownZemAll_v1.6.1_windows-mingw-x64.zip";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/DownZemAll_v1.6.1_windows-mingw-x86.zip";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/DownZemAll_v1.6.1_windows-msvc-x64.zip";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/DownZemAll_v1.6.1_windows-msvc-x86.zip";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/DownZemAll_v1.6.1_x86_64-linux-gnu.tar.gz";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/DownZemAll_x64_Setup.exe";
    QTest::newRow("") << false << true  << "/setvisible/DownZemAll/releases/DownZemAll_x86_Setup.exe";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/v1.6.1.zip";
    QTest::newRow("") << false << false << "/setvisible/DownZemAll/releases/v1.6.1.tar.gz";

    /* 64-bit */
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/myimage.png";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/1ds3tr1f2dfg";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownRightNow_chromium_v1.6.1.zip";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownRightNow_firefox_v1.6.1.xpi";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_v1.6.1_windows-mingw-x64.zip";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_v1.6.1_windows-mingw-x86.zip";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_v1.6.1_windows-msvc-x64.zip";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_v1.6.1_windows-msvc-x86.zip";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_v1.6.1_x86_64-linux-gnu.tar.gz";
    QTest::newRow("") << true << true  << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_x64_Setup.exe";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/download/v1.6.1/DownZemAll_x86_Setup.exe";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/archive/v1.6.1.zip";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/archive/v1.6.1.tar.gz";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/DownRightNow_chromium_v1.6.1.zip";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/DownRightNow_firefox_v1.6.1.xpi";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/DownZemAll_v1.6.1_windows-mingw-x64.zip";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/DownZemAll_v1.6.1_windows-mingw-x86.zip";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/DownZemAll_v1.6.1_windows-msvc-x64.zip";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/DownZemAll_v1.6.1_windows-msvc-x86.zip";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/DownZemAll_v1.6.1_x86_64-linux-gnu.tar.gz";
    QTest::newRow("") << true << true  << "/setvisible/DownZemAll/releases/DownZemAll_x64_Setup.exe";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/DownZemAll_x86_Setup.exe";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/v1.6.1.zip";
    QTest::newRow("") << true << false << "/setvisible/DownZemAll/releases/v1.6.1.tar.gz";

#elif defined __APPLE__
    /// \todo
#else
    /// \todo
#endif
}

void tst_UpdateChecker::addressMatcher()
{
    QFETCH(bool, isHost64bit);
    QFETCH(bool, expected);
    QFETCH(QString, url);

    auto addressMatcher = UpdateCheckerNS::addressMatcher(isHost64bit);
    auto actual = addressMatcher(url);

    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/
QTEST_APPLESS_MAIN(tst_UpdateChecker)

#include "tst_updatechecker.moc"
