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

#include "downloaditem_p.h"

#include <Core/DownloadManager>
#include <Core/File>
#include <Core/NetworkManager>
#include <Core/ResourceItem>
#include <Core/Settings>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtNetwork/QNetworkReply>

DownloadItemPrivate::DownloadItemPrivate(DownloadItem *qq)
    : q(qq)
{
    file = new File(qq);
}

/******************************************************************************
 ******************************************************************************/
DownloadItem::DownloadItem(DownloadManager *downloadManager) : AbstractDownloadItem(downloadManager)
  , d(new DownloadItemPrivate(this))
{
    d->downloadManager = downloadManager;
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
    logInfo(QString("Resume '%0' (destination: '%1').").arg(d->resource->url(), localFullFileName()));

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

        QUrl url(d->resource->url());
        d->reply = d->downloadManager->networkManager()->get(url);
        d->reply->setParent(this);

        /* Signals/Slots of QNetworkReply */
        connect(d->reply, SIGNAL(metaDataChanged()), this, SLOT(onMetaDataChanged()));
        connect(d->reply, SIGNAL(downloadProgress(qsizetype, qsizetype)),
                this, SLOT(onDownloadProgress(qsizetype, qsizetype)));
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
    /// \todo implement?
    logInfo(QString("Pause '%0'.").arg(d->resource->url()));
    AbstractDownloadItem::pause();
}

void DownloadItem::stop()
{
    logInfo(QString("Stop '%0'.").arg(d->resource->url()));
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
    if (d->reply) {
        auto rawNewUrl = d->reply->header(QNetworkRequest::LocationHeader);
        if (rawNewUrl.isValid()) {
            const QUrl oldUrl = d->resource->url();
            const QUrl newUrl = rawNewUrl.toUrl();
            /* Check if the metadata change is a redirection */
            if (newUrl.isValid() && oldUrl.isValid() && oldUrl != newUrl) {
                logInfo(QString("HTTP redirect: '%0' to '%1'.").arg(oldUrl.toString(), newUrl.toString()));
            }
        }
        auto settings = d->downloadManager->settings();
        auto rawTime = d->reply->header(QNetworkRequest::LastModifiedHeader);
        if (settings && rawTime.isValid()) {
            auto time = rawTime.toDateTime();
            if (settings->isRemoteCreationTimeEnabled()) {
                d->file->setCreationFileTime(time);
            }
            if (settings->isRemoteLastModifiedTimeEnabled()) {
                d->file->setLastModifiedFileTime(time);
            }
            if (settings->isRemoteAccessTimeEnabled()) {
                d->file->setAccessFileTime(time);
            }
            if (settings->isRemoteMetadataChangeTimeEnabled()) {
                d->file->setMetadataChangeFileTime(time);
            }
        }
    }
}

void DownloadItem::onDownloadProgress(qsizetype bytesReceived, qsizetype bytesTotal)
{
    if (d->reply && bytesReceived > 0 && bytesTotal > 0) {
        logInfo(QString("Downloaded '%0' (%1 of %2 bytes).")
                .arg(d->reply->url().toString(),
                     QString::number(bytesReceived),
                     QString::number(bytesTotal)));
    }
    updateInfo(bytesReceived, bytesTotal);
}

void DownloadItem::onRedirected(const QUrl &url)
{
    if (d->reply) {
        logInfo(QString("HTTP redirect: redirected '%0' to '%1'.")
                .arg(d->reply->url().toString(), url.toString()));
    }
}

void DownloadItem::onFinished()
{
    logInfo(QString("Finished (%0) '%1'.").arg(state_c_str(), localFullFileName()));
    switch (state()) {
    case Idle:
    case Preparing:
    case Connecting:
    case DownloadingMetadata:
    case Downloading:
    case Endgame:
    case Completed:
    case Seeding:
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
    }
    if (d->reply) {
        d->reply->deleteLater();
        d->reply = Q_NULLPTR;
    }
    this->finish();
}

