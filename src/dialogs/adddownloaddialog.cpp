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

#include "adddownloaddialog.h"
#include "ui_adddownloaddialog.h"

#include <Core/DownloadManager>
#include <Core/Regex>
#include <Core/ResourceItem>

#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>

static const QList<ResourceItem*> createJobs(const Ui::AddDownloadDialog *ui)
{
    QList<ResourceItem*> list;
    const QString text = ui->downloadLineEdit->text();
    const QStringList downloads = Regex::interpret(text);
    foreach (auto download, downloads) {
        ResourceItem *item = new ResourceItem();
        item->setUrl(download);
        item->setCustomFileName(ui->filenameLineEdit->text());
        item->setReferringPage(ui->referrerLineEdit->text());
        item->setDescription(ui->descriptionLineEdit->text());
        item->setDestination(ui->browserWidget->text());
        item->setMask(ui->maskWidget->text());
        item->setCheckSum(ui->hashLineEdit->text());
        list << item;
    }
    return list;
}

AddDownloadDialog::AddDownloadDialog(const QUrl &url, DownloadManager *downloadManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddDownloadDialog)
    , m_downloadManager(downloadManager)

{
    ui->setupUi(this);
    ui->downloadLineEdit->setText(url.toString());
    ui->downloadLineEdit->setFocus();
}

AddDownloadDialog::~AddDownloadDialog()
{
    delete ui;
}

void AddDownloadDialog::accept()
{
    m_downloadManager->append(createJobs(ui), true);
    QDialog::accept();
}

void AddDownloadDialog::acceptPaused()
{
    m_downloadManager->append(createJobs(ui), false);
    QDialog::accept();
}

void AddDownloadDialog::reject()
{
    QDialog::reject();
}
