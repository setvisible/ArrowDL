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

#include <Core/Utils>

#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtTest/QtTest>

class tst_Utils : public QObject
{
    Q_OBJECT

private slots:
    void transformToRanges_ascending();
    void transformToRanges_descending();
};

void tst_Utils::transformToRanges_ascending()
{
    // [1, 2, 3, 5, 6, 8, 9] must be transformed into ["1-3", "5-6", "8-9"].
    bool ascending = true;
    QList<int> input = {1, 2, 3, 5, 6, 8, 9};
    QList<QPair<int, int> > expected;
    expected << qMakePair(1, 3)
             << qMakePair(5, 6)
             << qMakePair(8, 9);

    auto actual = Utils::transformToRanges(input, ascending);
    QCOMPARE(actual, expected);
}

void tst_Utils::transformToRanges_descending()
{
    // [9, 8, 6, 5, 3, 2, 1] must be transformed into ["9-8", "6-5", "3-1"].
    bool ascending = false;
    QList<int> input = {9, 8, 6, 5, 3, 2, 1};
    QList<QPair<int, int> > expected;
    expected << qMakePair(9, 8)
             << qMakePair(6, 5)
             << qMakePair(3, 1);

    auto actual = Utils::transformToRanges(input, ascending);
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/
QTEST_APPLESS_MAIN(tst_Utils)

#include "tst_utils.moc"

