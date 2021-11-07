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

#include "streamconfigwidget.h"
#include "ui_streamconfigwidget.h"

#include <Core/Stream>
#include <Core/Theme>

#include <QtCore/QDebug>

/******************************************************************************
 ******************************************************************************/
StreamConfigWidget::StreamConfigWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::StreamConfigWidget)
{
    ui->setupUi(this);

    /* *********************************** */
    /* Connect main signals */
    QList<QCheckBox*> checkboxes;

    // Overview
    checkboxes << ui->skipDownloadCheckBox;
    checkboxes << ui->markWatchedCheckBox;

    // Subtitles
    checkboxes << ui->subtitleCheckBox;

    // Comments
    checkboxes << ui->commentCheckBox;

    // Thumbnails
    checkboxes << ui->thumbnailCheckBox;

    // Metadata
    checkboxes << ui->otherDescriptionCheckBox;
    checkboxes << ui->otherMetadataCheckBox;
    checkboxes << ui->otherShortcutCheckBox;

    // Post-Processing

    // SponsorBlock


    foreach (auto checkbox, checkboxes) {
        connect(checkbox, SIGNAL(toggled(bool)), this, SLOT(onCheckBoxToggled(bool)));
    }
    /* *********************************** */

    /* Connect buddies signals */
    connect(ui->commentLimitCheckBox, SIGNAL(toggled(bool)),
            ui->commentLimitSpinBox, SLOT(setEnabled(bool)));

    propagateIcons();
}

StreamConfigWidget::~StreamConfigWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void StreamConfigWidget::clear()
{
    // Don't clear() to keep the columns
}

/******************************************************************************
 ******************************************************************************/
void StreamConfigWidget::setConfig(const StreamObjectConfig &config)
{
    QSignalBlocker blocker(this);
    clear();

    // Overview
    ui->skipDownloadCheckBox->setChecked(config.overview.skipVideo);
    ui->markWatchedCheckBox->setChecked(config.overview.markWatched);

    // Subtitles
    ui->subtitleCheckBox->setChecked(config.subtitle.writeDefaultSubtitle);

    // Comments
    ui->commentCheckBox->setChecked(config.comment.writeComment);

    // Thumbnails
    ui->thumbnailCheckBox->setChecked(config.thumbnail.writeDefaultThumbnail);

    // Metadata
    ui->otherDescriptionCheckBox->setChecked(config.metadata.writeDescription);
    ui->otherMetadataCheckBox->setChecked(config.metadata.writeMetadata);
    ui->otherShortcutCheckBox->setChecked(config.metadata.writeInternetShortcut);

    // Post-Processing

    // SponsorBlock
}

/******************************************************************************
 ******************************************************************************/
void StreamConfigWidget::onCheckBoxToggled(bool checked)
{
    Q_UNUSED(checked)
    onChanged();
}

/******************************************************************************
 ******************************************************************************/
void StreamConfigWidget::onChanged()
{
    StreamObjectConfig config;

    // Overview
    config.overview.skipVideo = ui->skipDownloadCheckBox->isChecked();
    config.overview.markWatched = ui->markWatchedCheckBox->isChecked();

    // Subtitles
    config.subtitle.writeDefaultSubtitle = ui->subtitleCheckBox->isChecked();

    // Comments
    config.comment.writeComment = ui->commentCheckBox->isChecked();

    // Thumbnails
    config.thumbnail.writeDefaultThumbnail = ui->thumbnailCheckBox->isChecked();

    // Metadata
    config.metadata.writeDescription = ui->otherDescriptionCheckBox->isChecked();
    config.metadata.writeMetadata = ui->otherMetadataCheckBox->isChecked();
    config.metadata.writeInternetShortcut = ui->otherShortcutCheckBox->isChecked();

    // Post-Processing

    // SponsorBlock

    emit configChanged(config);
}

/******************************************************************************
 ******************************************************************************/
void StreamConfigWidget::propagateIcons()
{
}
