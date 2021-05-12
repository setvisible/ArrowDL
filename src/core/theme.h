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

#ifndef WIDGETS_THEME_H
#define WIDGETS_THEME_H

#include <QtCore/QMap>
#include <QtCore/QSize>
#include <QtCore/QVariant>

class QAbstractButton;
class QAction;
class QLabel;
class QWidget;

class Theme
{
public:
    static const QLatin1String PlatformStyle;
    static const QLatin1String IconTheme;
    static const QLatin1String ColorScheme;

    Theme();

    static QStringList availablePlatformStyles();
    static QString toPlatformStyle(int index);
    static int fromPlatformStyle(const QString &platformStyle);

    static QStringList availableIconThemes();
    static QString toIconTheme(int index);
    static int fromIconTheme(const QString &iconTheme);

    static QStringList availableColorSchemes();
    static QString toColorScheme(int index);
    static int fromColorScheme(const QString &colorScheme);

    static void applyTheme(const QMap<QString, QVariant> &map);

    /* Utils */
    static void setIcons(const QWidget *widget, const QMap<QAbstractButton*, QString> &map);
    static void setIcons(const QWidget *widget, const QMap<QAction*, QString> &map);
    static void setIcons(const QWidget *widget, const QMap<QLabel *, QString> &map, int extent = 48);
};

#endif // WIDGETS_THEME_H
