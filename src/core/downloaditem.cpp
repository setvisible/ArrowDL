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

#include "downloaditem_p.h"

#include <Core/DownloadManager>
#include <Core/File>
#include <Core/ResourceItem>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtNetwork/QNetworkReply>

DownloadItemPrivate::DownloadItemPrivate(DownloadItem *qq)
    : q(qq)
{
    networkManager = Q_NULLPTR;
    resource = Q_NULLPTR;
    reply = Q_NULLPTR;
    file = new File(qq);
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

    File::OpenFlag flag = d->file->open(d->resource);

    if (flag == File::Skip) {
        setState(Skipped);
        onAboutToClose();
        onFinished();
        return;
    }

    const bool connected = flag == File::Open;

    /* Prepare the connection, try to contact the server */
    if (this->checkResume(connected)) {

        QNetworkRequest request;
        request.setUrl(d->resource->url());
#if QT_VERSION >= 0x050600 && QT_VERSION < 0x050900
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
#if QT_VERSION >= 0x050900
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                             QNetworkRequest::NoLessSafeRedirectPolicy);
#endif

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
    // TODO
    AbstractDownloadItem::pause();
}

void DownloadItem::stop()
{
    d->file->cancel();
    if (d->reply) {
        d->reply->abort();
        d->reply->deleteLater();
        d->reply = Q_NULLPTR;
    }
    AbstractDownloadItem::stop();
}

/******************************************************************************
 ******************************************************************************/
void DownloadItem::rename(const QString &newName)
{
    QString newCustomFileName = newName.trimmed().isEmpty() ? QString() : newName;
    const QString oldPath = d->resource->localFileFullPath(d->resource->customFileName());
    const QString newPath = d->resource->localFileFullPath(newCustomFileName);

    const QString oldFileName = d->resource->fileName();

    if (oldPath == newPath) {
        return;
    }
    bool success = true;
    if (QFile::exists(newPath)) {
        success = false; /* File error */
    }
    if (QFile::exists(oldPath) && !QFile::rename(oldPath, newPath)) {
        success = false; /* File error */
    }
    if (success) {
        d->resource->setCustomFileName(newCustomFileName);
        if (d->file->isOpen()) {
            d->file->rename(d->resource);
        }
    }
    const QString newFileName = success ? d->resource->fileName() : newName;
    emit renamed(oldFileName, newFileName, success);
}

/******************************************************************************
 ******************************************************************************/
void DownloadItem::onMetaDataChanged()
{
#if defined QT_DEBUG
    /*
     * Check if the metadata change is a redirection
     */
    if (d->reply) {
        const QUrl oldUrl = d->resource->url();
        const QUrl newUrl = d->reply->header(QNetworkRequest::LocationHeader).toUrl();
        if (newUrl.isValid() && oldUrl.isValid() && oldUrl != newUrl) {
            qDebug() << Q_FUNC_INFO
                     << "redirecting " << oldUrl.toString()
                     << "to" << newUrl.toString();
            return;
        }
    }
#endif
    qDebug() << Q_FUNC_INFO;
}

void DownloadItem::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug() << Q_FUNC_INFO << bytesReceived << "/" << bytesTotal;

    updateInfo(bytesReceived, bytesTotal);
}

void DownloadItem::onRedirected(const QUrl &url)
{
    qDebug() << Q_FUNC_INFO << url.toString();
}

void DownloadItem::onFinished()
{
    qDebug() << Q_FUNC_INFO << state();

    switch (state()) {
    case Idle:
    case Preparing:
    case Connecting:
    case Downloading:
    case Endgame:
    case Completed:
        if (bytesTotal() == 0) {
            /*
             * Trick:
             * Server can close invalid connection by sending an empty reply.
             * In this case, QNetworkAccessManager triggers finished(),
             * but doesn't trigger error().
             * Here we verify the size of the received file and set the error.
             */
            setState(NetworkError);
            setBytesReceived(0);
            // setBytesTotal(0);
            d->file->cancel();
            emit changed();
        } else {
            /* Here, finish the operation if downloading. */
            /* If network error or file error, just ignore */
            bool commited = d->file->commit();
            preFinish(commited);
        }
        break;

    case Paused:
    case Stopped:
    case Skipped:
    case NetworkError:
    case FileError:
        setBytesReceived(0);
        setBytesTotal(0);
        d->file->cancel();
        emit changed();
        break;

    default:
        Q_UNREACHABLE();
        break;
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

    d->file->cancel();
    setHttpErrorNumber(static_cast<int>(error));
    setState(NetworkError);
}

void DownloadItem::onReadyRead()
{
#if !defined QT_DEBUG
    qDebug() << Q_FUNC_INFO;
#endif
    if (!d->reply || !d->file) {
        return;
    }
    QByteArray data = d->reply->readAll();
#if defined QT_DEBUG
    qDebug() << Q_FUNC_INFO << "<<" << data.size() << "bytes";
#endif
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
