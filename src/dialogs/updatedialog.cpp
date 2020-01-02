/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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

#include "updatedialog.h"
#include "ui_updatedialog.h"

#include <Core/UpdateChecker>

#include <QtCore/QDebug>
#include <QtCore/QStringBuilder>
#ifndef _WIN32
#  include <QtGui/QDesktopServices>
#endif

UpdateDialog::UpdateDialog(UpdateChecker *updateChecker, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UpdateDialog)
    , m_updateChecker(updateChecker)
{
    ui->setupUi(this);

    connect(ui->checkButton, SIGNAL(released()), this, SLOT(check()));
    connect(ui->installButton, SIGNAL(released()), this, SLOT(install()));

    connect(m_updateChecker, SIGNAL(updateAvailable(CAutoUpdaterGithub::ChangeLog)),
            this, SLOT(onUpdateAvailable(CAutoUpdaterGithub::ChangeLog)));
    connect(m_updateChecker, SIGNAL(updateDownloadProgress(float)),
            this, SLOT(onUpdateDownloadProgress(float)));
    connect(m_updateChecker, SIGNAL(updateDownloadFinished()),
            this, SLOT(onUpdateDownloadFinished()));
    connect(m_updateChecker, SIGNAL(updateError(QString)),
            this, SLOT(onUpdateError(QString)));

    ui->stackedWidget->setCurrentWidget(ui->pageAlreadyUpToDate);
    ui->progressBar->setVisible(false);
    ui->progressBarValue->setVisible(false);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
    ui->installButton->setEnabled(false);

    ui->downloadedToLabel->setText(QString("Downloaded to <a href=\"%0\">%0</a>")
                                   .arg(m_updateChecker->installTempDir()));
    ui->downloadedToLabel->setVisible(false);

    check();
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void UpdateDialog::check()
{
    ui->checkButton->setEnabled(false);
    ui->stackedWidget->setCurrentWidget(ui->pageProgress);
    ui->progressBar->setVisible(false);
    ui->progressBarValue->setVisible(false);
    ui->downloadedToLabel->setVisible(false);
    ui->progressLabel->setText(tr("Checking the updates..."));

    m_updateChecker->checkForUpdates();
}

void UpdateDialog::install()
{
#ifdef _WIN32
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
    // ui->progressBar->setVisible(true);
    ui->progressBarValue->setVisible(true);
    ui->downloadedToLabel->setVisible(true);
    ui->progressLabel->setText(tr("Downloading the update..."));
    ui->stackedWidget->setCurrentWidget(ui->pageProgress);

    m_updateChecker->downloadAndInstallUpdate();

#else
    ui->stackedWidget->setCurrentWidget(ui->pageError);
    ui->errorMessage->setText(tr("Manual update required"));
    QMessageBox msg(
                QMessageBox::Question,
                tr("Manual update required"),
                tr("Automatic update is not supported on this operating system. "
                   "Do you want to download and install the update manually?"),
                QMessageBox::Yes | QMessageBox::No,
                this);

    if (msg.exec() == QMessageBox::Yes) {
        QDesktopServices::openUrl(QUrl(m_updateChecker->latestUpdateUrl()));
    }
#endif
}

/******************************************************************************
 ******************************************************************************/
void UpdateDialog::onUpdateAvailable(CAutoUpdaterGithub::ChangeLog changelog)
{
    if (!changelog.empty()) {
        ui->stackedWidget->setCurrentWidget(ui->pageNewVersionAvailable);

        for (const auto& changelogItem: changelog) {
            QString text = "<b>" % changelogItem.versionString % "</b>" % '\n' % changelogItem.versionChanges % "<p></p>";
            ui->changeLogViewer->append(text);
        }
        ui->changeLogViewer->moveCursor(QTextCursor::Start);
        ui->changeLogViewer->ensureCursorVisible();
        ui->checkButton->setEnabled(true);
        ui->installButton->setEnabled(true);

    } else {
        ui->stackedWidget->setCurrentWidget(ui->pageAlreadyUpToDate);
        ui->checkButton->setEnabled(true);
        ui->installButton->setEnabled(false);
    }
}

void UpdateDialog::onUpdateDownloadProgress(float percentageDownloaded)
{
    // ui->progressBar->setValue(static_cast<int>(percentageDownloaded));
    const QString text = QString("%0 %")
            .arg(QString::number(static_cast<double>(percentageDownloaded), 'f', 2));
    ui->progressBarValue->setText(text);
}

void UpdateDialog::onUpdateDownloadFinished()
{
    accept();
}

void UpdateDialog::onUpdateError(QString errorMessage)
{
    ui->stackedWidget->setCurrentWidget(ui->pageError);
    ui->errorMessage->setText(errorMessage.toUtf8().data());
}
