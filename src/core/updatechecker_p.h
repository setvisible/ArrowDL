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

#ifndef CORE_UPDATE_CHECKER_PRIVATE_H
#define CORE_UPDATE_CHECKER_PRIVATE_H

#include <QtCore/QRegularExpression>
#include <QtCore/QString>

namespace UpdateCheckerNS
{

QString cleanTag(const QString &tag)
{
    static QRegularExpression re("[^\\d\\s]");
    auto cleaned = tag;
    cleaned = cleaned.replace(u'.', u' ');
    cleaned.remove(re);
    cleaned = cleaned.simplified();
    cleaned = cleaned.replace(u' ', u'.');
    return cleaned;
}

/*!
 * \brief Return true if the given string s1 representing a version is greater than the given s2.
 */
bool isVersionGreaterThan(const QString &s1, const QString &s2)
{
    auto v1 = UpdateCheckerNS::cleanTag(s1);
    auto v2 = UpdateCheckerNS::cleanTag(s2);
    // BUGFIX QCollator doesn't work on Linux system that don't have ICU
    // QCollator collator;
    // collator.setNumericMode(true); // 10 sorts after 9
    // collator.setCaseSensitivity(Qt::CaseInsensitive);
    // return collator.compare(v1, v2) > 0; // v1 > v2

    // transforms the tag into a list of integers and compare each integer.
    auto list1 = v1.split(u'.');
    auto list2 = v2.split(u'.');
    auto count = qMin(list1.count(), list2.count());
    for (auto i = 0; i < count; ++i) {
        auto d1 = list1.at(i).toInt();
        auto d2 = list2.at(i).toInt();
        if (d1 != d2) {
            return d1 > d2;
        }
    }
    return list1.count() > list2.count();
}

} // end namespace

#endif // CORE_UPDATE_CHECKER_PRIVATE_H
