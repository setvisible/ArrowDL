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

#include <Core/Regex>

#include <QtTest/QtTest>
#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

class tst_Regex : public QObject
{
    Q_OBJECT

private slots:
    void interpret_data();
    void interpret();
};

/******************************************************************************
******************************************************************************/
void tst_Regex::interpret_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QStringList>("expected");


    QTest::newRow("no regex")
            << "https://www.myweb.com/images/01/myimage.png"
            << QStringList{ "https://www.myweb.com/images/01/myimage.png" };

    QTest::newRow("simple :")
            << "https://www.myweb.com/images/[01:03]/myimage.png"
            << QStringList{
               "https://www.myweb.com/images/01/myimage.png",
               "https://www.myweb.com/images/02/myimage.png",
               "https://www.myweb.com/images/03/myimage.png"};

    QTest::newRow("simple -")
            << "https://www.myweb.com/images/[01-03]/myimage.png"
            << QStringList{
               "https://www.myweb.com/images/01/myimage.png",
               "https://www.myweb.com/images/02/myimage.png",
               "https://www.myweb.com/images/03/myimage.png"};


}

void tst_Regex::interpret()
{
    QFETCH(QString, url);
    QFETCH(QStringList, expected);

    QStringList actual = Regex::interpret(url);

    QCOMPARE(actual, expected);
}


/******************************************************************************
******************************************************************************/

QTEST_APPLESS_MAIN(tst_Regex)

#include "tst_regex.moc"
