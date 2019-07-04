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

MaskWidget::MaskWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::MaskWidget)
{
    ui->setupUi(this);
    connect(ui->comboBox, SIGNAL(currentTextChanged(QString)),
            this, SLOT(onCurrentTextChanged(QString)));
}

MaskWidget::~MaskWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
QString MaskWidget::text() const
{
    return ui->comboBox->currentText();
}

void MaskWidget::setText(const QString &text)
{    
    ui->comboBox->setCurrentText(text);
}

/******************************************************************************
 ******************************************************************************/
void MaskWidget::onCurrentTextChanged(const QString &text)
{
    emit textChanged(text);
}
