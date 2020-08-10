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
#include <QtCore/QtMath>
#include <QtGui/QMovie>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>

static const char *identifierKey = "identifier";

StreamWidget::StreamWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::StreamWidget)
{
    ui->setupUi(this);

    adjustSize();

    setState(Empty);

    connect(ui->defaultButton, SIGNAL(released()), this, SLOT(updateButtonBar()));
    connect(ui->customAudioButton, SIGNAL(released()), this, SLOT(updateButtonBar()));
    connect(ui->customVideoButton, SIGNAL(released()), this, SLOT(updateButtonBar()));

    connect(ui->defaultButton, SIGNAL(released()), this, SLOT(onChanged()));
    connect(ui->customAudioButton, SIGNAL(released()), this, SLOT(onChanged()));
    connect(ui->customVideoButton, SIGNAL(released()), this, SLOT(onChanged()));
    connect(ui->audioComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
    connect(ui->videoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));

    /* Fancy GIF animation */
    QMovie *movie = new QMovie(":/icons/menu/stream_wait_16x16.gif");
    ui->waitingIconLabel->setMovie(movie);
    movie->start();

    updateButtonBar();
}

StreamWidget::~StreamWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void StreamWidget::clear()
{
    clearDetectedFormat();
}

/******************************************************************************
 ******************************************************************************/
StreamWidget::State StreamWidget::state() const
{
    return m_state;
}

void StreamWidget::setState(State state)
{
    m_state = state;
    switch (m_state) {
    case Empty:
        ui->stackedWidget->setCurrentWidget(ui->pageEmpty);
        break;
    case Downloading:
        ui->stackedWidget->setCurrentWidget(ui->pageDownloading);
        break;
    case Normal:
        ui->stackedWidget->setCurrentWidget(ui->pageNormal);
        break;
    case Error:
        ui->stackedWidget->setCurrentWidget(ui->pageError);
        break;
    }
}

/******************************************************************************
 ******************************************************************************/
void StreamWidget::setStreamInfo(StreamInfoPtr streamInfo)
{
    Q_ASSERT(streamInfo);
    clear();

    qDebug() << Q_FUNC_INFO << streamInfo;

    m_streamInfo.swap(streamInfo);

    setState(StreamWidget::Normal);
    if (m_streamInfo) {
        ui->titleLabel->setText(m_streamInfo->safeTitle());
        ui->fileNameEdit->setText(m_streamInfo->fileBaseName());
        ui->fileExtensionEdit->setText(m_streamInfo->fileExtension(m_streamInfo->format_id));

        populateDefaultFormats(m_streamInfo->defaultFormats());
        populateComboBox(m_streamInfo->audioFormats(), ui->audioComboBox);
        populateComboBox(m_streamInfo->videoFormats(), ui->videoComboBox);

        setSelectedFormatId(m_streamInfo->formatId());
    }
}

void StreamWidget::setErrorMessage(QString errorMessage)
{
    clear();
    setState(StreamWidget::Error);
    ui->errorMessageLabel->setText(errorMessage);
}

/******************************************************************************
 ******************************************************************************/
QString StreamWidget::selectedFormatId() const
{
    if (m_state != Normal) {
        return QString();
    }
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

void StreamWidget::setSelectedFormatId(const QString &format_id)
{
    qDebug() << Q_FUNC_INFO << format_id;
    if (m_state != Normal) {
        return;
    }
    auto ids = format_id.split("+", QString::SkipEmptyParts);
    foreach (auto id, ids) {
        selectRadio(id);
        selectAudioComboBoxItem(id);
        selectVideoComboBoxItem(id);
    }
    updateButtonBar();
    onChanged();
}

/******************************************************************************
 ******************************************************************************/
QString StreamWidget::fileName() const
{
    auto fileBaseName = ui->fileNameEdit->text();
    auto fileExtension = ui->fileExtensionEdit->text();
    if (fileExtension.isEmpty()) {
        return fileBaseName;
    }
    return QString("%0.%1").arg(fileBaseName).arg(fileExtension);
}

qint64 StreamWidget::fileSize() const
{
    auto formatId = selectedFormatId();
    if (m_streamInfo) {
        return m_streamInfo->guestimateFullSize(formatId);
    }
    return -1;
}

QString StreamWidget::fileExtension() const
{
    auto formatId = selectedFormatId();
    if (m_streamInfo) {
        return m_streamInfo->fileExtension(formatId);
    }
    return QString();
}

/******************************************************************************
 ******************************************************************************/
void StreamWidget::updateButtonBar()
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

void StreamWidget::onCurrentIndexChanged(int /*index*/)
{
    onChanged();
}

void StreamWidget::onChanged()
{
    ui->fileExtensionEdit->setText(fileExtension());
    ui->estimedSizeLabel->setText(Format::fileSizeToString(fileSize()));
}

/******************************************************************************
 ******************************************************************************/
void StreamWidget::clearDetectedFormat()
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

void StreamWidget::populateDefaultFormats(const QList<StreamFormat*> &formats)
{
    for (auto format : formats) {
        auto button = appendDetectedFormat(format->toString());
        button->setProperty(identifierKey, format->format_id);
    }
    QWidget *parent = ui->detectedMediaCheckBoxList;
    QList<QRadioButton *> buttons = parent->findChildren<QRadioButton *>();
    if (!buttons.isEmpty()) {
        buttons.first()->setChecked(true);
    }
}

QRadioButton* StreamWidget::appendDetectedFormat(const QString &text)
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
void StreamWidget::populateComboBox(const QList<StreamFormat*> &formats, QComboBox *comboBox)
{
    comboBox->clear();
    for (auto format : formats) {
        comboBox->addItem(format->toString(), format->format_id);
    }
}

/******************************************************************************
 ******************************************************************************/
void StreamWidget::selectRadio(const QString &id)
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

void StreamWidget::selectAudioComboBoxItem(const QString &id)
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

void StreamWidget::selectVideoComboBoxItem(const QString &id)
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
QString StreamWidget::selectedRadio() const
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

QString StreamWidget::selectedAudioComboBoxItem() const
{
    return ui->audioComboBox->currentData(Qt::UserRole).toString();
}

QString StreamWidget::selectedVideoComboBoxItem() const
{
    return ui->videoComboBox->currentData(Qt::UserRole).toString();
}
