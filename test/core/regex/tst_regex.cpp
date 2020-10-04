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

#include <Core/Regex>

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_Regex : public QObject
{
    Q_OBJECT

private slots:
    void getCaptures_data();
    void getCaptures();

    void interpret_data();
    void interpret();
};

/******************************************************************************
******************************************************************************/
void tst_Regex::getCaptures_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QStringList>("expected");

    QTest::newRow("empty") << "" << QStringList{};

    QTest::newRow("simple") << "[01:03]" << QStringList{"[01:03]"};
    QTest::newRow("simple") << "(01:03)" << QStringList{"(01:03)"};
    QTest::newRow("simple") << "[01 03]" << QStringList{"[01 03]"};
    QTest::newRow("simple") << "(01 03)" << QStringList{"(01 03)"};
    QTest::newRow("simple") << "[01-03]" << QStringList{"[01-03]"};
    QTest::newRow("simple") << "(01-03)" << QStringList{"(01-03)"};

    QTest::newRow("multiple") << "[01:03](10 20)" << QStringList{"[01:03]", "(10 20)"};
    QTest::newRow("multiple") << "da/[01:03]/da/(10 20)/da" << QStringList{"[01:03]", "(10 20)"};

    QTest::newRow("embedded") << "[[01:03]/(10 20)]" << QStringList{"[01:03]", "(10 20)"};
    QTest::newRow("embedded") << "([01:03]/(10 20))" << QStringList{"[01:03]", "(10 20)"};

    QTest::newRow("invalid") << "01:03" << QStringList{};
    QTest::newRow("invalid") << "[01:03" << QStringList{};
    QTest::newRow("invalid") << "[123]" << QStringList{};
    QTest::newRow("invalid") << "[01:02:03]" << QStringList{};
    QTest::newRow("invalid") << "[01:02 03]" << QStringList{};
    QTest::newRow("invalid") << "[01 02 03]" << QStringList{};

    QTest::newRow("weird but valid") << "[01:03)" << QStringList{"[01:03)"};
}

void tst_Regex::getCaptures()
{
    QFETCH(QString, input);
    QFETCH(QStringList, expected);

    QStringList actual = Regex::getCaptures(input);

    QCOMPARE(actual, expected);
}

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

    QTest::newRow("simple space")
            << "https://www.myweb.com/images/[01 03]/myimage.png"
            << QStringList{
               "https://www.myweb.com/images/01/myimage.png",
               "https://www.myweb.com/images/02/myimage.png",
               "https://www.myweb.com/images/03/myimage.png"};


    QTest::newRow("simple : with parenthesis")
            << "https://www.myweb.com/images/(1:3)/myimage.png"
            << QStringList{
               "https://www.myweb.com/images/1/myimage.png",
               "https://www.myweb.com/images/2/myimage.png",
               "https://www.myweb.com/images/3/myimage.png"};

    QTest::newRow("several : and -")
            << "https://www.myweb.com/images_[1:2]/myimage_[01-03].png"
            << QStringList{
               "https://www.myweb.com/images_1/myimage_01.png",
               "https://www.myweb.com/images_1/myimage_02.png",
               "https://www.myweb.com/images_1/myimage_03.png",
               "https://www.myweb.com/images_2/myimage_01.png",
               "https://www.myweb.com/images_2/myimage_02.png",
               "https://www.myweb.com/images_2/myimage_03.png"};

    QTest::newRow("inversed") << "[10:8]" << QStringList{"8", "9", "10"};

    QTest::newRow("no http") << "image01" << QStringList{"image01"};
    QTest::newRow("no padding") << "image_[9-10]" << QStringList{"image_9", "image_10"};
    QTest::newRow("no end bracket") << "image_[00009-00010" << QStringList{"image_[00009-00010"};
    QTest::newRow("no space but %20") << "file://[01%2003].png" << QStringList{"file://[01%2003].png"};

    QTest::newRow("padding 100") << "image_[009-010]" << QStringList{"image_009", "image_010"};
    QTest::newRow("padding 10000") << "image_[00009-00010]" << QStringList{"image_00009", "image_00010"};
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
