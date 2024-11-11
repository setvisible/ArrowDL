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

#include <Widgets/TextEdit>

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_TextEdit : public QObject
{
    Q_OBJECT

private slots:
    void fragmentToPaste_data();
    void fragmentToPaste();
};

/******************************************************************************
 ******************************************************************************/
void tst_TextEdit::fragmentToPaste_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("empty") << "" << "";
    QTest::newRow("trivial") << "Hello World" << "Hello World";

    QTest::newRow("multiple line RN 1") << "\r\nHello World" << "";
    QTest::newRow("multiple line RN 2") << "Hello\r\n World" << "Hello";
    QTest::newRow("multiple line RN 3") << "Hello World\r\n" << "Hello World";

    QTest::newRow("multiple line R 1") << "\rHello World" << "";
    QTest::newRow("multiple line R 2") << "Hello\r World" << "Hello";
    QTest::newRow("multiple line R 3") << "Hello World\r" << "Hello World";

    QTest::newRow("multiple line N 1") << "\nHello World" << "";
    QTest::newRow("multiple line N 2") << "Hello\n World" << "Hello";
    QTest::newRow("multiple line N 3") << "Hello World\n" << "Hello World";
}

void tst_TextEdit::fragmentToPaste()
{
    TextEdit target;
    QFETCH(QString, input);
    QFETCH(QString, expected);
    QString actual = target.fragmentToPaste(input);
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/

QTEST_MAIN(tst_TextEdit)

#include "tst_textedit.moc"
