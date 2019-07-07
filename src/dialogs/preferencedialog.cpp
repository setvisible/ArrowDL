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

#include "preferencedialog.h"
#include "ui_preferencedialog.h"

#include <Core/Settings>
#include <Widgets/BrowserWidget>

#include <QtCore/QSettings>
#include <QtGui/QCloseEvent>
#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

PreferenceDialog::PreferenceDialog(Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PreferenceDialog)
    , m_settings(settings)

{
    Q_ASSERT(m_settings);
    ui->setupUi(this);

    initializeGui();
    read();
    readSettings();
}

PreferenceDialog::~PreferenceDialog()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void PreferenceDialog::accept()
{
    write();
    writeSettings();
    QDialog::accept();
}

void PreferenceDialog::reject()
{
    writeSettings();
    QDialog::reject();
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::initializeGui()
{
    // otherwise it's not white?
    this->setAutoFillBackground(true);
    ui->tabWidget->setAutoFillBackground(true);
    ui->tabWidget->tabBar()->setAutoFillBackground(true);

    ui->browseDatabaseFile->setType(BrowserWidget::File);
    ui->browseDatabaseFile->setExtensionName("Queue Database");
    ui->browseDatabaseFile->setExtensionType(".json");
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::restoreDefaultSettings()
{
    m_settings->beginRestoreDefault();
    read();
    m_settings->endRestoreDefault();
}

void PreferenceDialog::read()
{
    ui->browseDatabaseFile->setText(m_settings->database());
}

void PreferenceDialog::write()
{
    m_settings->setDatabase(ui->browseDatabaseFile->text());
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("Preference");
    resize(settings.value("DialogSize", QSize(350, 350)).toSize());
    const int index = settings.value("TabIndex", 0).toInt();
    if (index >=0 && index < ui->tabWidget->count()) {
        ui->tabWidget->setCurrentIndex(index);
    }
    settings.endGroup();
}

void PreferenceDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Preference");
    settings.setValue("DialogSize", size());
    settings.setValue("TabIndex", ui->tabWidget->currentIndex());
    settings.endGroup();
}