QString DownloadItem::statusToHttp(QNetworkReply::NetworkError error)
{
    /*
     * See QNetworkReply::NetworkError Documentation for conversion
     *
     * In particular:
     * QNetworkReply::NetworkError
     *      QNetworkReplyWasmImplPrivate::statusCodeFromHttp(
     *          int httpStatusCode, const QUrl &url)
     */
    switch (error) {
    case QNetworkReply::NoError:   return tr("No Error");

        // network layer errors [relating to the destination server] (1-99):
    case QNetworkReply::ConnectionRefusedError:           return tr("3xx Redirect connection refused");
    case QNetworkReply::RemoteHostClosedError:            return tr("3xx Redirect remote host closed");
    case QNetworkReply::HostNotFoundError:                return tr("3xx Redirect host not found");
    case QNetworkReply::TimeoutError:                     return tr("3xx Redirect timeout");
    case QNetworkReply::OperationCanceledError:           return tr("3xx Redirect operation canceled");
    case QNetworkReply::SslHandshakeFailedError:          return tr("3xx Redirect SSL handshake failed");
    case QNetworkReply::TemporaryNetworkFailureError:     return tr("3xx Redirect temporary network failure");
    case QNetworkReply::NetworkSessionFailedError:        return tr("3xx Redirect network session failed");
    case QNetworkReply::BackgroundRequestNotAllowedError: return tr("3xx Redirect background request not allowed");
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    case QNetworkReply::TooManyRedirectsError:            return tr("3xx Too many redirects");
    case QNetworkReply::InsecureRedirectError:            return tr("3xx Insecure redirect");
#endif
    case QNetworkReply::UnknownNetworkError:              return tr("3xx Unknown redirect error");

        // proxy errors (101-199):
    case QNetworkReply::ProxyConnectionRefusedError :     return tr("5xx Proxy connection refused");
    case QNetworkReply::ProxyConnectionClosedError:       return tr("5xx Proxy connection closed");
    case QNetworkReply::ProxyNotFoundError:               return tr("5xx Proxy not found");
    case QNetworkReply::ProxyTimeoutError:                return tr("504 Proxy timeout error");
    case QNetworkReply::ProxyAuthenticationRequiredError: return tr("407 Proxy authentication required");
    case QNetworkReply::UnknownProxyError:                return tr("5xx Unknown proxy error");

        // content errors (201-299):
    case QNetworkReply::ContentAccessDenied:                return tr("403 Access denied");
    case QNetworkReply::ContentOperationNotPermittedError:  return tr("405 Method not allowed");
    case QNetworkReply::ContentNotFoundError:               return tr("404 Not found");
    case QNetworkReply::AuthenticationRequiredError:        return tr("401 Authorization required");
    case QNetworkReply::ContentReSendError:     return tr("4xx Resend error");
    case QNetworkReply::ContentConflictError:   return tr("409 Conflict");
    case QNetworkReply::ContentGoneError:       return tr("410 Content no longer available");
    case QNetworkReply::UnknownContentError:    return tr("4xx Unknown content error");

        // protocol errors (301-399):
    case QNetworkReply::ProtocolUnknownError: return tr("4xx Unknown protocol error");
    case QNetworkReply::ProtocolInvalidOperationError:  return tr("400 Bad request");
    case QNetworkReply::ProtocolFailure:     return tr("4xx Protocol failure");

        // Server side errors (401-499):
    case QNetworkReply::InternalServerError:          return tr("500 Internal server error");
    case QNetworkReply::OperationNotImplementedError: return tr("501 Server does not support this functionality");
    case QNetworkReply::ServiceUnavailableError:      return tr("503 Service unavailable");
    case QNetworkReply::UnknownServerError:           return tr("5xx Unknown serveur error");
    }
    Q_UNREACHABLE();
}

void DownloadItem::onError(QNetworkReply::NetworkError error)
{
    /// \todo Use instead: auto reply = qobject_cast<QNetworkReply*>(sender());
    if (d->reply) {
        logInfo(QString("Error '%0': '%1'.").arg(d->reply->url().toString(),d->reply->errorString()));
    }
    d->file->cancel();
    auto httpError = statusToHttp(error);
    setErrorMessage(httpError);
    setState(NetworkError);
}

void DownloadItem::onReadyRead()
{
    if (!d->reply || !d->file) {
        return;
    }
    QByteArray data = d->reply->readAll();
    d->file->write(data);
}

void DownloadItem::onAboutToClose()
{
    logInfo(QString("Finished (%0) '%1'.").arg(state_c_str(), localFullFileName()));
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
File* DownloadItem::file() const
{
    return d->file;
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
