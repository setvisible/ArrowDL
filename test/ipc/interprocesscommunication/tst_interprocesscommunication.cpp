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

#include <Ipc/InterProcessCommunication>

#include <QtCore/QDebug>
#include <QtTest/QtTest>


class tst_InterProcessCommunication : public QObject
{
    Q_OBJECT

private slots:
    void clean_data();
    void clean();
};

/******************************************************************************
******************************************************************************/
void tst_InterProcessCommunication::clean_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("")
            << "https://www.myweb.com/myimage.tar.gz"
            << "https://www.myweb.com/myimage.tar.gz";

    QTest::newRow("percent decoding")
            << "https://www.myweb.com/images/01/myimage.tar.gz"
            << "https://www.myweb.com/images/01/myimage.tar.gz";

    QTest::newRow("extra quotes and spaces")
            << "\t\"  https://www.myweb.com/images/01/myimage.tar.gz \"  \""
            << "https://www.myweb.com/images/01/myimage.tar.gz";
}

void tst_InterProcessCommunication::clean()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);
    QString actual = InterProcessCommunication::clean(input);
    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/
QTEST_APPLESS_MAIN(tst_InterProcessCommunication)

#include "tst_interprocesscommunication.moc"
