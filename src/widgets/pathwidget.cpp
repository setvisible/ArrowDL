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

#include "pathwidget.h"
#include "ui_pathwidget.h"

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QFileDialog>

PathWidget::PathWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::PathWidget)
  , m_suffix(QString())
  , m_suffixName(QString())
{
    ui->setupUi(this);

    ui->comboBox->setInputIsValidWhen( [](const QString &t) { return !t.isEmpty(); } );

    connect(ui->browseButton, SIGNAL(released()), this, SLOT(onBrowseButtonReleased()));
    connect(ui->comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onCurrentTextChanged(QString)));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onCurrentTextChanged(QString)));
}

PathWidget::~PathWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
QString PathWidget::currentPath() const
{
    return QDir::toNativeSeparators(ui->comboBox->currentText());
}

void PathWidget::setCurrentPath(const QString &path)
{
    const QString nativePath = path.isEmpty() ? path : QDir::toNativeSeparators(path);
    ui->comboBox->setCurrentText(nativePath);
}

/******************************************************************************
 ******************************************************************************/
QStringList PathWidget::pathHistory() const
{
    QStringList paths = ui->comboBox->history();
    for (int i = 0; i < paths.count(); ++i) {
        const QString nativePath = QDir::toNativeSeparators(paths.at(i));
        paths.replace(i, nativePath);
    }
    return paths;
}

void PathWidget::setPathHistory(const QStringList &paths)
{
    QStringList nativePaths;
    for (int i = 0; i < paths.count(); ++i) {
        const QString nativePath = QDir::toNativeSeparators(paths.at(i));
        nativePaths.append(nativePath);
    }
    ui->comboBox->setHistory(nativePaths);
}

/******************************************************************************
 ******************************************************************************/
PathWidget::PathType PathWidget::pathType() const
{
    return m_pathType;
}

void PathWidget::setPathType(PathType type)
{
    m_pathType = type;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Get the suffix (file's extension prepended with '.') of the selectable file.
 *
 * Example: ".txt"
 */
QString PathWidget::suffix() const
{
    return m_suffix;
}

/*!
 * Set the suffix (file's extension prepended with '.') of the selectable file.
 *
 * Example: ".txt"
 */
void PathWidget::setSuffix(const QString &suffix)
{
    m_suffix = suffix;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Example: "Text Files"
 */
QString PathWidget::suffixName() const
{
    return m_suffixName;
}

/*!
 * Example: "Text Files"
 */
void PathWidget::setSuffixName(const QString &suffixName)
{
    m_suffixName = suffixName;
}

/******************************************************************************
 ******************************************************************************/
void PathWidget::onBrowseButtonReleased()
{
    QString path = ui->comboBox->currentText();
    if (m_pathType == File) {
        const QString filter = tr("All Files (*);;%0 (*%1)").arg(m_suffixName, m_suffix);
        path = QFileDialog::getOpenFileName(this, tr("Please select a file"), path, filter);
    } else {
        if (path.isEmpty()) {
            path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        }
        path = QFileDialog::getExistingDirectory(this, tr("Please select a directory"), path);
    }
    if (!path.isEmpty()) {
        setCurrentPath(path);
    }
}

/******************************************************************************
 ******************************************************************************/
void PathWidget::onCurrentTextChanged(const QString &text)
{
    emit currentPathChanged(text);
    emit currentPathValidityChanged(ui->comboBox->isInputValid());
}
