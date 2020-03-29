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
#include <Core/Format>
#include <Core/IDownloadItem>
#include <Core/MimeDatabase>
#include <Core/ResourceItem>
#include <Widgets/UrlFormWidget>

#include <QtCore/QDir>
#include <QtCore/QScopedPointer>

InformationDialog::InformationDialog(const QList<IDownloadItem*> &jobs, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InformationDialog)
{
    ui->setupUi(this);

    ui->urlFormWidget->setExternalUrlLabelAndLineEdit(nullptr, nullptr);

    initialize(jobs);
}

InformationDialog::~InformationDialog()
{
    delete ui;
}

void InformationDialog::accept()
{
    if (!m_items.isEmpty()) {
        IDownloadItem *item = m_items.first();
        DownloadItem *downloadItem = static_cast<DownloadItem*>(item);
        if (downloadItem) {
            auto resource = downloadItem->resource();
            QScopedPointer<ResourceItem> copy(ui->urlFormWidget->createResourceItem());

            resource->setUrl(copy->url());
            resource->setCustomFileName(copy->customFileName());
            resource->setReferringPage(copy->referringPage());
            resource->setDescription(copy->description());
            resource->setDestination(copy->destination());
            resource->setMask(copy->mask());
            resource->setCheckSum(copy->checkSum());

            downloadItem->stop();
            downloadItem->pause();
        }
    }
    QDialog::accept();
}

void InformationDialog::initialize(const QList<IDownloadItem *> &items)
{
    if (items.isEmpty()) {
        return;
    }
    m_items = items;
    const IDownloadItem *item = items.first();
    const DownloadItem *downloadItem = static_cast<const DownloadItem*>(item);

    /* Title and subtitles */
    const QUrl localFileUrl = item->localFileUrl();
    const QString filename = QDir::toNativeSeparators(localFileUrl.toLocalFile());
    ui->filenameLabel->setText(filename);

    const QUrl url = item->sourceUrl();
    const QString urlHtml = QString("<a href=\"%0\">%0</a>").arg(url.toString());
    ui->urlLabel->setText(urlHtml);
    ui->urlLabel->setTextFormat(Qt::RichText);
    ui->urlLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->urlLabel->setOpenExternalLinks(true);

    auto bytes = item->bytesTotal();
    if (bytes > 0) {
        auto text = QString("%0 (%1 bytes)")
                .arg(Format::fileSizeToString(bytes))
                .arg(Format::fileSizeThousandSeparator(bytes));
        ui->sizeLabel->setText(text);
    } else {
        auto text = QString("%0").arg(Format::fileSizeToString(-1));
        ui->sizeLabel->setText(text);
    }

    const QPixmap pixmap = MimeDatabase::fileIcon(url, 256);
    ui->fileIcon->setPixmap(pixmap);

    /* Form */
    if (downloadItem) {
        ui->urlFormWidget->setResource(downloadItem->resource());
    }
}
