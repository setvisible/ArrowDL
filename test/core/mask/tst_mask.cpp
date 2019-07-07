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

    QString url1 = "https://www.myweb.com/images/01/myimage.tar.gz?id=1345&lang=eng";

    /* Simples */
    QTest::newRow("simple ext") << url1 << "*ext*" << "gz";
    QTest::newRow("simple name") << url1 << "*name*" << "myimage.tar";
    QTest::newRow("simple url") << url1 << "*url*" << "www.myweb.com";
    QTest::newRow("simple curl") << url1 << "*curl*" << "www.myweb.com/images/01/myimage.tar.gz";
    QTest::newRow("simple flaturl") << url1 << "*flaturl*" << "www.myweb.com-images-01-myimage.tar.gz";
    QTest::newRow("simple subdirs") << url1 << "*subdirs*" << "images/01";
    QTest::newRow("simple flatsubdirs") << url1 << "*flatsubdirs*" << "images-01";
    QTest::newRow("simple qstring") << url1 << "*qstring*" << "id=1345&lang=eng";

    /* Composites */
    QTest::newRow("composite") << url1 << "*name*.*ext*" << "myimage.tar.gz";
    QTest::newRow("file separator /") << url1 << "*url*/*subdirs*/*name*.*ext*" << "www.myweb.com/images/01/myimage.tar.gz";
    QTest::newRow("file separator \\") << url1 << "*url*\\*subdirs*\\*name*.*ext*" << "www.myweb.com/images/01/myimage.tar.gz";

}

void tst_Mask::interpret()
{
    QFETCH(QString, url);
    QFETCH(QString, mask);
    QFETCH(QString, expected);

    QString actual = Mask::interpret(url, QString(), mask);

    QCOMPARE(actual, expected);
}


/******************************************************************************
******************************************************************************/

QTEST_APPLESS_MAIN(tst_Mask)

#include "tst_mask.moc"
