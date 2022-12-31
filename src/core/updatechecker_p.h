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

#include <QtCore/QCollator>
#include <QtCore/QRegularExpression>
#include <QtCore/QString>

namespace UpdateCheckerNS
{

QString cleanTag(const QString &tag)
{
    auto cleaned = tag;
    cleaned = cleaned.replace('.', ' ');
    cleaned.remove(QRegularExpression("[^\\d\\s]"));
    cleaned = cleaned.simplified();
    cleaned = cleaned.replace(' ', '.');
    return cleaned;
}

bool isVersionGreaterThan(const QString &s1, const QString &s2)
{
    auto v1 = UpdateCheckerNS::cleanTag(s1);
    auto v2 = UpdateCheckerNS::cleanTag(s2);
    QCollator collator;
    collator.setNumericMode(true); // 10 sorts after 9
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    return collator.compare(v1, v2) > 0; // v1 > v2
}

} // end namespace

#endif // CORE_UPDATE_CHECKER_PRIVATE_H
