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
#include <Core/Stream>
#include <Core/Theme>

#include <QtCore/QDebug>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QComboBox>


/******************************************************************************
 ******************************************************************************/
RadioItemDelegate::RadioItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void RadioItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    initStyleOption(&myOption, index);
    auto text = index.data(Qt::DisplayRole).toString();
    auto checked = index.data(StreamFormatPicker::CheckStateRole).toBool();
    if (index.column() == 0) {
        QStyleOptionButton radio;
        radio.rect = myOption.rect;
        radio.palette = myOption.palette;
        radio.text = text;
        radio.features |= QStyleOptionButton::Flat;
        radio.state |= QStyle::State_Enabled;
        radio.state |= checked ? QStyle::State_On : QStyle::State_Off;
        QApplication::style()->drawControl(QStyle::CE_RadioButton, &radio, painter);
    } else {
        QStyledItemDelegate::paint(painter, myOption, index);
    }
}

bool RadioItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease && index.column() == 0) {
        for (int row = 0; row < index.model()->rowCount(); ++row) {
            auto checked = row == index.row();
            model->setData(model->index(row, 0), checked, StreamFormatPicker::CheckStateRole);
        }
        return true;
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

/******************************************************************************
 ******************************************************************************/
StreamFormatPicker::StreamFormatPicker(QWidget *parent) : QWidget(parent)
  , ui(new Ui::StreamFormatPicker)
  , m_model(new QStandardItemModel(0, 1, this))
{
    ui->setupUi(this);

    ui->listView->setItemDelegate(new RadioItemDelegate(ui->listView));
    ui->listView->setModel(m_model);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->listView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listView->setSpacing(3);
    ui->listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->listView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->listView->viewport()->setAutoFillBackground(false); // transparent background
    ui->listView->setFrameShape(QAbstractItemView::NoFrame);
    ui->listView->setCursor(Qt::PointingHandCursor);
    ui->listView->setUniformItemSizes(true);
    ui->listView->setSizeAdjustPolicy(QAbstractItemView::AdjustToContents);

    adjustSize();

    connect(ui->categorySimpleButton, SIGNAL(released()), this, SLOT(onCategoryChanged()));
    connect(ui->categoryAudioButton, SIGNAL(released()), this, SLOT(onCategoryChanged()));
    connect(ui->categoryVideoButton, SIGNAL(released()), this, SLOT(onCategoryChanged()));

    connect(ui->audioComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
    connect(ui->videoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
    connect(ui->listView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(onCurrentChanged(QModelIndex, QModelIndex)));

    updateButtonBar();
    propagateIcons();
}

StreamFormatPicker::~StreamFormatPicker()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void StreamFormatPicker::clear()
{
    // Don't clear() to keep the columns
    m_model->removeRows(0, m_model->rowCount());
}

void StreamFormatPicker::setData(const StreamObject &streamObject)
{
    QSignalBlocker blocker(this);
    clear();

    populateSimple(streamObject.defaultFormats());
    populateComboBox(streamObject.audioFormats(), ui->audioComboBox);
    populateComboBox(streamObject.videoFormats(), ui->videoComboBox);

    select(streamObject.formatId());
}

/******************************************************************************
 ******************************************************************************/
void StreamFormatPicker::select(const StreamFormatId &formatId)
{
    if (!formatId.isEmpty()) {
        const QSignalBlocker blocker(this);
        foreach (auto compoundId, formatId.compoundIds()) {
            setCurrentSimple(compoundId);
            setCurrentAudio(compoundId);
            setCurrentVideo(compoundId);
        }
    }
    onCategoryChanged();
}

StreamFormatId StreamFormatPicker::selection() const
{
    if (ui->categorySimpleButton->isChecked()) {
        return currentSimple();
    }
    if (ui->videoGroup->isVisible()) {
        /* The first format must be the video. */
        return currentVideo() + currentAudio();
    }
    return currentAudio();
}

/******************************************************************************
 ******************************************************************************/
void StreamFormatPicker::onCurrentChanged(const QModelIndex &current,
                                          const QModelIndex &/*previous*/)
{
    auto formatId = current.data(StreamFormatPicker::FormatIdRole).value<StreamFormatId>();
    if (!formatId.isEmpty()) {
        emit selectionChanged(formatId);
    }
}

void StreamFormatPicker::onCurrentIndexChanged(int /*index*/)
{
    auto formatId = selection();
    if (!formatId.isEmpty()) {
        emit selectionChanged(formatId);
    }
}

void StreamFormatPicker::onCategoryChanged()
{
    updateButtonBar();
    auto formatId = selection();
    if (!formatId.isEmpty()) {
        emit selectionChanged(formatId);
    }
}

/******************************************************************************
 ******************************************************************************/
void StreamFormatPicker::propagateIcons()
{
    const QMap<QAbstractButton*, QString> map = {
        {ui->categorySimpleButton, "add-stream"},
        {ui->categoryAudioButton, "stream-audio"},
        {ui->categoryVideoButton, "stream-video"}
    };
    Theme::setIcons(this, map);
}

/******************************************************************************
 ******************************************************************************/
void StreamFormatPicker::updateButtonBar()
{
    if (ui->categorySimpleButton->isChecked()) {
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
        ui->videoGroup->setVisible(!ui->categoryAudioButton->isChecked());
    }
}

/******************************************************************************
 ******************************************************************************/
void StreamFormatPicker::populateSimple(const QList<StreamFormat> &formats)
{
    bool checked = true;
    foreach (auto format, formats) {
        auto item = new QStandardItem(format.toString());

        item->setData(checked, StreamFormatPicker::CheckStateRole);
        checked = false;

        QVariant variant;
        variant.setValue(format.formatId);
        item->setData(variant, StreamFormatPicker::FormatIdRole);

        m_model->appendRow(item);
    }
    if (m_model->rowCount() > 0) {
        ui->listView->selectionModel()->setCurrentIndex(m_model->index(0, 0), QItemSelectionModel::ClearAndSelect);
    }
}

void StreamFormatPicker::populateComboBox(const QList<StreamFormat> &formats, QComboBox *comboBox)
{
    comboBox->clear();
    foreach (auto format, formats) {
        QVariant variant;
        variant.setValue(format.formatId);
        comboBox->addItem(format.toString(), variant);
    }
}

/******************************************************************************
 ******************************************************************************/
QModelIndex StreamFormatPicker::find(const StreamFormatId &id) const
{
    for (int row = 0; row < m_model->rowCount(); ++row) {
        auto index = m_model->index(row, 0);
        auto indexId = index.data(StreamFormatPicker::FormatIdRole).value<StreamFormatId>();
        if (id == indexId) {
            return index;
        }
    }
    return {};
}

void StreamFormatPicker::setCurrentSimple(const StreamFormatId &id)
{
    // Uncheck all rows
    for (int row = 0; row < m_model->rowCount(); ++row) {
        auto index = m_model->index(row, 0);
        m_model->setData(index, false, StreamFormatPicker::CheckStateRole);
    }

    // Check current or first row
    QModelIndex indexToSelect = find(id);
    if (indexToSelect.isValid()) {
        m_model->setData(indexToSelect, true, StreamFormatPicker::CheckStateRole);
        ui->categorySimpleButton->setChecked(true);
    } else {
        if (m_model->rowCount() > 0) {
            m_model->setData(m_model->index(0, 0), true, StreamFormatPicker::CheckStateRole);
        }
    }
}

void StreamFormatPicker::setCurrentAudio(const StreamFormatId &id)
{
    for (int i = 0; i < ui->audioComboBox->count(); ++i) {
        if (ui->audioComboBox->itemData(i, Qt::UserRole).value<StreamFormatId>() == id) {
            ui->audioComboBox->setCurrentIndex(i);
            if (!ui->categoryVideoButton->isChecked()) {
                ui->categoryAudioButton->setChecked(true);
            }
            return;
        }
    }
}

void StreamFormatPicker::setCurrentVideo(const StreamFormatId &id)
{
    for (int i = 0; i < ui->videoComboBox->count(); ++i) {
        if (ui->videoComboBox->itemData(i, Qt::UserRole).value<StreamFormatId>() == id) {
            ui->videoComboBox->setCurrentIndex(i);
            ui->categoryVideoButton->setChecked(true);
            return;
        }
    }
}

/******************************************************************************
 ******************************************************************************/
StreamFormatId StreamFormatPicker::currentSimple() const
{
    for (int row = 0; row < m_model->rowCount(); ++row) {
        auto index = m_model->index(row, 0);
        auto checked = index.data(StreamFormatPicker::CheckStateRole).toBool();
        if (checked) {
            return index.data(StreamFormatPicker::FormatIdRole).value<StreamFormatId>();
        }
    }
    return StreamFormatId();
}

StreamFormatId StreamFormatPicker::currentAudio() const
{
    return ui->audioComboBox->currentData(Qt::UserRole).value<StreamFormatId>();
}

StreamFormatId StreamFormatPicker::currentVideo() const
{
    return ui->videoComboBox->currentData(Qt::UserRole).value<StreamFormatId>();
}
