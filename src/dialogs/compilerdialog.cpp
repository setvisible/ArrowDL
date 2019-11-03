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

#include "compilerdialog.h"
#include "ui_compilerdialog.h"

#include <Globals>

CompilerDialog::CompilerDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CompilerDialog)
{
    ui->setupUi(this);

    ui->title->setText(QString("%0 %1").arg(STR_APPLICATION_NAME, STR_COMPILER_WORDSIZE));
    ui->version->setText(STR_APPLICATION_VERSION);

    ui->link->setText(QString("<a href=\"%0\">%0</a>").arg(STR_APPLICATION_WEBSITE));
    ui->link->setTextFormat(Qt::RichText);
    ui->link->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->link->setOpenExternalLinks(true);

    ui->compilerName->setText(STR_COMPILER_NAME);
    ui->compilerVersion->setText(STR_COMPILER_BUILD_ABI);
    ui->compilerCpuArchitecture->setText(STR_COMPILER_BUILD_CPU);
    ui->compilerBuildDate->setText(STR_APPLICATION_BUILD);

    ui->currentOS->setText(STR_CURRENT_PLATFORM);
    ui->currentVersion->setText(STR_CURRENT_VERSION);
    ui->currentCpuArchitecture->setText(STR_CURRENT_CPU);

    ui->QtVersion->setText(QT_VERSION_STR);
    ui->googleGumboVersion->setText(GOOGLE_GUMBO_VERSION_STR);
}

CompilerDialog::~CompilerDialog()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void CompilerDialog::on_okButton_released()
{
    QDialog::reject();
}
