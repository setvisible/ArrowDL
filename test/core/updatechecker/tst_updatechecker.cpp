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

        QTest::newRow("null") << QString() << QString();
        QTest::newRow("null") << "" << QString();
        QTest::newRow("null") << "" << "";

        QTest::newRow("equal") << "2.5.0" << "2.5.0";
        QTest::newRow("equal") << "2.5.0" << "  2.5.0  ";

        QTest::newRow("dot") << "2.5.0" << "...2.5.0...";
        QTest::newRow("dot") << "2.5.0" << "...2.,  ,5..Aaa..0";
        QTest::newRow("dot") << "2.5.0.1254" << "version 2.5.0 build 1254";

        QTest::newRow("prefix") << "2.5.0" << "v2.5.0";
        QTest::newRow("prefix") << "2.5.0" << "v_2.5.0";
        QTest::newRow("prefix") << "2.5.0" << "v.2.5.0";
        QTest::newRow("prefix") << "2.5.0" << "v 2.5.0";
        QTest::newRow("prefix") << "2.5.0" << "version 2.5.0";
        QTest::newRow("prefix") << "2.5.0" << "  version 2.5.0 bis ";
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

        QTest::newRow("null") << false << QString() << QString();
        QTest::newRow("null") << false << "" << QString();
        QTest::newRow("null") << false << "" << "";

        QTest::newRow("equal") << false << "2.5.0" << "2.5.0";
        QTest::newRow("number") << false << "2.5.0" << "2.5.1";
        QTest::newRow("number") << true << "2.5.1" << "2.5.0";

        // Sort versions with different length:
        // "2" < "2.0" < "2.4" <"2.4.0"
        QTest::newRow("different length") << true << "2.0" << "2";
        QTest::newRow("different length") << true << "2.4" << "2.0";
        QTest::newRow("different length") << true << "2.4.0" << "2.4";
        QTest::newRow("different length") << true << "2.5" << "2.4.9";

        // Bugfix QCollator when no ICU is available
        QTest::newRow("QCollator 100 > 99") << true << "2.5.100" << "2.5.99";
        QTest::newRow("QCollator 99 > 10 ") << true << "2.5.99" << "2.5.10";
        QTest::newRow("QCollator 099 > 10 ") << true << "2.5.099" << "2.5.10";

        QTest::newRow("prefix") << true << "2.5.1" << "v2.5.0";
        QTest::newRow("prefix") << true << "2.5.1" << "v_2.5.0";
        QTest::newRow("prefix") << true << "2.5.1" << "v.2.5.0";
        QTest::newRow("prefix") << true << "2.5.1" << "v 2.5.0";
        QTest::newRow("prefix") << true << "2.5.1" << "version 2.5.0";

        QTest::newRow("prefix") << true << "v2.5.1" << "2.5.0";
        QTest::newRow("prefix") << true << "v_2.5.1" << "2.5.0";
        QTest::newRow("prefix") << true << "v.2.5.1" << "2.5.0";
        QTest::newRow("prefix") << true << "v 2.5.1" << "2.5.0";
        QTest::newRow("prefix") << true << "version 2.5.1" << "2.5.0";
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
