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

    state = DownloadItem::State::Idle;

    speed = 0;
    bytesReceived = 0;
    bytesTotal = 0;

    error = QNetworkReply::NoError;

    maxConnectionSegments = 4;
    maxConnections = 1;
}

/******************************************************************************
 ******************************************************************************/
DownloadItem::DownloadItem(DownloadManager *downloadManager) : QObject(downloadManager)
  , d(new DownloadItemPrivate(this))
{
    connect(&d->updateInfoTimer, SIGNAL(timeout()), this, SLOT(updateInfo()));
    d->networkManager = downloadManager->networkManager();
}

DownloadItem::~DownloadItem()
{
    if (d->file) {
        d->file->cancelWriting();
        d->file = Q_NULLPTR;
    }

    if (d->reply) {
        d->reply->abort();
        d->reply->deleteLater(); // ???
        d->reply = Q_NULLPTR;
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadItem::resume()
{
    qDebug() << Q_FUNC_INFO << localFullFileName() << d->resource->url();

    d->state = DownloadItem::Idle;
    emit changed();

    d->downloadTime.start();

    /* Ensure the destination directory exists */
    d->state = DownloadItem::Preparing;
    emit changed();

    QDir().mkpath(localFilePath());

    /// \todo options
    if (QFile::exists(localFullFileName())) {
        QFile::remove(localFullFileName());
    }

    if (d->file) {
        d->file->deleteLater();
        d->file = Q_NULLPTR;
    }
    d->file = new QSaveFile(this);
    d->file->setFileName(localFullFileName());
    if (!d->file->isOpen() && !d->file->open(QIODevice::WriteOnly)) {
        d->state = DownloadItem::FileError;
        emit changed();
        return;
    }

    /* Prepare the connection, try to contact the server */
    d->state = DownloadItem::Connecting;
    emit changed();

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

    d->updateInfoTimer.start(1000);

    /* The download starts to download */
    d->state = DownloadItem::Downloading;
    emit changed();
}

void DownloadItem::pause()
{
    // TO DO
    // https://kunalmaemo.blogspot.com/2011/07/simple-download-manager-with-pause.html

    stop();

    d->state = DownloadItem::Paused;
    emit changed();
}

void DownloadItem::stop()
{
    d->updateInfoTimer.stop();

    if (d->file) {
        d->file->cancelWriting();
        d->file = Q_NULLPTR;
    }

    if (d->reply) {
        d->reply->abort();
        d->reply->deleteLater(); // ???
        d->reply = Q_NULLPTR;
    }

    d->state = DownloadItem::Stopped;
    d->speed = 0;
    d->bytesReceived = 0;
    d->bytesTotal = 0;

    emit changed();
    emit finished();
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
    qDebug() << Q_FUNC_INFO << bytesReceived << bytesTotal;
    d->bytesReceived = bytesReceived;
    d->bytesTotal = bytesTotal;
    d->speed = bytesReceived * 1000.0 / d->downloadTime.elapsed();

    //emit changed();
}

void DownloadItem::onRedirected(const QUrl &url)
{
    qDebug() << Q_FUNC_INFO << url;
}

void DownloadItem::onFinished()
{
    qDebug() << Q_FUNC_INFO;
    d->updateInfoTimer.stop();

    if (d->state == DownloadItem::Downloading) {
        d->state = DownloadItem::Endgame;
        if (d->file->commit()) {
            d->state = DownloadItem::Completed;
        } else {
            d->state = DownloadItem::FileError;
        }
    }
    d->file->deleteLater();
    d->file = Q_NULLPTR;

    if (d->reply) {
        d->reply->deleteLater();
        d->reply = Q_NULLPTR;
    }

    emit changed();
    emit finished();
}

void DownloadItem::onError(QNetworkReply::NetworkError error)
{
    qDebug() << Q_FUNC_INFO;
    d->error = error;
    d->state = DownloadItem::NetworkError;
    emit changed();
    emit finished();
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
DownloadItem::State DownloadItem::state() const
{
    return d->state;
}

void DownloadItem::setState(const State status)
{
    d->state = status;
}

/******************************************************************************
 ******************************************************************************/
double DownloadItem::speed() const
{
    return d->state == DownloadItem::Downloading ? d->speed : -1;
}

qint64 DownloadItem::bytesReceived() const
{
    return d->bytesReceived;
}

void DownloadItem::setBytesReceived(qint64 bytesReceived)
{
    d->bytesReceived = bytesReceived;
}

qint64 DownloadItem::bytesTotal() const
{
    return d->bytesTotal;
}

void DownloadItem::setBytesTotal(qint64 bytesTotal)
{
    d->bytesTotal = bytesTotal;
}

int DownloadItem::progress() const
{
    if (d->bytesTotal > 0) {
        return (int)((qreal)d->bytesReceived / (qreal)d->bytesTotal * 100.0);
    } else {
        return 0;
    }
}

/******************************************************************************
 ******************************************************************************/
QNetworkReply::NetworkError DownloadItem::error() const
{
    return d->error;
}

void DownloadItem::setError(QNetworkReply::NetworkError error)
{
    d->error = error;
}

/******************************************************************************
 ******************************************************************************/
int DownloadItem::maxConnectionSegments() const
{
    return d->maxConnectionSegments;
}

void DownloadItem::setMaxConnectionSegments(int connectionSegments)
{
    if (connectionSegments > 0 && connectionSegments <= 10) {
        d->maxConnectionSegments = connectionSegments;
    }
}

/******************************************************************************
 ******************************************************************************/
int DownloadItem::maxConnections() const
{
    return d->maxConnections;
}

void DownloadItem::setMaxConnections(int connections)
{
    d->maxConnections = connections;
}

/******************************************************************************
 ******************************************************************************/
/**
 * The source Url
 */
QUrl DownloadItem::sourceUrl() const
{
    return d->resource->url();
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

/******************************************************************************
 ******************************************************************************/
bool DownloadItem::isResumable() const
{
    return d->state == DownloadItem::Idle ||
            d->state == DownloadItem::Paused ||
            d->state == DownloadItem::Stopped ||
            d->state == DownloadItem::Skipped ||
            d->state == DownloadItem::NetworkError ||
            d->state == DownloadItem::FileError;
}

bool DownloadItem::isPausable() const
{
    return d->state == DownloadItem::Idle ||
            d->state == DownloadItem::Preparing ||
            d->state == DownloadItem::Connecting ||
            d->state == DownloadItem::Downloading ||
            d->state == DownloadItem::Endgame;
}

bool DownloadItem::isCancelable() const
{
    return d->state == DownloadItem::Idle ||
            d->state == DownloadItem::Paused ||
            d->state == DownloadItem::Preparing ||
            d->state == DownloadItem::Connecting ||
            d->state == DownloadItem::Downloading ||
            d->state == DownloadItem::Endgame ||
            d->state == DownloadItem::Completed;
}

bool DownloadItem::isDownloading() const
{
    return isPausable();
}

/******************************************************************************
 ******************************************************************************/
QTime DownloadItem::remainingTime()
{
    return d->remainingTime;
}

QString DownloadItem::remaingTimeToString(QTime time)
{
    if (time < QTime(0, 0, 1)) {
        return "--:--";
    } else if (time < QTime(1, 0)) {
        return time.toString("mm:ss");
    } else {
        return time.toString("hh:mm:ss");
    }
}

QString DownloadItem::fileSizeToString(qint64 size)
{
    if (size < 0) {
        return tr("Unknown");
    }
    double correctSize = size / 1024.0; // KB
    if (correctSize < 1000) {
        return QString::number(correctSize > 1 ? correctSize : 1, 'f', 0) + " KB";
    }
    correctSize /= 1024; // MB
    if (correctSize < 1000) {
        return QString::number(correctSize, 'f', 1) + " MB";
    }
    correctSize /= 1024; // GB
    return QString::number(correctSize, 'f', 2) + " GB";
}

QString DownloadItem::currentSpeedToString(double speed)
{
    if (speed < 0) {
        return tr("-");
    }
    speed /= 1024; // kB
    if (speed < 1000) {
        return QString::number(speed, 'f', 0) + " KB/s";
    }
    speed /= 1024; //MB
    if (speed < 1000) {
        return QString::number(speed, 'f', 2) + " MB/s";
    }
    speed /= 1024; //GB
    return QString::number(speed, 'f', 2) + " GB/s";
}

void DownloadItem::updateInfo()
{
    int estimatedTime = ((d->bytesTotal - d->bytesReceived) / 1024) / (d->speed / 1024);
    QTime time;
    time = time.addSecs(estimatedTime);
    d->remainingTime = time;
    emit changed();
}
