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

#include "downloaditem_p.h"

#include <Core/DownloadManager>
#include <Core/ResourceItem>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QSaveFile>
#include <QtNetwork/QNetworkReply>

DownloadItemPrivate::DownloadItemPrivate(DownloadItem *qq)
    : q(qq)
{
    networkManager = Q_NULLPTR;
    resource = Q_NULLPTR;
    reply = Q_NULLPTR;
    file = Q_NULLPTR;
}

/******************************************************************************
 ******************************************************************************/
DownloadItem::DownloadItem(DownloadManager *downloadManager) : AbstractDownloadItem(downloadManager)
  , d(new DownloadItemPrivate(this))
{
    d->networkManager = downloadManager->networkManager();
}

DownloadItem::~DownloadItem()
{
    if (d->file) {
        d->file->cancelWriting();
        d->file->deleteLater();
        d->file = Q_NULLPTR;
    }

    if (d->reply) {
        d->reply->abort();
        d->reply->deleteLater();
        d->reply = Q_NULLPTR;
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadItem::resume()
{
    qDebug() << Q_FUNC_INFO << localFullFileName() << d->resource->url();

    this->beginResume();

    QDir().mkpath(localFilePath());

    /// \todo Add more options
    if (QFile::exists(localFullFileName())) {
        QFile::remove(localFullFileName());
    }

    if (d->file) {
        d->file->deleteLater();
        d->file = Q_NULLPTR;
    }
    d->file = new QSaveFile(this);
    d->file->setFileName(localFullFileName());
    const bool connected = d->file->isOpen() || d->file->open(QIODevice::WriteOnly);

    /* Prepare the connection, try to contact the server */
    if (this->checkResume(connected)) {

        QNetworkRequest request;
        request.setUrl(d->resource->url());

        d->reply = d->networkManager->get(request);
        d->reply->setParent(this);

        /* Signals/Slots of QNetworkReply */
        connect(d->reply, SIGNAL(metaDataChanged()), this, SLOT(onMetaDataChanged()));
        connect(d->reply, SIGNAL(downloadProgress(qint64,qint64)),
                this, SLOT(onDownloadProgress(qint64,qint64)));
        connect(d->reply, SIGNAL(redirected(QUrl)), this, SLOT(onRedirected(QUrl)));
        connect(d->reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(onError(QNetworkReply::NetworkError)));
        connect(d->reply, SIGNAL(finished()), this, SLOT(onFinished()));

        /* Signals/Slots of QIODevice */
        connect(d->reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        connect(d->reply, SIGNAL(aboutToClose()), this, SLOT(onAboutToClose()));

        this->tearDownResume();
    }
}

void DownloadItem::pause()
{
    // TO DO
    // https://kunalmaemo.blogspot.com/2011/07/simple-download-manager-with-pause.html

    AbstractDownloadItem::pause();
}

void DownloadItem::stop()
{
    if (d->file) {
        d->file->cancelWriting();
        d->file->deleteLater();
        d->file = Q_NULLPTR;
    }

    if (d->reply) {
        d->reply->abort();
        d->reply->deleteLater();
        d->reply = Q_NULLPTR;
    }

    AbstractDownloadItem::stop();
}

/******************************************************************************
 ******************************************************************************/
void DownloadItem::onMetaDataChanged()
{
    qDebug() << Q_FUNC_INFO;
    if (d->reply) {
        QUrl locationHeader = d->reply->header(QNetworkRequest::LocationHeader).toUrl();
        if (locationHeader.isValid()) {
            d->reply->close();
            d->reply->deleteLater();

            Q_ASSERT(d->networkManager);
            d->reply = d->networkManager->get(QNetworkRequest(locationHeader));
            resume();
        }
    }
}

void DownloadItem::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug() << Q_FUNC_INFO << bytesReceived << "/" << bytesTotal;

    updateInfo(bytesReceived, bytesTotal);
}

void DownloadItem::onRedirected(const QUrl &url)
{
    qDebug() << Q_FUNC_INFO << url;
}

void DownloadItem::onFinished()
{
    qDebug() << Q_FUNC_INFO << state();

    switch (state()) {
    case Idle:
    case Preparing:
    case Connecting:
        break;

    case Downloading:
    case Endgame:
    case Completed:
    {
        /* Here, finish the operation if downloading. */
        /* If network error or file error, just ignore */
        if (d->file) {
            bool commited = d->file->commit();
            preFinish(commited);
        }
    }
        break;

    case Paused:
    case Stopped:
    case Skipped:
    case NetworkError:
    case FileError:
        if (d->file) {
            d->file->cancelWriting();
        }
        setBytesReceived(0);
        setBytesTotal(0);
        break;

    default:
        Q_UNREACHABLE();
        break;
    }
    if (d->file) {
        d->file->deleteLater();
        d->file = Q_NULLPTR;
    }
    if (d->reply) {
        d->reply->deleteLater();
        d->reply = Q_NULLPTR;
    }

    this->finish();
}

void DownloadItem::onError(QNetworkReply::NetworkError error)
{
    qDebug() << Q_FUNC_INFO;

    if (d->file) {
        d->file->cancelWriting();
        d->file->deleteLater();
        d->file = Q_NULLPTR;
    }
    setHttpErrorNumber((int) error);
    setState(NetworkError);
}

void DownloadItem::onReadyRead()
{
    qDebug() << Q_FUNC_INFO;

    if (!d->reply || !d->file) {
        return;
    }
    QByteArray data = d->reply->readAll();
    d->file->write(data);

}

void DownloadItem::onAboutToClose()
{
    qDebug() << Q_FUNC_INFO;
}

/******************************************************************************
 ******************************************************************************/
ResourceItem* DownloadItem::resource() const
{
    return d->resource;
}

void DownloadItem::setResource(ResourceItem *resource)
{
    d->resource = resource;
}

/******************************************************************************
 ******************************************************************************/
/**
 * The source Url
 */
QUrl DownloadItem::sourceUrl() const
{
    return QUrl(d->resource->url());
}

/**
 * The destination's full file name
 */
QString DownloadItem::localFullFileName() const
{
    const QUrl target = d->resource->localFileUrl();
    return target.toLocalFile();
}

/**
 * The destination's file name
 */
QString DownloadItem::localFileName() const
{
    const QUrl target = d->resource->localFileUrl();
    const QFileInfo fi(target.toLocalFile());
    return fi.fileName();
}

/**
 * The destination's absolute path
 */
QString DownloadItem::localFilePath() const
{
    const QUrl target = d->resource->localFileUrl();
    const QFileInfo fi(target.toLocalFile());
    return fi.absolutePath();
}

QUrl DownloadItem::localFileUrl() const
{
    return d->resource->localFileUrl();
}

QUrl DownloadItem::localDirUrl() const
{
    return QUrl::fromLocalFile(localFilePath());
}
