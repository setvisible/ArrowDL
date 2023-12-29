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

#include "themewidget.h"
#include "ui_themewidget.h"

#include <Core/Theme>

#include <QtCore/QMap>
#include <QtCore/QDebug>
#include <QtCore/QSignalBlocker>

ThemeWidget::ThemeWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::ThemeWidget)
{
    ui->setupUi(this);

    retranslateComboBox();

    connect(ui->platformStyleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboboxChanged(int)));
    connect(ui->iconThemeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboboxChanged(int)));
    connect(ui->colorSchemeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboboxChanged(int)));
}

ThemeWidget::~ThemeWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void ThemeWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateComboBox();
    }
    QWidget::changeEvent(event);
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Returns the non-default settings only.
 */
QMap<QString, QVariant> ThemeWidget::theme() const
{
    auto platformStyle = Theme::toPlatformStyle(ui->platformStyleComboBox->currentIndex());
    auto iconTheme = Theme::toIconTheme(ui->iconThemeComboBox->currentIndex());
    auto colorScheme = Theme::toColorScheme(ui->colorSchemeComboBox->currentIndex());

    QMap<QString, QVariant> map;
    map.insert(Theme::PlatformStyle, platformStyle);
    map.insert(Theme::IconTheme, iconTheme);
    map.insert(Theme::ColorScheme, colorScheme);
    return map;
}

void ThemeWidget::setTheme(const QMap<QString, QVariant> &map)
{
    QSignalBlocker blocker1(ui->platformStyleComboBox);
    QSignalBlocker blocker2(ui->iconThemeComboBox);
    QSignalBlocker blocker3(ui->colorSchemeComboBox);

    auto platformStyle = map.value(Theme::PlatformStyle, QString()).toString();
    auto iconTheme = map.value(Theme::IconTheme, QString()).toString();
    auto colorScheme = map.value(Theme::ColorScheme, QString()).toString();

    ui->platformStyleComboBox->setCurrentIndex(Theme::fromPlatformStyle(platformStyle));
    ui->iconThemeComboBox->setCurrentIndex(Theme::fromIconTheme(iconTheme));
    ui->colorSchemeComboBox->setCurrentIndex(Theme::fromColorScheme(colorScheme));

    blocker1.unblock();
    blocker2.unblock();
    blocker3.unblock();

    emit changed();
}

void ThemeWidget::comboboxChanged(int /*value*/)
{
    emit changed();
}

/******************************************************************************
 ******************************************************************************/
void ThemeWidget::retranslateComboBox()
{
    const QSignalBlocker blocker1(ui->platformStyleComboBox);
    const QSignalBlocker blocker2(ui->iconThemeComboBox);
    const QSignalBlocker blocker3(ui->colorSchemeComboBox);

    auto index1 = ui->platformStyleComboBox->currentIndex();
    auto index2 = ui->iconThemeComboBox->currentIndex();
    auto index3 = ui->colorSchemeComboBox->currentIndex();

    ui->platformStyleComboBox->clear();
    ui->iconThemeComboBox->clear();
    ui->colorSchemeComboBox->clear();

    ui->platformStyleComboBox->addItems(Theme::availablePlatformStyles());
    ui->iconThemeComboBox->addItems(Theme::availableIconThemes());
    ui->colorSchemeComboBox->addItems(Theme::availableColorSchemes());

    ui->platformStyleComboBox->setCurrentIndex(index1);
    ui->iconThemeComboBox->setCurrentIndex(index2);
    ui->colorSchemeComboBox->setCurrentIndex(index3);
}
