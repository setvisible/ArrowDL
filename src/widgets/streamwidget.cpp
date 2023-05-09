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

#include "streamwidget.h"
#include "ui_streamwidget.h"

#include <Core/Format>

#include <QtCore/QDebug>


StreamWidget::StreamWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::StreamWidget)
{
    ui->setupUi(this);

    adjustSize();

    connect(ui->streamFormatPicker, SIGNAL(selectionChanged(StreamFormatId)),
            this, SLOT(onFormatSelected(StreamFormatId)));
    connect(ui->streamFormatPicker, SIGNAL(configChanged()),
            this, SLOT(onConfigChanged()));
    connect(ui->fileNameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onTitleChanged(QString)));
    connect(ui->fileExtensionEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onSuffixChanged(QString)));

    ui->stackedWidget->setCurrentWidget(ui->pageInfo);

    QFontMetrics fm(ui->fileExtensionEdit->font());
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    int pixelsWide = fm.horizontalAdvance(".webm3");
#else
    int pixelsWide = fm.width(".webm3");
#endif
    ui->fileExtensionEdit->setMaximumWidth(pixelsWide);
    ui->fileExtensionEdit->setMinimumWidth(pixelsWide);
}

StreamWidget::~StreamWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void StreamWidget::setStreamObject(const StreamObject &streamObject)
{
    m_streamObject = streamObject;
    ui->titleLabel->setText(m_streamObject.title());

    if (streamObject.isAvailable()) {
        ui->streamFormatPicker->setData(streamObject);
        ui->fileNameEdit->setText(m_streamObject.fileBaseName());
        ui->fileExtensionEdit->setText(m_streamObject.suffix());
        updateEstimatedSize();

        ui->stackedWidget->setCurrentWidget(ui->pageInfo);
    } else {
        ui->stackedWidget->setCurrentWidget(ui->pageError);
    }
}

/******************************************************************************
 ******************************************************************************/
void StreamWidget::onFormatSelected(StreamFormatId formatId)
{
    m_streamObject.setFormatId(formatId);
    emit streamObjectChanged(m_streamObject);
    updateEstimatedSize();
}

void StreamWidget::onConfigChanged()
{
    auto config = ui->streamFormatPicker->config();
    m_streamObject.setConfig(config);
    emit streamObjectChanged(m_streamObject);
    updateEstimatedSize();
}

void StreamWidget::onTitleChanged(QString title)
{
    Q_UNUSED(title)
    m_streamObject.setTitle(ui->fileNameEdit->text());
    emit streamObjectChanged(m_streamObject);
}

void StreamWidget::onSuffixChanged(QString suffix)
{
    Q_UNUSED(suffix)
    m_streamObject.setSuffix(ui->fileExtensionEdit->text());
    emit streamObjectChanged(m_streamObject);
}

/******************************************************************************
 ******************************************************************************/
void StreamWidget::updateEstimatedSize()
{
    ui->fileExtensionEdit->setText(m_streamObject.suffix());
    QString text;
    auto config = m_streamObject.config();
    if (config.overview.skipVideo) {
        text = Format::fileSizeToString(0);
        text += tr(" (no video)");
    } else {
        text = Format::fileSizeToString(m_streamObject.guestimateFullSize());
    }
    if (config.subtitle.writeSubtitle) {
        text += tr(" + subtitles");
    }
    if (config.chapter.writeChapters) {
        text += tr(" + chapters");
    }
    if (config.thumbnail.writeDefaultThumbnail) {
        text += tr(" + thumbnails");
    }
    if (config.metadata.writeDescription) {
        text += tr(" + .description");
    }
    if (config.metadata.writeMetadata) {
        text += tr(" + .info.json");
    }
    if (config.metadata.writeInternetShortcut) {
        text += tr(" + shortcut");
    }
    ui->estimatedSizeEdit->setText(text);
}
