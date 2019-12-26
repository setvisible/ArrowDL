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

#include "tutorialdialog.h"
#include "ui_tutorialdialog.h"

#include <Globals>

#include <Core/Settings>

#include <QtCore/QDebug>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QCheckBox>


TutorialDialog::TutorialDialog(Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TutorialDialog)
    , m_settings(settings)

{
    ui->setupUi(this);

    ui->link->setText(QString("<a href=\"%0\">%0</a>").arg(STR_TUTORIAL_WEBSITE));

    readSettings();
}

TutorialDialog::~TutorialDialog()
{
    delete ui;
}

void TutorialDialog::closeEvent(QCloseEvent *)
{
    writeSettings();
}

/******************************************************************************
 ******************************************************************************/
void TutorialDialog::readSettings()
{
    ui->dontShowAgainCheckBox->setChecked(m_settings->isDontShowTutorialEnabled());
}

void TutorialDialog::writeSettings()
{
    m_settings->setDontShowTutorialEnabled(ui->dontShowAgainCheckBox->isChecked());
}
