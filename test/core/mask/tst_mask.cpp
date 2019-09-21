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

#include <Core/Mask>

#include <QtTest/QtTest>
#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

class tst_Mask : public QObject
{
    Q_OBJECT

private slots:
    void interpret_data();
    void interpret();
};

/******************************************************************************
******************************************************************************/
void tst_Mask::interpret_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QString>("mask");
    QTest::addColumn<QString>("expected");

    const QString url = "https://www.myweb.com/images/01/myimage.tar.gz?id=1345&lang=eng";
    const QString mask = "*url*/*subdirs*/*name*.*ext*";

    /* Simples */
    QTest::newRow("simple ext") << url << "*ext*" << "gz";
    QTest::newRow("simple name") << url << "*name*" << "myimage.tar";
    QTest::newRow("simple url") << url << "*url*" << "www.myweb.com";
    QTest::newRow("simple curl") << url << "*curl*" << "www.myweb.com/images/01/myimage.tar.gz";
    QTest::newRow("simple flaturl") << url << "*flaturl*" << "www.myweb.com-images-01-myimage.tar.gz";
    QTest::newRow("simple subdirs") << url << "*subdirs*" << "images/01";
    QTest::newRow("simple flatsubdirs") << url << "*flatsubdirs*" << "images-01";
    QTest::newRow("simple qstring") << url << "*qstring*" << "id=1345&lang=eng";

    /* Composites */
    QTest::newRow("composite") << url << "*name*.*ext*" << "myimage.tar.gz";
    QTest::newRow("file separator /") << url << "*url*/*subdirs*/*name*.*ext*" << "www.myweb.com/images/01/myimage.tar.gz";
    QTest::newRow("file separator \\") << url << "*url*\\*subdirs*\\*name*.*ext*" << "www.myweb.com/images/01/myimage.tar.gz";

    /* Limit cases */
    QTest::newRow("no file")   << "https://www.myweb.com/images/" << mask << "www.myweb.com/images";
    QTest::newRow("no suffix") << "https://www.myweb.com/images/myimage" << mask << "www.myweb.com/images/myimage";
    QTest::newRow("no subdir") << "https://www.myweb.com/myimage.png" << mask << "www.myweb.com/myimage.png";
    QTest::newRow("no host")   << "https://myimage.png" << mask << "myimage.png";
    QTest::newRow("no prefix") << "myimage.png" << mask << "myimage.png";
    QTest::newRow("no host/suffix") << "image" << mask << "image";
    QTest::newRow("no basename") << "https://www.myweb.com/.image" << mask << "www.myweb.com/.image";
}

void tst_Mask::interpret()
{
    QFETCH(QString, url);
    QFETCH(QString, mask);
    QFETCH(QString, expected);

    const QString actual = Mask::interpret(url, QString(), mask);

    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/

QTEST_APPLESS_MAIN(tst_Mask)

#include "tst_mask.moc"
