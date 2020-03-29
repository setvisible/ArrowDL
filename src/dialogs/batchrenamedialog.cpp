/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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

#include "batchrenamedialog.h"
#include "ui_batchrenamedialog.h"

#include <Core/DownloadItem>
#include <Core/IDownloadItem>
#include <Core/ResourceItem>

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QAbstractItemView>


class PopupItemDelegate: public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(40);
        return size;
    }
};

BatchRenameDialog::BatchRenameDialog(const QList<IDownloadItem*> &items, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BatchRenameDialog)
    , m_items(items)
{
    ui->setupUi(this);

    ui->subtitleLabel->setText(tr("%0 selected files to rename").arg(m_items.count()));

    ui->comboBox->view()->setItemDelegate(new PopupItemDelegate(ui->comboBox));

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onComboboxChanged(int)));

    readSettings();
}

BatchRenameDialog::~BatchRenameDialog()
{
    delete ui;
}

void BatchRenameDialog::closeEvent(QCloseEvent *)
{
    writeSettings();
}

/******************************************************************************
 ******************************************************************************/
void BatchRenameDialog::accept()
{
    writeSettings();
    switch (ui->comboBox->currentIndex()) {
    case 0:
        renameToDefault();
        break;
    case 1:
    default:
        renameToEnumeration();
        break;
    }

    QDialog::accept();
}

/******************************************************************************
 ******************************************************************************/
void BatchRenameDialog::renameToDefault()
{
    foreach (auto item, m_items) {
        DownloadItem *downloadItem = static_cast<DownloadItem*>(item);
        rename(downloadItem, QString());
    }
}

void BatchRenameDialog::renameToEnumeration()
{
    const int count = m_items.count();
    const int from = ui->startSpinBox->value();
    const int by = ui->incrementSpinBox->value();

    int digits = 0;
    if (ui->fillRadioButton->isChecked()) {
        int last = from + count * by;
        QString lastName = QString("%0").arg(last);
        digits = lastName.count();

    } else if (ui->customFillRadioButton->isChecked()) {
        digits = ui->digitSpinBox->value();
    }

    int i = from;
    foreach (auto item, m_items) {
        DownloadItem *downloadItem = static_cast<DownloadItem*>(item);
        QString newName = QString("%0").arg(i);
        if (digits > newName.count()) {
            newName = newName.rightJustified(digits, QChar('0'));
        }
        rename(downloadItem, newName);
        i += by;
    }
}

void BatchRenameDialog::rename(DownloadItem *downloadItem, const QString &newName)
{
    if (!downloadItem)
        return;

    if (ui->safeRenameCheckBox->isChecked()) {
        auto resource = downloadItem->resource();
        resource->setCustomFileName(newName);
        downloadItem->stop();
        downloadItem->pause();
    } else {
        downloadItem->rename(newName);
    }
}

/******************************************************************************
 ******************************************************************************/
void BatchRenameDialog::onComboboxChanged(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

/******************************************************************************
 ******************************************************************************/
int BatchRenameDialog::currentRadio() const
{
    return ui->fillRadioButton->isChecked()
            ? 1
            : ui->customFillRadioButton->isChecked()
              ? 2 : 0;
}

void BatchRenameDialog::setCurrentRadio(int index)
{
    switch (index) {
    case 1:
        ui->fillRadioButton->setChecked(true);
        break;
    case 2:
        ui->customFillRadioButton->setChecked(true);
        break;
    case 0:
    default:
        ui->classicRadioButton->setChecked(true);
        break;
    }
}

/******************************************************************************
 ******************************************************************************/
void BatchRenameDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("BatchRenaming");
    ui->comboBox->setCurrentIndex(settings.value("Index", 0).toInt());
    this->setCurrentRadio(settings.value("RadioIndex", 0).toInt());
    ui->startSpinBox->setValue( settings.value("StartFrom", 1).toInt());
    ui->incrementSpinBox->setValue(settings.value("IncrementBy", 1).toInt());
    ui->digitSpinBox->setValue(settings.value("NumberOfDigits", 3).toInt());
    ui->safeRenameCheckBox->setChecked(settings.value("SafeRename", true).toBool());
    settings.endGroup();
}

void BatchRenameDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("BatchRenaming");
    settings.setValue("Index", ui->comboBox->currentIndex());
    settings.setValue("RadioIndex", this->currentRadio());
    settings.setValue("StartFrom", ui->startSpinBox->value());
    settings.setValue("IncrementBy", ui->incrementSpinBox->value());
    settings.setValue("NumberOfDigits", ui->digitSpinBox->value());
    settings.setValue("SafeRename", ui->safeRenameCheckBox->isChecked());
    settings.endGroup();
}
