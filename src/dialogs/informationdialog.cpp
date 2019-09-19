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

#include "informationdialog.h"
#include "ui_informationdialog.h"

#include <Core/DownloadItem>
#include <Core/ResourceItem>
#include <Core/MimeDatabase>

#include <QtCore/QDir>

InformationDialog::InformationDialog(const QList<DownloadItem*> &jobs, QWidget *parent) : QDialog(parent)
  , ui(new Ui::InformationDialog)
{
    ui->setupUi(this);
    init(jobs);
}

InformationDialog::~InformationDialog()
{
    delete ui;
}

void InformationDialog::accept()
{
    QDialog::accept();
}

void InformationDialog::init(const QList<DownloadItem*> &selection)
{
    if (selection.isEmpty()) {
        return;
    }
    DownloadItem *item = selection.first();

    QUrl url = item->localFileUrl();
    QString filename = QDir::toNativeSeparators(url.toLocalFile());

    ui->filenameLabel->setText(filename);
    ui->urlLabel->setText(item->sourceUrl().toString());

    QString bytes = item->bytesTotal() > 0
            ? tr("%0 bytes").arg(item->bytesTotal())
            : tr("Unknown");
    ui->sizeLabel->setText(bytes);

    if (item->resource()) {
        const QString url = item->resource()->url();
        const QPixmap pixmap = MimeDatabase::fileIcon(url, 256);
        ui->fileIcon->setPixmap(pixmap);
    }
}
