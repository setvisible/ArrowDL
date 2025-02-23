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

#include "utils.h"

#include <QtCore/QDebug>


/*!
 * \brief Transform a list of int into list of ranges.
 * [1, 2, 3, 5, 6, 8, 9] will be transformed into ["1-3", "5-6", "8-9"].
 */
QList<QPair<int, int> > Utils::transformToRanges(QList<int> nums, bool ascending)
{
    QList<QPair<int, int> > ranges;
    if (nums.empty()) {
        return ranges;
    }
    int start = nums.at(0);
    int end = nums.at(0);
    for (int i = 1; i < nums.count(); ++i) {
        if (nums.at(i) == end + (ascending ? 1 : -1)) {
            // If the number is consecutive, extend the range
            end = nums.at(i);
        } else {
            // If the sequence breaks, push the previous range
            ranges.append(qMakePair(start, end));
            start = nums.at(i);
            end = nums.at(i);
        }
    }
    // Add the last range
    ranges.append(qMakePair(start, end));
    return ranges;
}
