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

#include "addtorrentdialog.h"
#include "ui_addtorrentdialog.h"

#include <Core/DownloadItem>
#include <Core/DownloadManager>
#include <Core/DownloadTorrentItem>
#include <Core/ResourceItem>
#include <Core/Settings>
#include <Widgets/UrlFormWidget>

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtGui/QCloseEvent>


AddTorrentDialog::AddTorrentDialog(const QUrl &url, DownloadManager *downloadManager,
                                   Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddTorrentDialog)
    , m_downloadManager(downloadManager)
    , m_settings(settings)
{
    ui->setupUi(this);

    adjustSize();
    setFixedHeight(height());

    ui->urlFormWidget->setExternalUrlLabelAndLineEdit(ui->urlLabel, ui->urlLineEdit);

    ui->urlLineEdit->setText(url.toString());
    ui->urlLineEdit->setFocus();

    connect(ui->urlLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onChanged(QString)));
    connect(ui->urlFormWidget, SIGNAL(changed(QString)), this, SLOT(onChanged(QString)));

    readSettings();
}

AddTorrentDialog::~AddTorrentDialog()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
bool AddTorrentDialog::isTorrentUrl(const QUrl &url)
{
    if (url.scheme().toLower() == QLatin1String("magnet")) {
        return true;

    } else {
        QFileInfo fi(url.path());
        if (fi.suffix().toLower() == QLatin1String("torrent")) {
            return true;
        }
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
    m_downloadManager->append(toList(createItem(url)), started);
    QDialog::accept();
    writeSettings();
}

/******************************************************************************
 ******************************************************************************/
IDownloadItem* AddTorrentDialog::createItem(const QString &url) const
{
    auto resource = ui->urlFormWidget->createResourceItem();
    resource->setUrl(url);
    resource->setTorrentEnabled(true);


    // resource->setUrl("magnet:?xt=urn:btih:b17e2c6ce8d901a59b77f68781500640e5c0d917&dn=Bad.Boys.for.Life.2020.1080p.WEBRip.x264.AAC5.1-MP4&tr=udp%3A%2F%2Ftracker.openbittorrent.com%3A80&tr=udp%3A%2F%2Ftracker.publicbt.com%3A80&tr=udp%3A%2F%2Ftracker.ccc.de%3A80");
    //  resource->setCustomFileName("[Wait... Downloading metadata...].torrent");

    auto item = new DownloadTorrentItem(m_downloadManager);
    item->setResource(resource);
    return item;
}

inline QList<IDownloadItem*> AddTorrentDialog::toList(IDownloadItem *item)
{
    return QList<IDownloadItem*>() << item;
}

/******************************************************************************
 ******************************************************************************/
void AddTorrentDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    ui->urlFormWidget->setCurrentPath(settings.value("Path", QString()).toString());
    ui->urlFormWidget->setPathHistory(settings.value("PathHistory").toStringList());
    ui->urlFormWidget->setCurrentMask(settings.value("Mask", QString()).toString());
    settings.endGroup();
}

void AddTorrentDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    settings.setValue("Path", ui->urlFormWidget->currentPath());
    settings.setValue("PathHistory", ui->urlFormWidget->pathHistory());
    settings.setValue("Mask", ui->urlFormWidget->currentMask());
    settings.endGroup();
}
