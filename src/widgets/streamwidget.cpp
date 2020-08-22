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
    connect(ui->fileNameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onTitleChanged(QString)));
    connect(ui->fileExtensionEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onSuffixChanged(QString)));

    ui->stackedWidget->setCurrentWidget(ui->pageInfo);
}

StreamWidget::~StreamWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void StreamWidget::setStreamInfo(StreamInfo streamInfo)
{
    m_streamInfo = streamInfo;
    ui->titleLabel->setText(m_streamInfo.title());

    if (streamInfo.isAvailable()) {
        ui->streamFormatPicker->setData(streamInfo);
        ui->fileNameEdit->setText(m_streamInfo.fileBaseName());
        ui->fileExtensionEdit->setText(m_streamInfo.suffix());

        ui->stackedWidget->setCurrentWidget(ui->pageInfo);
    } else {
        ui->stackedWidget->setCurrentWidget(ui->pageError);
    }
}

/******************************************************************************
 ******************************************************************************/
void StreamWidget::onFormatSelected(StreamFormatId formatId)
{
    m_streamInfo.setFormatId(formatId);
    ui->fileExtensionEdit->setText(m_streamInfo.suffix());
    ui->estimatedSizeEdit->setText(Format::fileSizeToString(m_streamInfo.guestimateFullSize()));
    emit streamInfoChanged(m_streamInfo);
}

void StreamWidget::onTitleChanged(const QString &)
{
    m_streamInfo.setTitle(ui->fileNameEdit->text());
    emit streamInfoChanged(m_streamInfo);
}

void StreamWidget::onSuffixChanged(const QString &)
{
    m_streamInfo.setSuffix(ui->fileExtensionEdit->text());
    emit streamInfoChanged(m_streamInfo);
}
