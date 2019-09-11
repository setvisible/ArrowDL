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

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QAction>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>

#define MAX_HISTORY_COUNT 10


BrowserWidget::BrowserWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::BrowserWidget)
  , m_pathType(File)
  , m_suffix(QString())
  , m_suffixName(QString())
{
    ui->setupUi(this);

    ui->comboBox->setDuplicatesEnabled(false);
    ui->comboBox->setMaxCount(MAX_HISTORY_COUNT);

    connect(ui->browseButton, SIGNAL(released()), this, SLOT(onBrowseButtonReleased()));
    connect(ui->comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onCurrentTextChanged(QString)));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onCurrentTextChanged(QString)));

    ui->comboBox->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->comboBox, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));
}

BrowserWidget::~BrowserWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
QString BrowserWidget::currentPath() const
{
    return QDir::toNativeSeparators(ui->comboBox->currentText());
}

void BrowserWidget::setCurrentPath(const QString &path)
{
    if (path.isEmpty() && ui->comboBox->count() > 0) {
        ui->comboBox->setCurrentIndex(0);
    } else {
        const QString nativePath = QDir::toNativeSeparators(path);
        removePathfromHistory(nativePath);
        ui->comboBox->insertItem(0, nativePath);
        ui->comboBox->setCurrentText(nativePath);
    }
}

/******************************************************************************
 ******************************************************************************/
QStringList BrowserWidget::pathHistory() const
{
    QStringList ret;
    for (int i = 0; i < ui->comboBox->count(); ++i) {
        ret.append(QDir::toNativeSeparators(ui->comboBox->itemText(i)));
    }
    return ret;
}

void BrowserWidget::setPathHistory(const QStringList &paths)
{
    clearHistory();
    const int count = qMin(MAX_HISTORY_COUNT, paths.count());
    for (int i = 0; i < count; ++i) {
        const QString nativePath = QDir::toNativeSeparators(paths.at(i));
        removePathfromHistory(nativePath);
        ui->comboBox->addItem(nativePath);
    }
}

/******************************************************************************
 ******************************************************************************/
BrowserWidget::PathType BrowserWidget::pathType() const
{
    return m_pathType;
}

void BrowserWidget::setPathType(PathType type)
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
QString BrowserWidget::suffix() const
{
    return m_suffix;
}

/*!
 * Set the suffix (file's extension prepended with '.') of the selectable file.
 *
 * Example: ".txt"
 */
void BrowserWidget::setSuffix(const QString &suffix)
{
    m_suffix = suffix;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Example: "Text Files"
 */
QString BrowserWidget::suffixName() const
{
    return m_suffixName;
}

/*!
 * Example: "Text Files"
 */
void BrowserWidget::setSuffixName(const QString &suffixName)
{
    m_suffixName = suffixName;
}

/******************************************************************************
 ******************************************************************************/
void BrowserWidget::clearHistory()
{
    const QString path = currentPath();
    ui->comboBox->clear();
    setCurrentPath(path);
}

/******************************************************************************
 ******************************************************************************/
void BrowserWidget::removePathfromHistory(const QString &path)
{
    int i = ui->comboBox->count();
    while (i > 0) {
        i--;
        if (ui->comboBox->itemText(i) == QDir::toNativeSeparators(path)) {
            ui->comboBox->removeItem(i);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void BrowserWidget::onBrowseButtonReleased()
{
    QString path = ui->comboBox->currentText();
    if (m_pathType == File) {
        const QString filter = tr("All Files (*);;%0 (*%1)").arg(m_suffixName).arg(m_suffix);
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
void BrowserWidget::onCurrentTextChanged(const QString &text)
{
    emit currentPathChanged(text);
}

/******************************************************************************
 ******************************************************************************/
void BrowserWidget::showContextMenu(const QPoint &/*pos*/)
{
    QMenu *contextMenu = ui->comboBox->lineEdit()->createStandardContextMenu();

    contextMenu->addSeparator();
    QAction *clearAction = contextMenu->addAction(tr("Clear History"));
    connect(clearAction, SIGNAL(triggered()), this, SLOT(clearHistory()));

    contextMenu->exec(QCursor::pos());
    contextMenu->deleteLater();
}
