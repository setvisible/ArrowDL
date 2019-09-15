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

#include "maskwidget.h"
#include "ui_maskwidget.h"

#define MAX_HISTORY_COUNT 10

MaskWidget::MaskWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::MaskWidget)
  , m_colorizeErrorsEnabled(true)
{
    ui->setupUi(this);

    ui->comboBox->setDuplicatesEnabled(false);
    ui->comboBox->setMaxCount(MAX_HISTORY_COUNT);

    connect(ui->comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onCurrentTextChanged(QString)));
}

MaskWidget::~MaskWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
QString MaskWidget::currentMask() const
{
    return ui->comboBox->currentText();
}

void MaskWidget::setCurrentMask(const QString &text)
{
    if (text.isEmpty() && ui->comboBox->count() > 0) {
        ui->comboBox->setCurrentIndex(0);
    } else {
        ui->comboBox->setCurrentText(text);
    }
}

/******************************************************************************
 ******************************************************************************/
bool MaskWidget::colorizeErrors() const
{
    return m_colorizeErrorsEnabled;
}

void MaskWidget::setColorizeErrors(bool enabled)
{
    m_colorizeErrorsEnabled = enabled;
}

/******************************************************************************
 ******************************************************************************/
void MaskWidget::onCurrentTextChanged(const QString &text)
{
    colorizeErrors(text);
    emit currentMaskChanged(text);
}

/******************************************************************************
 ******************************************************************************/
inline void MaskWidget::colorizeErrors(const QString &text)
{
    if (m_colorizeErrorsEnabled && text.isEmpty()) {
        ui->comboBox->setStyleSheet(
                    QLatin1String("QComboBox { background-color: rgb(255, 100, 100); }"));
    } else {
        ui->comboBox->setStyleSheet(QString());
    }
}
