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

#include "addstreamdialog.h"
#include "ui_addstreamdialog.h"

#include <Globals>
#include <Core/DownloadItem>
#include <Core/DownloadManager>
#include <Core/DownloadStreamItem>
#include <Core/ResourceItem>
#include <Core/Settings>
#include <Core/Theme>
#include <Widgets/UrlFormWidget>

#include <QtCore/QDebug>
#include <QtCore/QSettings>

constexpr int default_width = 800;
constexpr int default_height = 600;


AddStreamDialog::AddStreamDialog(const QUrl &url, DownloadManager *downloadManager,
                                 Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddStreamDialog)
    , m_downloadManager(downloadManager)
    , m_streamObjectDownloader(new StreamObjectDownloader(this))
    , m_settings(settings)
{
    ui->setupUi(this);

    setWindowTitle(QString("%0 - %1").arg(STR_APPLICATION_NAME, tr("Add Stream")));

    adjustSize();
    Theme::setIcons(this, { {ui->logo, "add-stream"} });

    ui->urlFormWidget->setExternalUrlLabelAndLineEdit(ui->urlLabel, ui->urlLineEdit);
    if (m_settings->isHttpReferringPageEnabled()) {
        ui->urlFormWidget->setReferringPage(m_settings->httpReferringPage());
    }

    connect(ui->urlLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onChanged(QString)));
    connect(ui->urlFormWidget, SIGNAL(changed(QString)), this, SLOT(onChanged(QString)));
    connect(ui->continueButton, SIGNAL(released()), this, SLOT(onContinueClicked()));

    connect(m_streamObjectDownloader, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(m_streamObjectDownloader, SIGNAL(collected(QList<StreamObject>)), this, SLOT(onCollected(QList<StreamObject>)));

    readSettings();

    ui->urlLineEdit->setText(url.toString());
    ui->urlLineEdit->setFocus();
    ui->urlLineEdit->setClearButtonEnabled(true);

    if (!url.isEmpty()) {
        onContinueClicked();
    }
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
    if (m_streamObjectDownloader->isRunning()) {
        m_streamObjectDownloader->stop();
        return;
    }
    setGuiEnabled(false);
    ui->streamListWidget->setMessageWait();
    const QString url = ui->urlLineEdit->text();
    m_streamObjectDownloader->runAsync(url);
    onChanged(QString());
}

void AddStreamDialog::onError(const QString &errorMessage)
{
    setGuiEnabled(true);
    ui->streamListWidget->setMessageError(errorMessage);
    onChanged(QString());
}

void AddStreamDialog::onCollected(const QList<StreamObject> &streamObjects)
{
    setGuiEnabled(true);
    ui->streamListWidget->setStreamObjects(streamObjects);
    onChanged(QString());
}

/******************************************************************************
 ******************************************************************************/
void AddStreamDialog::onChanged(QString)
{
    ui->continueButton->setEnabled(!ui->urlLineEdit->text().isEmpty());

    const bool enabled =
            ui->streamListWidget->isValid() &&
            ui->urlFormWidget->isValid();

    ui->startButton->setEnabled(enabled);
    ui->addPausedButton->setEnabled(enabled);
}

/******************************************************************************
 ******************************************************************************/
void AddStreamDialog::doAccept(bool started)
{
    m_downloadManager->append(createItems(), started);
    QDialog::accept();
    writeSettings();
}

/******************************************************************************
 ******************************************************************************/
QList<IDownloadItem*> AddStreamDialog::createItems() const
{
    QList<IDownloadItem*> items;
    foreach (auto item, ui->streamListWidget->selection()) {
        items << createItem(item);
    }
    return items;
}

IDownloadItem* AddStreamDialog::createItem(const StreamObject &streamObject) const
{
    auto resource = ui->urlFormWidget->createResourceItem();

    if (!streamObject.webpage_url.isEmpty()) {
        // Replace playlist URL with the video url
        resource->setUrl(streamObject.webpage_url);
    }

    resource->setType(ResourceItem::Type::Stream);
    resource->setStreamFileName(streamObject.fullFileName());
    resource->setStreamFileSize(streamObject.guestimateFullSize());
    resource->setStreamFormatId(streamObject.formatId().toString());

    auto item = new DownloadStreamItem(m_downloadManager);
    item->setResource(resource);
    return item;
}

/******************************************************************************
 ******************************************************************************/
void AddStreamDialog::setGuiEnabled(bool enabled)
{
    ui->continueButton->setText(enabled ? tr("Continue") : tr("Stop"));
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
    settings.beginGroup("StreamDialog");
    resize(settings.value("DialogSize", QSize(default_width, default_height)).toSize());

    QList<int> defaultWidths = {};
    QVariant variant = QVariant::fromValue(defaultWidths);
    ui->streamListWidget->setColumnWidths(settings.value("ColumnWidths", variant).value<QList<int> >());
    settings.endGroup();

    settings.beginGroup("Wizard");
    ui->urlFormWidget->setCurrentPath(settings.value("Path", QString()).toString());
    ui->urlFormWidget->setPathHistory(settings.value("PathHistory").toStringList());
    ui->urlFormWidget->setCurrentMask(settings.value("Mask", QString()).toString());
    settings.endGroup();
}

void AddStreamDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("StreamDialog");
    settings.setValue("DialogSize", size());
    settings.setValue("ColumnWidths", QVariant::fromValue(ui->streamListWidget->columnWidths()));
    settings.endGroup();

    settings.beginGroup("Wizard");
    settings.setValue("Path", ui->urlFormWidget->currentPath());
    settings.setValue("PathHistory", ui->urlFormWidget->pathHistory());
    settings.setValue("Mask", ui->urlFormWidget->currentMask());
    settings.endGroup();
}
