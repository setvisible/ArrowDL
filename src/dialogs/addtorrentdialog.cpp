/* - ArrowDL - Copyright (C) 2019-present Sebastien Vavassori
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

#include "addtorrentdialog.h"
#include "ui_addtorrentdialog.h"

#include <Constants>
#include <Core/DownloadFileItem>
#include <Core/DownloadManager>
#include <Core/JobTorrent>
#include <Core/ResourceItem>
#include <Core/Settings>
#include <Core/Theme>
#include <Widgets/UrlFormWidget>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtGui/QCloseEvent>


AddTorrentDialog::AddTorrentDialog(
    const QUrl &url,
    DownloadManager *downloadManager,
    Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddTorrentDialog)
    , m_downloadManager(downloadManager)
    , m_settings(settings)
{
    ui->setupUi(this);

    setWindowTitle(QString("%0 - %1").arg(STR_APPLICATION_NAME, tr("Add Magnet Links and Torrent")));

    // QLayout::SetFixedSize
    //   Although the name doesn't sound like it will work,
    //   it ensures that your layout will *only* use the space it needs.
    // layout()->setSizeConstraint(QLayout::SetFixedSize);

    Theme::setIcons(this, { {ui->logo, "add-torrent"} });

    ui->urlFormWidget->setExternalUrlLabelAndLineEdit(ui->urlLabel, ui->urlLineEdit);
    if (m_settings->isHttpReferringPageEnabled()) {
        ui->urlFormWidget->setReferringPage(m_settings->httpReferringPage());
    }

    // The input URL can be a .torrent on local drive, or a remote .torrent.
    if (url.isLocalFile()) {
        ui->urlLineEdit->setText(QDir::toNativeSeparators(url.toLocalFile()));
    } else {
        ui->urlLineEdit->setText(url.toString());
    }
    ui->urlLineEdit->setFocus();
    ui->urlLineEdit->setClearButtonEnabled(true);

    connect(ui->urlLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onChanged(QString)));
    connect(ui->urlFormWidget, SIGNAL(changed(QString)), this, SLOT(onChanged(QString)));

    readUiSettings();
}

AddTorrentDialog::~AddTorrentDialog()
{
    writeUiSettings();
    delete ui;
}

void AddTorrentDialog::readUiSettings()
{
    QSettings settings;
    settings.beginGroup("TorrentDialog");
    resize(settings.value("DialogSize", size()).toSize());
    settings.endGroup();
}

void AddTorrentDialog::writeUiSettings()
{
    QSettings settings;
    settings.beginGroup("TorrentDialog");
    settings.setValue("DialogSize", size());
    settings.endGroup();
}

/******************************************************************************
 ******************************************************************************/
bool AddTorrentDialog::isTorrentUrl(const QUrl &url)
{
    if (url.scheme().toLower() == QLatin1String("magnet")) {
        return true;
    }
    QFileInfo fi(url.path());
    if (fi.suffix().toLower() == QLatin1String("torrent")) {
        return true;
    }
    return false;
}

/******************************************************************************
 ******************************************************************************/
void AddTorrentDialog::accept()
{
    doAccept(true);
}

void AddTorrentDialog::acceptPaused()
{
    doAccept(false);
}

void AddTorrentDialog::reject()
{
    QDialog::reject();
}

/******************************************************************************
 ******************************************************************************/
void AddTorrentDialog::onChanged(QString)
{
    const bool enabled = ui->urlFormWidget->isValid();
    ui->startButton->setEnabled(enabled);
    ui->addPausedButton->setEnabled(enabled);
}

/******************************************************************************
 ******************************************************************************/
void AddTorrentDialog::doAccept(bool started)
{
    const QString url = ui->urlFormWidget->url();
    m_downloadManager->append(toList(createTorrentItem(url)), started);
    QDialog::accept();
}

/******************************************************************************
 ******************************************************************************/
AbstractDownloadItem* AddTorrentDialog::createTorrentItem(const QString &url) const
{
    auto resource = ui->urlFormWidget->createResourceItem();
    resource->setUrl(url);
    resource->setType(ResourceItem::Type::Torrent);
    auto item = new JobTorrent(m_downloadManager, resource);
    return item;
}

inline QList<AbstractDownloadItem*> AddTorrentDialog::toList(AbstractDownloadItem *item)
{
    return QList<AbstractDownloadItem*>() << item;
}
