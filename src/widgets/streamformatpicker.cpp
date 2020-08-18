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

#include "streamformatpicker.h"
#include "ui_streamformatpicker.h"

#include <Core/Format>

#include <QtCore/QDebug>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QRadioButton>

static const char *identifierKey = "identifier";

StreamFormatPicker::StreamFormatPicker(QWidget *parent) : QWidget(parent)
  , ui(new Ui::StreamFormatPicker)
{
    ui->setupUi(this);

    adjustSize();

    connect(ui->defaultButton, SIGNAL(released()), this, SLOT(updateButtonBar()));
    connect(ui->customAudioButton, SIGNAL(released()), this, SLOT(updateButtonBar()));
    connect(ui->customVideoButton, SIGNAL(released()), this, SLOT(updateButtonBar()));

    connect(ui->defaultButton, SIGNAL(released()), this, SLOT(onChanged()));
    connect(ui->customAudioButton, SIGNAL(released()), this, SLOT(onChanged()));
    connect(ui->customVideoButton, SIGNAL(released()), this, SLOT(onChanged()));
    connect(ui->audioComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
    connect(ui->videoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));

    updateButtonBar();
}

StreamFormatPicker::~StreamFormatPicker()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void StreamFormatPicker::setStreamInfo(StreamInfo streamInfo)
{
    clearDetectedFormat();

    qDebug() << Q_FUNC_INFO << streamInfo;

    populateDefaultFormats(streamInfo.defaultFormats());
    populateComboBox(streamInfo.audioFormats(), ui->audioComboBox);
    populateComboBox(streamInfo.videoFormats(), ui->videoComboBox);

    setSelectedFormatId(streamInfo.formatId());
}

/******************************************************************************
 ******************************************************************************/
QString StreamFormatPicker::selectedFormatId() const
{
    if (ui->defaultButton->isChecked()) {
        return selectedRadio();
    }
    auto id = selectedAudioComboBoxItem();
    if (ui->videoGroup->isVisible()) {
        // The first format must contain the video.
        return selectedVideoComboBoxItem() + "+" + id;
    }
    return id;
}

void StreamFormatPicker::setSelectedFormatId(const QString &format_id)
{
    qDebug() << Q_FUNC_INFO << format_id;
    if (!format_id.isEmpty()) {
        auto ids = format_id.split("+", QString::SkipEmptyParts);
        foreach (auto id, ids) {
            selectRadio(id);
            selectAudioComboBoxItem(id);
            selectVideoComboBoxItem(id);
        }
        updateButtonBar();
        onChanged();
    }
}

/******************************************************************************
 ******************************************************************************/
void StreamFormatPicker::updateButtonBar()
{
    if (ui->defaultButton->isChecked()) {
        ui->choiceStackedWidget->setCurrentWidget(ui->pageDefault);
    } else {
        ui->choiceStackedWidget->setCurrentWidget(ui->pageCustom);
    }

    bool empty = (ui->audioComboBox->count() + ui->videoComboBox->count()) == 0;
    ui->warningGroup->setVisible(empty);
    if (empty) {
        ui->audioGroup->setVisible(false);
        ui->videoGroup->setVisible(false);
    } else{
        ui->audioGroup->setVisible(true);
        ui->videoGroup->setVisible(!ui->customAudioButton->isChecked());
    }
}

void StreamFormatPicker::onCurrentIndexChanged(int /*index*/)
{
    onChanged();
}

void StreamFormatPicker::onChanged()
{
    emit ddd();
}

/******************************************************************************
 ******************************************************************************/
void StreamFormatPicker::clearDetectedFormat()
{
    QWidget *parent = ui->detectedMediaCheckBoxList;
    if (!parent->layout()) {
        parent->setLayout(new QVBoxLayout());
    }
    QLayoutItem *wItem;
    while ((wItem = parent->layout()->takeAt(0)) != nullptr) {
        if (wItem->widget()) {
            wItem->widget()->setParent(nullptr);
        }
        delete wItem;
    }
}

void StreamFormatPicker::populateDefaultFormats(const QList<StreamFormat> &formats)
{
    for (auto format : formats) {
        auto button = appendDetectedFormat(format.toString());
        button->setProperty(identifierKey, format.format_id);
    }
    QWidget *parent = ui->detectedMediaCheckBoxList;
    QList<QRadioButton *> buttons = parent->findChildren<QRadioButton *>();
    if (!buttons.isEmpty()) {
        buttons.first()->setChecked(true);
    }
}

QRadioButton* StreamFormatPicker::appendDetectedFormat(const QString &text)
{
    QWidget *parent = ui->detectedMediaCheckBoxList;
    QRadioButton *button = new QRadioButton(parent);
    button->setText(text);
    button->setAutoExclusive(true);
    button->setCursor(Qt::PointingHandCursor);
    connect(button, SIGNAL(released()), this, SLOT(onChanged()));
    parent->layout()->addWidget(button);
    return button;
}

/******************************************************************************
 ******************************************************************************/
void StreamFormatPicker::populateComboBox(const QList<StreamFormat> &formats, QComboBox *comboBox)
{
    comboBox->clear();
    for (auto format : formats) {
        comboBox->addItem(format.toString(), format.format_id);
    }
}

/******************************************************************************
 ******************************************************************************/
void StreamFormatPicker::selectRadio(const QString &id)
{
    QWidget *parent = ui->detectedMediaCheckBoxList;
    QList<QRadioButton *> buttons = parent->findChildren<QRadioButton *>();
    for (auto button: buttons) {
        QString identifier = button->property(identifierKey).toString();
        if (identifier == id) {
            button->setChecked(true);
            ui->defaultButton->setChecked(true);
            return;
        }
    }
}

void StreamFormatPicker::selectAudioComboBoxItem(const QString &id)
{
    for (int i = 0; i < ui->audioComboBox->count(); ++i) {
        if (ui->audioComboBox->itemData(i, Qt::UserRole).toString() == id) {
            ui->audioComboBox->setCurrentIndex(i);
            if (!ui->customVideoButton->isChecked()) {
                ui->customAudioButton->setChecked(true);
            }
            return;
        }
    }
}

void StreamFormatPicker::selectVideoComboBoxItem(const QString &id)
{
    for (int i = 0; i < ui->videoComboBox->count(); ++i) {
        if (ui->videoComboBox->itemData(i, Qt::UserRole).toString() == id) {
            ui->videoComboBox->setCurrentIndex(i);
            ui->customVideoButton->setChecked(true);
            return;
        }
    }
}

/******************************************************************************
 ******************************************************************************/
QString StreamFormatPicker::selectedRadio() const
{
    QWidget *parent = ui->detectedMediaCheckBoxList;
    QList<QRadioButton *> buttons = parent->findChildren<QRadioButton *>();
    for (auto button: buttons) {
        if (button->isChecked()) {
            return button->property(identifierKey).toString();
        }
    }
    return QString();
}

QString StreamFormatPicker::selectedAudioComboBoxItem() const
{
    return ui->audioComboBox->currentData(Qt::UserRole).toString();
}

QString StreamFormatPicker::selectedVideoComboBoxItem() const
{
    return ui->videoComboBox->currentData(Qt::UserRole).toString();
}
