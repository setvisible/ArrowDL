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

#include "informationdialog.h"
#include "ui_informationdialog.h"

#include <Globals>
#include <Core/DownloadItem>
#include <Core/Format>
#include <Core/IDownloadItem>
#include <Core/MimeDatabase>
#include <Core/ResourceItem>
#include <Core/Theme>
#include <Widgets/UrlFormWidget>

#include <QtCore/QDir>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>

InformationDialog::InformationDialog(const QList<IDownloadItem *> &jobs, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InformationDialog)
{
    ui->setupUi(this);

    setWindowTitle(QString("%0 - %1").arg(STR_APPLICATION_NAME, tr("Properties")));
    ui->urlFormWidget->setExternalUrlLabelAndLineEdit(nullptr, nullptr);
    ui->urlFormWidget->setCollapsible(false);
    Theme::setIcons(this, { {ui->logo, "info"} });

    ui->wrapCheckBox->setChecked(false);
    ui->logTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

    connect(ui->wrapCheckBox, SIGNAL(toggled(bool)), this, SLOT(wrapLog(bool)));

    initialize(jobs);
    readUiSettings();
}

InformationDialog::~InformationDialog()
{
    writeUiSettings();
    delete ui;
}

void InformationDialog::readUiSettings()
{
    QSettings settings;
    settings.beginGroup("InfoDialog");
    resize(settings.value("DialogSize", size()).toSize());
    ui->tabWidget->setCurrentIndex(settings.value("TabIndex", 0).toInt());
    settings.endGroup();
}

void InformationDialog::writeUiSettings()
{
    QSettings settings;
    settings.beginGroup("InfoDialog");
    settings.setValue("DialogSize", size());
    settings.setValue("TabIndex", ui->tabWidget->currentIndex());
    settings.endGroup();
}

void InformationDialog::accept()
{
    if (!m_items.isEmpty()) {
        IDownloadItem *item = m_items.first();
        auto downloadItem = dynamic_cast<DownloadItem*>(item);
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
    auto downloadItem = dynamic_cast<const DownloadItem*>(item);

    /* Title and subtitles */
    const QUrl localFileUrl = item->localFileUrl();
    const QString filename = QDir::toNativeSeparators(localFileUrl.toLocalFile());
    ui->fileNameLineEdit->setText(Format::wrapText(filename, 50));

    const QUrl url = item->sourceUrl();
    const QString urlHtml = Format::toHtmlMark(url, true);
    ui->urlLineEdit->setText(urlHtml);
    ui->urlLineEdit->setTextFormat(Qt::RichText);
    ui->urlLineEdit->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    ui->urlLineEdit->setOpenExternalLinks(true);

    qsizetype bytes = item->bytesTotal();
    if (bytes > 0) {
        auto text = QString("%0 (%1 bytes)").arg(
                    Format::fileSizeToString(bytes),
                    Format::fileSizeThousandSeparator(bytes));
        ui->sizeLineEdit->setText(text);
    } else {
        auto text = QString("%0").arg(Format::fileSizeToString(-1));
        ui->sizeLineEdit->setText(text);
    }

    const QPixmap pixmap = MimeDatabase::fileIcon(localFileUrl.toLocalFile(), 48);
    ui->logo->setPixmap(pixmap);

    /* Form */
    if (downloadItem) {
        ui->urlFormWidget->setResource(downloadItem->resource());
    }

    /* Log */
    if (downloadItem) {
        ui->logTextEdit->setPlainText(downloadItem->log());

        // Scroll to last line
        QTextCursor cursor = ui->logTextEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->logTextEdit->setTextCursor(cursor);
        ui->logTextEdit->ensureCursorVisible();
    }
}

void InformationDialog::wrapLog(bool enabled)
{
    ui->logTextEdit->setLineWrapMode(enabled ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}
