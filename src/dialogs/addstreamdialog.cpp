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

#include <Globals>
#include <Core/DownloadItem>
#include <Core/DownloadManager>
#include <Core/DownloadStreamItem>
#include <Core/ResourceItem>
#include <Core/Settings>
#include <Core/Stream>
#include <Widgets/UrlFormWidget>

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

    setWindowTitle(QString("%0 - %1").arg(STR_APPLICATION_NAME).arg(tr("Add Stream")));

    adjustSize();

    ui->urlFormWidget->setExternalUrlLabelAndLineEdit(ui->urlLabel, ui->urlLineEdit);

    ui->urlLineEdit->setText(url.toString());
    ui->urlLineEdit->setFocus();
    ui->streamWidget->setState(StreamWidget::Empty);


    connect(ui->urlLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onChanged(QString)));
    connect(ui->urlFormWidget, SIGNAL(changed(QString)), this, SLOT(onChanged(QString)));
    connect(ui->continueButton, SIGNAL(released()), this, SLOT(onContinueClicked()));

    connect(m_streamInfoDownloader, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(m_streamInfoDownloader, SIGNAL(collected(StreamInfosPtr)), this, SLOT(onCollected(StreamInfosPtr)));

    readSettings();

    onContinueClicked();
}

AddStreamDialog::~AddStreamDialog()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
bool AddStreamDialog::isStreamUrl(const QUrl &url, const Settings *settings)
{
    if (url.isLocalFile()) {
        return false;
    }
    if (settings->isStreamHostEnabled()) {
        auto host = url.host();
        auto regexHosts = settings->streamHosts();
        return Stream::matchesHost(host, regexHosts);
    }
    return false;
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

    const QString url = ui->urlLineEdit->text();
    m_streamInfoDownloader->runAsync(url);

    onChanged(QString());
}

void AddStreamDialog::onError(QString errorMessage)
{
    setGuiEnabled(true);
    ui->streamWidget->showErrorMessage(errorMessage);
    onChanged(QString());
}

void AddStreamDialog::onCollected(StreamInfosPtr infos)
{
    setGuiEnabled(true);
    ui->streamWidget->showStreamInfos(infos);
    onChanged(QString());
}

/******************************************************************************
 ******************************************************************************/
void AddStreamDialog::onChanged(QString)
{
    ui->continueButton->setEnabled(!ui->urlLineEdit->text().isEmpty());

    const bool enabled =
            ui->streamWidget->state() == StreamWidget::Normal &&
            ui->urlFormWidget->isValid();

    ui->startButton->setEnabled(enabled);
    ui->addPausedButton->setEnabled(enabled);
}

/******************************************************************************
 ******************************************************************************/
void AddStreamDialog::doAccept(bool started)
{
    /// \todo implement playlist download:
    /// maybe open a new dialog to select which video/option to download

    m_downloadManager->append(toList(createItem()), started);
    QDialog::accept();
    writeSettings();
}

/******************************************************************************
 ******************************************************************************/
IDownloadItem* AddStreamDialog::createItem() const
{
    auto resource = ui->urlFormWidget->createResourceItem();

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
    ui->urlLineEdit->setEnabled(enabled);
    ui->continueButton->setEnabled(enabled);
    ui->urlFormWidget->setChildrenEnabled(enabled);
    ui->startButton->setEnabled(enabled);
    ui->addPausedButton->setEnabled(enabled);
}

/******************************************************************************
 ******************************************************************************/
void AddStreamDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    ui->urlFormWidget->setCurrentPath(settings.value("Path", QString()).toString());
    ui->urlFormWidget->setPathHistory(settings.value("PathHistory").toStringList());
    ui->urlFormWidget->setCurrentMask(settings.value("Mask", QString()).toString());
    settings.endGroup();
}

void AddStreamDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    settings.setValue("Path", ui->urlFormWidget->currentPath());
    settings.setValue("PathHistory", ui->urlFormWidget->pathHistory());
    settings.setValue("Mask", ui->urlFormWidget->currentMask());
    settings.endGroup();
}
