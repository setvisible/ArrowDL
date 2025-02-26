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

#ifndef TORRENT_SETTING_ITEM_H
#define TORRENT_SETTING_ITEM_H

#include <QtCore/QString>
#include <QtCore/QVariant>

struct TorrentSettingItem
{
    QString displayKey;
    QString key;
    QVariant value;
    QVariant defaultValue;
};

#endif // TORRENT_SETTING_ITEM_H
