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

    connect(ui->streamFormatPicker, SIGNAL(ddd()), this, SLOT(onStreamFormatPickerChanged()));
    connect(ui->fileNameEdit, SIGNAL(textChanged(QString)), this, SLOT(onTitleChanged(QString)));
    connect(ui->fileExtensionEdit, SIGNAL(textChanged(QString)), this, SLOT(onSuffixChanged(QString)));
}

StreamWidget::~StreamWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void StreamWidget::setStreamInfo(StreamInfo streamInfo)
{
    qDebug() << Q_FUNC_INFO << streamInfo;

    m_streamInfo = streamInfo;

    ui->titleLabel->setText(m_streamInfo.title());
    ui->fileNameEdit->setText(m_streamInfo.fileBaseName());
    ui->fileExtensionEdit->setText(m_streamInfo.fileExtension());

    ui->streamFormatPicker->setStreamInfo(streamInfo);

    ui->fileExtensionEdit->setText(m_streamInfo.fileExtension()); // supersede with user data
}

/******************************************************************************
 ******************************************************************************/
void StreamWidget::onStreamFormatPickerChanged()
{
    m_streamInfo.userFormatId = ui->streamFormatPicker->selectedFormatId();
    ui->fileExtensionEdit->setText(m_streamInfo.fileExtension());
    ui->estimatedSizeEdit->setText(Format::fileSizeToString(m_streamInfo.guestimateFullSize()));
}

void StreamWidget::onTitleChanged(const QString &)
{
    m_streamInfo.userTitle = ui->fileNameEdit->text();
}

void StreamWidget::onSuffixChanged(const QString &)
{
    m_streamInfo.userSuffix = ui->fileExtensionEdit->text();
}
