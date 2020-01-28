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

#include "addstreamdialog.h"
#include "ui_addstreamdialog.h"

#include <Core/DownloadItem>
#include <Core/DownloadManager>
#include <Core/DownloadStreamItem>
#include <Core/ResourceItem>
#include <Core/Settings>
#include <Core/Stream>

#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>


AddStreamDialog::AddStreamDialog(const QUrl &url, DownloadManager *downloadManager,
                                 Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddStreamDialog)
    , m_downloadManager(downloadManager)
    , m_streamInfoDownloader(new StreamInfoDownloader(this))
    , m_settings(settings)
{
    ui->setupUi(this);

    ui->pathWidget->setPathType(PathWidget::Directory);

    ui->downloadLineEdit->setText(url.toString());
    ui->downloadLineEdit->setFocus();
    ui->streamWidget->setState(StreamWidget::Empty);

    connect(ui->downloadLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onChanged(QString)));
    connect(ui->pathWidget, SIGNAL(currentPathChanged(QString)), this, SLOT(onChanged(QString)));
    connect(ui->maskWidget, SIGNAL(currentMaskChanged(QString)), this, SLOT(onChanged(QString)));
    connect(ui->continueButton, SIGNAL(released()), this, SLOT(onContinueClicked()));

    connect(m_streamInfoDownloader, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(m_streamInfoDownloader, SIGNAL(collected(StreamInfos*)), this, SLOT(onCollected(StreamInfos*)));

    readSettings();
}

AddStreamDialog::~AddStreamDialog()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void AddStreamDialog::accept()
{
    doAccept(true);
}

void AddStreamDialog::acceptPaused()
{
    doAccept(false);
}

void AddStreamDialog::reject()
{
    QDialog::reject();
}

/******************************************************************************
 ******************************************************************************/
void AddStreamDialog::onContinueClicked()
{
    setGuiEnabled(false);
    ui->streamWidget->setState(StreamWidget::Downloading);

    const QString url = ui->downloadLineEdit->text();
    m_streamInfoDownloader->runAsync(url);

    onChanged(QString());
}

void AddStreamDialog::onError(QString errorMessage)
{
    setGuiEnabled(true);
    ui->streamWidget->showErrorMessage(errorMessage);
    onChanged(QString());
}

void AddStreamDialog::onCollected(StreamInfos *infos)
{
    setGuiEnabled(true);
    ui->streamWidget->showStreamInfos(infos);
    onChanged(QString());
}

/******************************************************************************
 ******************************************************************************/
void AddStreamDialog::onChanged(QString)
{
    ui->continueButton->setEnabled(!ui->downloadLineEdit->text().isEmpty());

    const bool enabled =
            ui->streamWidget->state() == StreamWidget::Normal &&
            !ui->downloadLineEdit->text().isEmpty() &&
            !ui->pathWidget->currentPath().isEmpty() &&
            !ui->maskWidget->currentMask().isEmpty();

    ui->startButton->setEnabled(enabled);
    ui->addPausedButton->setEnabled(enabled);
}

/******************************************************************************
 ******************************************************************************/
void AddStreamDialog::doAccept(bool started)
{
    const QString input = ui->downloadLineEdit->text();
    const QUrl url(input);

    // Remove trailing / and \ and .
    const QString adjusted = url.adjusted(QUrl::StripTrailingSlash).toString();

    // TODO if is list :open a new dialo to select the videos ?

    m_downloadManager->append(toList(createItem(adjusted)), started);
    QDialog::accept();
    writeSettings();
}

/******************************************************************************
 ******************************************************************************/
IDownloadItem* AddStreamDialog::createItem(const QString &url) const
{
    auto resource = new ResourceItem();
    resource->setUrl(url);
    resource->setCustomFileName(ui->filenameLineEdit->text());
    resource->setReferringPage(ui->referrerLineEdit->text());
    resource->setDescription(ui->descriptionLineEdit->text());
    resource->setDestination(ui->pathWidget->currentPath());
    resource->setMask(ui->maskWidget->currentMask());
    resource->setCheckSum(ui->hashLineEdit->text());

    resource->setStreamEnabled(true);
    resource->setStreamFileName(ui->streamWidget->fileName());
    resource->setStreamFileSize(ui->streamWidget->fileSize());
    resource->setStreamFormatId(ui->streamWidget->selectedFormatId());

    auto item = new DownloadStreamItem(m_downloadManager);
    item->setResource(resource);
    return item;
}

inline QList<IDownloadItem*> AddStreamDialog::toList(IDownloadItem *item)
{
    return QList<IDownloadItem*>() << item;
}

/******************************************************************************
 ******************************************************************************/
void AddStreamDialog::setGuiEnabled(bool enabled)
{
    ui->downloadLineEdit->setEnabled(enabled);
    ui->continueButton->setEnabled(enabled);
    ui->filenameLineEdit->setEnabled(enabled);
    ui->referrerLineEdit->setEnabled(enabled);
    ui->descriptionLineEdit->setEnabled(enabled);
    ui->pathWidget->setEnabled(enabled);
    ui->maskWidget->setEnabled(enabled);
    ui->hashLineEdit->setEnabled(enabled);

    ui->startButton->setEnabled(enabled);
    ui->addPausedButton->setEnabled(enabled);
}

/******************************************************************************
 ******************************************************************************/
void AddStreamDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    ui->pathWidget->setCurrentPath(settings.value("Path", QString()).toString());
    ui->pathWidget->setPathHistory(settings.value("PathHistory").toStringList());
    ui->maskWidget->setCurrentMask(settings.value("Mask", QString()).toString());
    settings.endGroup();
}

void AddStreamDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    settings.setValue("Path", ui->pathWidget->currentPath());
    settings.setValue("PathHistory", ui->pathWidget->pathHistory());
    settings.setValue("Mask", ui->maskWidget->currentMask());
    settings.endGroup();
}
