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

#include "../../../src/core/updatechecker_p.h"

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_UpdateChecker : public QObject
{
    Q_OBJECT

private slots:
    void cleanTag_data();
    void cleanTag();

    void isVersionGreaterThan_data();
    void isVersionGreaterThan();
};

/******************************************************************************
 ******************************************************************************/
void tst_UpdateChecker::cleanTag_data()
{
        QTest::addColumn<QString>("expected");
        QTest::addColumn<QString>("input");

        QTest::newRow("null") << "" << QString();
        QTest::newRow("empty") << "" << "";

        QTest::newRow("equal") << "2.5.0" << "2.5.0";
        QTest::newRow("equal untrimmed") << "2.5.0" << "  2.5.0  ";

        QTest::newRow("dot before") << "2.5.0" << "...2.5.0...";
        QTest::newRow("dot inside") << "2.5.0" << "...2.,  ,5..Aaa..0";
        QTest::newRow("dot verbose") << "2.5.0.1254" << "version 2.5.0 build 1254";

        QTest::newRow("prefix v") << "2.5.0" << "v2.5.0";
        QTest::newRow("prefix v_") << "2.5.0" << "v_2.5.0";
        QTest::newRow("prefix v.") << "2.5.0" << "v.2.5.0";
        QTest::newRow("prefix v+space") << "2.5.0" << "v 2.5.0";
        QTest::newRow("prefix version") << "2.5.0" << "version 2.5.0";
        QTest::newRow("prefix untrimmed") << "2.5.0" << "  version 2.5.0 bis ";
}

void tst_UpdateChecker::cleanTag()
{
    QFETCH(QString, expected);
    QFETCH(QString, input);
    auto actual = UpdateCheckerNS::cleanTag(input);
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/
void tst_UpdateChecker::isVersionGreaterThan_data()
{
        QTest::addColumn<bool>("expected");
        QTest::addColumn<QString>("s1");
        QTest::addColumn<QString>("s2");

        QTest::newRow("null") << false << "" << QString();
        QTest::newRow("empty") << false << "" << "";

        QTest::newRow("equal") << false << "2.5.0" << "2.5.0";
        QTest::newRow("number") << false << "2.5.0" << "2.5.1";
        QTest::newRow("number inversed") << true << "2.5.1" << "2.5.0";

        // Sort versions with different length:
        // "2" < "2.0" < "2.4" <"2.4.0"
        QTest::newRow("different length 1 vs 0") << true << "2.0" << "2";
        QTest::newRow("different length 1 vs 1") << true << "2.4" << "2.0";
        QTest::newRow("different length 3 vs 2") << true << "2.4.0" << "2.4";
        QTest::newRow("different length 2 vs 3") << true << "2.5" << "2.4.9";

        // Bugfix QCollator when no ICU is available
        QTest::newRow("QCollator 100 > 99") << true << "2.5.100" << "2.5.99";
        QTest::newRow("QCollator 99 > 10 ") << true << "2.5.99" << "2.5.10";
        QTest::newRow("QCollator 099 > 10 ") << true << "2.5.099" << "2.5.10";

        QTest::newRow("prefix v") << true << "2.5.1" << "v2.5.0";
        QTest::newRow("prefix v_") << true << "2.5.1" << "v_2.5.0";
        QTest::newRow("prefix v.") << true << "2.5.1" << "v.2.5.0";
        QTest::newRow("prefix v+space") << true << "2.5.1" << "v 2.5.0";
        QTest::newRow("prefix version") << true << "2.5.1" << "version 2.5.0";

        QTest::newRow("prefix 2 v") << true << "v2.5.1" << "2.5.0";
        QTest::newRow("prefix 2 v_") << true << "v_2.5.1" << "2.5.0";
        QTest::newRow("prefix 2 v.") << true << "v.2.5.1" << "2.5.0";
        QTest::newRow("prefix 2 v+space") << true << "v 2.5.1" << "2.5.0";
        QTest::newRow("prefix 2 version") << true << "version 2.5.1" << "2.5.0";
}

void tst_UpdateChecker::isVersionGreaterThan()
{
    QFETCH(bool, expected);
    QFETCH(QString, s1);
    QFETCH(QString, s2);
    auto actual = UpdateCheckerNS::isVersionGreaterThan(s1, s2);
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/
QTEST_APPLESS_MAIN(tst_UpdateChecker)

#include "tst_updatechecker.moc"
