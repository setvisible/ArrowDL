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

#include <Core/Format>
#include <Core/IDownloadItem>
#include <Core/MimeDatabase>

#include <QtCore/QDir>

InformationDialog::InformationDialog(const QList<IDownloadItem*> &jobs, QWidget *parent)
    : QDialog(parent)
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

void InformationDialog::init(const QList<IDownloadItem *> &selection)
{
    if (selection.isEmpty()) {
        return;
    }
    const IDownloadItem *item = selection.first();

    const QUrl localFileUrl = item->localFileUrl();
    const QString filename = QDir::toNativeSeparators(localFileUrl.toLocalFile());
    ui->filenameLabel->setText(filename);

    const QString urlHtml = QString("<a href=\"%0\">%0</a>").arg(item->sourceUrl().toString());
    ui->urlLabel->setText(urlHtml);
    ui->urlLabel->setTextFormat(Qt::RichText);
    ui->urlLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->urlLabel->setOpenExternalLinks(true);

    auto bytes = item->bytesTotal();
    auto text = QString("%0 (%1 bytes)")
            .arg(Format::fileSizeToString(bytes))
            .arg(Format::fileSizeThousandSeparator(bytes));
    ui->sizeLabel->setText(text);


    const QUrl url = item->sourceUrl();
    const QPixmap pixmap = MimeDatabase::fileIcon(url, 256);
    ui->fileIcon->setPixmap(pixmap);
}
