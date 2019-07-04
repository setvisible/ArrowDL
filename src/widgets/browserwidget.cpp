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

#include "browserwidget.h"
#include "ui_browserwidget.h"

#include <QDir>
#include <QFileDialog>

BrowserWidget::BrowserWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::BrowserWidget)
  , m_type(File)
  , m_extensionType(".txt")
  , m_extensionName("Text Files")
{
    ui->setupUi(this);

    connect(ui->browseButton, SIGNAL(released()),
            this, SLOT(onBrowseButtonReleased()));
    connect(ui->comboBox, SIGNAL(currentTextChanged(QString)),
            this, SLOT(onCurrentTextChanged(QString)));
}

BrowserWidget::~BrowserWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
QString BrowserWidget::text() const
{
    return QDir::toNativeSeparators(ui->comboBox->currentText());
}

void BrowserWidget::setText(const QString &text)
{
    ui->comboBox->setCurrentText(QDir::toNativeSeparators(text));
}

/******************************************************************************
 ******************************************************************************/
BrowserWidget::Type BrowserWidget::type() const
{
    return m_type;
}

void BrowserWidget::setType(Type type)
{
    m_type = type;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Example: ".txt"
 */
QString BrowserWidget::extensionType() const
{
    return m_extensionType;
}

void BrowserWidget::setExtensionType(const QString &extension)
{
    m_extensionType = extension;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Example: "Text Files"
 */
QString BrowserWidget::extensionName() const
{
    return m_extensionName;
}

void BrowserWidget::setExtensionName(const QString &extensionName)
{
    m_extensionName = extensionName;
}

/******************************************************************************
 ******************************************************************************/
void BrowserWidget::onBrowseButtonReleased()
{
    if (m_type == File) {
        QString filter = tr("All Files (*);;%0 (*%1)")
                .arg(m_extensionName)
                .arg(m_extensionType);
        QString fileName
                = QFileDialog::getOpenFileName(
                    this, tr("Select File"), ui->comboBox->currentText(), filter);
        if (!fileName.isEmpty()) {
            setText(fileName);
        }

    } else {
        QString directory
                = QFileDialog::getExistingDirectory(
                    this, tr("Directory"), ui->comboBox->currentText());
        if (!directory.isEmpty()) {
            setText(directory);
        }
    }
}

void BrowserWidget::onCurrentTextChanged(const QString &text)
{
    emit textChanged(text);
}
