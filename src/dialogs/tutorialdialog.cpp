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

    setWindowTitle(QString("%0 - %1").arg(STR_APPLICATION_NAME, tr("Tutorial")));

    ui->titleLabel->setText(tr("Welcome to %0").arg(STR_APPLICATION_NAME));

    ui->subtitleLabel->setText(QString("        %0")
                               .arg(tr("This brief tutorial will help you use the"
                                       " application for the first time.")));

    ui->quickTutorialLabel->setText(
                QString("<html><head/><body>"
                        "<p><span style=\" font-size:10pt; font-weight:600;\">"
                        "%0"
                        "</span></p></body></html>")
                .arg(tr("Quick tutorial")));

    ui->label_1->setText(
                QString("<html><head/><body><p>%0<br/></p></body></html>")
                .arg(tr("Go to the Quick Sample page on the website:")));

    ui->label_2->setText(
                QString("<html><head/><body><p><br/>"
                        "%0"
                        "</p><ul style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px; -qt-list-indent: 1;\">"
                        "<li style=\" margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                        "%1"
                        "<br/></li>"
                        "<li style=\" margin-top:0px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                        "%2"
                        "<br/></li></ul></body></html>").arg(
                    tr("Read the tutorial. The page contains quick sample files:"),
                    tr("You can mass-download them"),
                    tr("Try the powerful batch-download mode too!"))
                );

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
