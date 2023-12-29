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

#include "theme.h"

#include <Constants>
#include <Widgets/CustomStyle>

#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtGui/QAction>
#include <QtGui/QColor>
#include <QtGui/QPalette>
#include <QtWidgets/QApplication>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleFactory>


/* Public static constants */
const QLatin1StringView Theme::PlatformStyle("platformStyle");
const QLatin1StringView Theme::IconTheme("iconTheme");
const QLatin1StringView Theme::ColorScheme("colorScheme");

/* Private static constants */
static QString s_themeSearchPath = QString::fromUtf8(":/resources/icons/");

/* Runtime Debug Purpose */
static void _assertNoMissingIconTheme(const QWidget *widget)
{
    auto widgetName = widget->windowTitle().toLatin1().data();
    QList<QAbstractButton*> buttons = widget->findChildren<QAbstractButton*>();
    foreach (QAbstractButton* button, buttons) {
        if ( button->objectName() == QLatin1String("qt_menubar_ext_button") ||
             button->objectName() == QLatin1String("qt_toolbar_ext_button")) {
            continue;
        }
        if (!button->icon().isNull() && button->icon().name().isEmpty()) {
            qWarning("Missing icon theme name for QAbstractButton '%s' ('%s') in QWidget '%s'.",
                     button->objectName().toLatin1().data(),
                     button->text().toLatin1().data(),
                     widgetName);
        }
    }
    QList<QAction*> actions = widget->findChildren<QAction*>();
    foreach (QAction* action, actions) {
        if (!action->icon().isNull() && action->icon().name().isEmpty()) {
            qWarning("Missing icon theme name for QAction '%s' ('%s') in QWidget '%s'.",
                     action->objectName().toLatin1().data(),
                     action->text().toLatin1().data(),
                     widgetName);
        }
    }
}

Theme::Theme()
{
}

/******************************************************************************
 ******************************************************************************/
QStringList Theme::availablePlatformStyles()
{
    return QStyleFactory::keys();
}

QString Theme::toPlatformStyle(qsizetype index)
{
    auto keys = QStyleFactory::keys();
    if (index >= 0 && index < keys.count()) {
        return keys.at(index);
    }
    return QLatin1String(""); // Must be an empty string, not a null QString
}

qsizetype Theme::fromPlatformStyle(const QString &platformStyle)
{
    auto index = QStyleFactory::keys().indexOf(platformStyle);
    return index == -1 ? 0 : index;
}

/******************************************************************************
 ******************************************************************************/
QStringList Theme::availableIconThemes()
{
    return {QObject::tr("Classic (default)"), QObject::tr("Flat Design")};
}

QString Theme::toIconTheme(int index)
{
    switch (index) {
    case 0: return THEME_DEFAULT;
    case 1: return THEME_FLAT;
    default: break;
    }
    return QLatin1String(""); // Must be an empty string, not a null QString
}

int Theme::fromIconTheme(const QString &iconTheme)
{
    if (iconTheme.compare(THEME_DEFAULT, Qt::CaseInsensitive) == 0) {
        return 0;
    } else if (iconTheme.compare(THEME_FLAT, Qt::CaseInsensitive) == 0) {
        return 1;
    }
    return 0;
}

/******************************************************************************
 ******************************************************************************/
QStringList Theme::availableColorSchemes()
{
    return {QObject::tr("Light"), QObject::tr("Dark")};
}

QString Theme::toColorScheme(int index)
{
    switch (index) {
    case 0: return SCHEME_LIGHT;
    case 1: return SCHEME_DARK;
    default: break;
    }
    return QLatin1String(""); // Must be an empty string, not a null QString
}

int Theme::fromColorScheme(const QString &colorScheme)
{
    if (colorScheme.compare(SCHEME_LIGHT, Qt::CaseInsensitive) == 0) {
        return 0;
    } else if (colorScheme.compare(SCHEME_DARK, Qt::CaseInsensitive) == 0) {
        return 1;
    }
    return 0;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Modify palette to dark.
 *
 * Inspiration:
 * https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle/blob/master/DarkStyle.cpp
 */
static void polishDark(QPalette &palette)
{
    // --
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    palette.setColor(QPalette::Button, QColor(53, 53, 53));
    palette.setColor(QPalette::Light, QColor(90, 90, 90)); // custom
    palette.setColor(QPalette::Midlight, QColor(75,75,75)); // custom
    palette.setColor(QPalette::Dark, QColor(35, 35, 35));
    palette.setColor(QPalette::Mid, QColor(45, 45, 45)); // custom
    // --
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    palette.setColor(QPalette::Base, QColor(42, 42, 42));
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::Shadow, QColor(20, 20, 20));
    // --
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));
    // --
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::LinkVisited, QColor(130, 42, 218)); // custom
    // --
    palette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    // --
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, QColor(53, 53, 53));
    // --
    palette.setColor(QPalette::PlaceholderText, QColor(127, 127, 127)); // custom
    // --
}

/******************************************************************************
 ******************************************************************************/
void Theme::applyTheme(const QMap<QString, QVariant> &map)
{
    if (!QIcon::themeSearchPaths().contains(s_themeSearchPath)) {
        QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << s_themeSearchPath);
    }
    if (QIcon::fallbackThemeName().isEmpty()) {
        QIcon::setFallbackThemeName(QLatin1String("default"));
    }

    const QString platformStyle = map.value(Theme::PlatformStyle, QString()).toString();
    const QString iconTheme = map.value(Theme::IconTheme, QString()).toString();
    const QString colorScheme = map.value(Theme::ColorScheme, QString()).toString();

    auto index = Theme::fromIconTheme(iconTheme);
    auto isDarkMode = (Theme::fromColorScheme(colorScheme) == 1);

    if (index == 1)  {
        QIcon::setThemeName(isDarkMode
                            ? QLatin1String("flat-dark")
                            : QLatin1String("flat"));
    } else {
        QIcon::setThemeName(QLatin1String("default"));
    }
    auto dark_palette = qApp->style()->standardPalette();
    if (isDarkMode) {
        polishDark(dark_palette);
    }
    qApp->setPalette(dark_palette); // emits QEvent::PaletteChange
    qApp->setStyle(new CustomStyle(platformStyle)); // emits QEvent::StyleChange
}

/******************************************************************************
 ******************************************************************************/
/*!
 * QtDesigner doesn't icon themes correctly, this is a workaround
 */
void Theme::setIcons(const QWidget *widget, const QMap<QAbstractButton*, QString> &map)
{
    QMapIterator<QAbstractButton*, QString> it(map);
    while (it.hasNext()) {
        it.next();
        auto button = it.key();
        auto name = it.value();
        button->setIcon(QIcon::fromTheme(name));
    }
    _assertNoMissingIconTheme(widget);
}

void Theme::setIcons(const QWidget *widget, const QMap<QAction*, QString> &map)
{
    QMapIterator<QAction*, QString> it(map);
    while (it.hasNext()) {
        it.next();
        auto action = it.key();
        auto name = it.value();
        action->setIcon(QIcon::fromTheme(name));
    }
    _assertNoMissingIconTheme(widget);
}

void Theme::setIcons(const QWidget *widget, const QMap<QLabel*, QString> &map, int extent)
{
    QMapIterator<QLabel*, QString> it(map);
    while (it.hasNext()) {
        it.next();
        auto label = it.key();
        auto name = it.value();
        label->setPixmap(QIcon::fromTheme(name).pixmap(extent));
    }
    _assertNoMissingIconTheme(widget);
}
