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

#include "downloadstreamitem.h"

#include <Core/DownloadManager>
#include <Core/File>
#include <Core/ResourceItem>
#include <Core/Stream>

/******************************************************************************
 ******************************************************************************/
DownloadStreamItem::DownloadStreamItem(DownloadManager *downloadManager)
    : DownloadItem(downloadManager)
    , m_stream(Q_NULLPTR)
{
}

/******************************************************************************
 ******************************************************************************/
void DownloadStreamItem::resume()
{
    logInfo(QString("Resume '%0' (destination: '%1').").arg(resource()->url(), localFullFileName()));

    this->beginResume();

    File::OpenFlag flag = file()->open(resource());

    if (flag == File::Skip) {
        setState(Skipped);
        onFinished();
        return;
    }

    const bool connected = flag == File::Open;

    /* Prepare the connection, try to contact the server */
    if (this->checkResume(connected)) {

        if (m_stream) {
            m_stream->deleteLater();
            m_stream = Q_NULLPTR;
        }
        m_stream = new Stream(this);

        const QString outputPath = localFullFileName();
        m_stream->setLocalFullOutputPath(outputPath);

        m_stream->setUrl(resource()->url());
        m_stream->setReferringPage(resource()->referringPage());
        m_stream->setSelectedFormatId(StreamFormatId(resource()->streamFormatId()));
        m_stream->setFileSizeInBytes(resource()->streamFileSize());

        connect(m_stream, SIGNAL(downloadMetadataChanged()), this, SLOT(onMetaDataChanged()));
        connect(m_stream, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress(qint64,qint64)));
        connect(m_stream, SIGNAL(downloadError(QString)), this, SLOT(onError(QString)));
        connect(m_stream, SIGNAL(downloadFinished()), this, SLOT(onFinished()));

        logInfo(m_stream->command());

        m_stream->start();

        this->tearDownResume();
    }
}

void DownloadStreamItem::pause()
{
    /// \todo implement?
    logInfo(QString("Pause '%0'.").arg(resource()->url()));
    AbstractDownloadItem::pause();
}

void DownloadStreamItem::stop()
{
    logInfo(QString("Stop '%0'.").arg(resource()->url()));
    file()->cancel();
    if (m_stream) {
        m_stream->abort();
        m_stream->deleteLater();
        m_stream = Q_NULLPTR;
    }
    AbstractDownloadItem::stop();
}

/******************************************************************************
 ******************************************************************************/
void DownloadStreamItem::onMetaDataChanged()
{
    auto oldFileName = resource()->streamFileName();
    auto newFileName = m_stream->fileName();
    if (oldFileName != newFileName) {
        logInfo(QString("HTTP redirect: '%0' to '%1'.").arg(oldFileName, newFileName));
        resource()->setStreamFileName(newFileName);
    }
}

void DownloadStreamItem::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesReceived > 0 && bytesTotal > 0) {
        logInfo(QString("Downloaded '%0' (%1 of %2 bytes).")
                .arg(resource()->url(),
                     QString::number(bytesReceived),
                     QString::number(bytesTotal)));
    }
    updateInfo(bytesReceived, bytesTotal);
}

void DownloadStreamItem::onFinished()
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
            file()->cancel();
            emit changed();
        } else {
            /* Here, finish the operation if downloading. */
            /* If network error or file error, just ignore */

            // bool commited = file()->commit();
            file()->cancel();       /* HACK */
            bool commited = true;   /* HACK */
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
        file()->cancel();
        emit changed();
        break;
    }
    this->finish();
}

void DownloadStreamItem::onError(const QString &errorMessage)
{
    logInfo(QString("Error '%0': '%1'.").arg(resource()->url(), errorMessage));
    file()->cancel();
    setErrorMessage(errorMessage);
    setState(NetworkError);
}
