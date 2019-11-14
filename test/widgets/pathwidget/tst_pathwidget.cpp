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

#include <Widgets/PathWidget>

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_PathWidget : public QObject
{
    Q_OBJECT

private slots:
    void setCurrentPath();
    void setPathHistory();
};

/******************************************************************************
 ******************************************************************************/
void tst_PathWidget::setCurrentPath()
{
    PathWidget target;
    const QLatin1String input("test path 1");
    target.setCurrentPath(input);

    const QString actual = target.currentPath();

    QCOMPARE(actual, input);
}

/******************************************************************************
 ******************************************************************************/
void tst_PathWidget::setPathHistory()
{
    PathWidget target;
    const QLatin1String input("item 1");
    target.setCurrentPath(input);

    QStringList inputList;
    inputList << "item 1" << "item 2" << "item 3";
    target.setPathHistory(inputList);

    const QString expected = input;

    const QString actual = target.currentPath();
    const QStringList actualList = target.pathHistory();

    QCOMPARE(actual, expected);
    QCOMPARE(actualList, inputList);
}

/******************************************************************************
 ******************************************************************************/

QTEST_MAIN(tst_PathWidget)

#include "tst_pathwidget.moc"
