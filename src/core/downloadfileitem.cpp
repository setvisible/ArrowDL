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

#include "downloadfileitem.h"

#include <Core/DownloadManager>
#include <Core/File>
#include <Core/NetworkManager>
#include <Core/ResourceItem>
#include <Core/Settings>

#include <QtNetwork/QNetworkReply>

using namespace Qt::Literals::StringLiterals;


DownloadFileItem::DownloadFileItem(QObject *parent, ResourceItem *resource)
    : AbstractDownloadItem(parent, resource)
    , m_downloadManager((DownloadManager*)parent)
    , m_reply(nullptr)
{
}

DownloadFileItem::~DownloadFileItem()
{
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadFileItem::resume()
{
    logInfo(QString("Resume '%0' (destination: '%1').").arg(m_resource->url(), localFullFileName()));

    this->beginResume();

    auto flag = m_file->open(m_resource);

    if (flag == File::Skip) {
        setState(Skipped);
        onAboutToClose();
        onFinished();
        return;
    }

    auto connected = flag == File::Open;

    /* Prepare the connection, try to contact the server */
    if (this->checkResume(connected)) {

        auto url = m_resource->url_TODO();
        m_reply = m_downloadManager->networkManager()->get(url);
        m_reply->setParent(this);

        /* Signals/Slots of QNetworkReply */
        connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(onMetaDataChanged()));
        connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress(qint64,qint64)));
        connect(m_reply, SIGNAL(redirected(QUrl)), this, SLOT(onRedirected(QUrl)));
        connect(m_reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(onErrorOccurred(QNetworkReply::NetworkError)));
        connect(m_reply, SIGNAL(finished()), this, SLOT(onFinished()));

        /* Signals/Slots of QIODevice */
        connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        connect(m_reply, SIGNAL(aboutToClose()), this, SLOT(onAboutToClose()));

        this->tearDownResume();
    }
}

void DownloadFileItem::pause()
{
    AbstractDownloadItem::pause();
}

void DownloadFileItem::stop()
{
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    AbstractDownloadItem::stop();
}

/******************************************************************************
 ******************************************************************************/
void DownloadFileItem::onMetaDataChanged()
{
    if (m_reply) {
        auto rawNewUrl = m_reply->header(QNetworkRequest::LocationHeader);
        if (rawNewUrl.isValid()) {
            auto oldUrl = m_resource->url_TODO();
            auto newUrl = rawNewUrl.toUrl();
            /* Check if the metadata change is a redirection */
            if (newUrl.isValid() && oldUrl.isValid() && oldUrl != newUrl) {
                logInfo(QString("HTTP redirect: '%0' to '%1'.").arg(oldUrl.toString(), newUrl.toString()));
            }
        }
        auto settings = m_downloadManager->settings();
        auto rawTime = m_reply->header(QNetworkRequest::LastModifiedHeader);
        if (settings && rawTime.isValid()) {
            auto time = rawTime.toDateTime();
            if (settings->isRemoteCreationTimeEnabled()) {
                m_file->setCreationFileTime(time);
            }
            if (settings->isRemoteLastModifiedTimeEnabled()) {
                m_file->setLastModifiedFileTime(time);
            }
            if (settings->isRemoteAccessTimeEnabled()) {
                m_file->setAccessFileTime(time);
            }
            if (settings->isRemoteMetadataChangeTimeEnabled()) {
                m_file->setMetadataChangeFileTime(time);
            }
        }
    }
}

void DownloadFileItem::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (m_reply && bytesReceived > 0 && bytesTotal > 0) {
        logInfo(QString("Downloaded '%0' (%1 of %2 bytes).")
                .arg(m_reply->url().toString(),
                     QString::number(bytesReceived),
                     QString::number(bytesTotal)));
    }
    updateInfo(static_cast<qsizetype>(bytesReceived),
               static_cast<qsizetype>(bytesTotal));
}

void DownloadFileItem::onRedirected(const QUrl &url)
{
    if (m_reply) {
        logInfo(QString("HTTP redirect: redirected '%0' to '%1'.")
                .arg(m_reply->url().toString(), url.toString()));
    }
}

void DownloadFileItem::onFinished()
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
            m_file->cancel();
            emit changed();
        } else {
            /* Here, finish the operation if downloading. */
            /* If network error or file error, just ignore */
            bool commited = m_file->commit();
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
        m_file->cancel();
        emit changed();
        break;
    }
    if (m_reply) {
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    this->finish();
}

QString DownloadFileItem::statusToHttp(QNetworkReply::NetworkError error)
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
    case QNetworkReply::TooManyRedirectsError:            return tr("3xx Too many redirects");
    case QNetworkReply::InsecureRedirectError:            return tr("3xx Insecure redirect");
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
    case QNetworkReply::UnknownServerError:           return tr("5xx Unknown server error");
    }
    Q_UNREACHABLE();
}

void DownloadFileItem::onErrorOccurred(QNetworkReply::NetworkError error)
{
    /// \todo Use instead: auto reply = qobject_cast<QNetworkReply*>(sender());
    if (m_reply) {
        logInfo(QString("Error '%0': '%1'.").arg(m_reply->url().toString(),m_reply->errorString()));
    }
    m_file->cancel();
    auto httpError = statusToHttp(error);
    setErrorMessage(httpError);
    setState(NetworkError);
}

void DownloadFileItem::onReadyRead()
{
    if (!m_reply || !m_file) {
        return;
    }
    QByteArray data = m_reply->readAll();
    m_file->write(data);
}

void DownloadFileItem::onAboutToClose()
{
    logInfo(QString("Finished (%0) '%1'.").arg(state_c_str(), localFullFileName()));
}
