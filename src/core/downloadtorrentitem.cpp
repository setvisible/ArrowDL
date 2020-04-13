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

#include "downloadtorrentitem_p.h"

#include <Core/DownloadManager>
#include <Core/File>
#include <Core/Format>
#include <Core/ResourceItem>
#include <Core/TorrentContext>


DownloadTorrentItem::DownloadTorrentItem(DownloadManager *downloadManager)
    : DownloadItem(downloadManager)
    , d(new DownloadTorrentItemPrivate(this))
{
}

DownloadTorrentItem::DownloadTorrentItem(DownloadManager *downloadManager, QByteArray torrentData)
    : DownloadItem(downloadManager)
    , d(new DownloadTorrentItemPrivate(this))
{
    setTorrentData(torrentData);
}

DownloadTorrentItem::~DownloadTorrentItem()
{
}

/******************************************************************************
 ******************************************************************************/
QByteArray DownloadTorrentItem::torrentData() const
{
    return d->m_torrentData;
}

void DownloadTorrentItem::setTorrentData(const QByteArray &torrentData)
{
    d->m_torrentData = torrentData;
    emit changed();
}

/******************************************************************************
 ******************************************************************************/
TorrentMetaInfo DownloadTorrentItem::metaInfo() const
{
    return d->metaInfo;
}

void DownloadTorrentItem::setMetaInfo(TorrentMetaInfo metaInfo)
{
    d->metaInfo = metaInfo;
    emit changed();
}

/******************************************************************************
 ******************************************************************************/
TorrentInfo DownloadTorrentItem::info() const
{
    return d->info;
}

void DownloadTorrentItem::setInfo(TorrentInfo info)
{
    d->info = info;
    updateInfo(info.bytesReceived, info.bytesTotal);
    setState(info.status);
}
/******************************************************************************
 ******************************************************************************/
TorrentHandleInfo DownloadTorrentItem::detail() const
{
    return d->detail;
}

void DownloadTorrentItem::setDetail(TorrentHandleInfo detail)
{
    d->detail = detail;
}

/******************************************************************************
 ******************************************************************************/
QAbstractTableModel* DownloadTorrentItem::fileModel() const
{
    return d->m_fileModel;
}

QAbstractTableModel* DownloadTorrentItem::peerModel() const
{
    return d->m_peerModel;
}

QAbstractTableModel* DownloadTorrentItem::trackerModel() const
{
    return d->m_trackerModel;
}

/******************************************************************************
 ******************************************************************************/
QList<int> DownloadTorrentItem::defaultFileColumnWidths() const
{
    return d->m_fileModel->defaultColumnWidths();
}

QList<int> DownloadTorrentItem::defaultPeerColumnWidths() const
{
    return d->m_peerModel->defaultColumnWidths();
}

QList<int> DownloadTorrentItem::defaultTrackerColumnWidths() const
{
    return d->m_trackerModel->defaultColumnWidths();
}


/******************************************************************************
 ******************************************************************************/
void DownloadTorrentItem::setResource(ResourceItem *resource)
{
    DownloadItem::setResource(resource);
    //    if (resource) {
    //        m_torrent->setUrl(resource->url());// weird
    //        m_torrent->setTorrentData(m_torrent->torrentData());
    //    }
    d->startTorrentInfoAsync(); // hack to detect when resource's URL is available
}

/******************************************************************************
 ******************************************************************************/
/*
 * DownloadTorrentItem is a container of sub-items.
 * So we need to override these methods.
 */
QUrl DownloadTorrentItem::sourceUrl() const
{
    QUrl url = DownloadItem::sourceUrl();
    url.setHost("peer-to-peer"); // dummy hostname, to differentiate torrent vs. classic item
    return url;
}

/******************************************************************************
 ******************************************************************************/
void DownloadTorrentItem::resume()
{
    qDebug() << Q_FUNC_INFO
             << localFullFileName()  // localdrive/destination/t.torrent
             << resource()->url() // remote/origine/t.torrent
             << localFilePath(); // localdrive/destination/

    this->beginResume();

    /*
     * Contrary to other downloadItem types,
     * we don't check if the torrent already exists.
     */

    if (!TorrentContext::getInstance().hasTorrent(this)) {
        TorrentContext::getInstance().addTorrent(this);
    }
    TorrentContext::getInstance().resumeTorrent(this);
    this->tearDownResume();
}

void DownloadTorrentItem::pause()
{
    TorrentContext::getInstance().pauseTorrent(this);
    AbstractDownloadItem::pause();
}

void DownloadTorrentItem::stop()
{
    file()->cancel();
    if (TorrentContext::getInstance().hasTorrent(this)) {
        TorrentContext::getInstance().removeTorrent(this);
    }
    AbstractDownloadItem::stop();
}


/******************************************************************************
 ******************************************************************************/
void DownloadTorrentItem::onMetaDataChanged()
{
    qDebug() << Q_FUNC_INFO;
}

void DownloadTorrentItem::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug() << Q_FUNC_INFO << bytesReceived << "/" << bytesTotal;
    updateInfo(bytesReceived, bytesTotal);
}

void DownloadTorrentItem::onFinished()
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
            emit changed();
        } else {
            /* Here, finish the operation if downloading. */
            /* If network error or file error, just ignore */

            // bool commited = file()->commit();
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
        emit changed();
        break;

    default:
        Q_UNREACHABLE();
        break;
    }
    this->finish();
}

void DownloadTorrentItem::onError(QString errorMessage)
{
    qDebug() << Q_FUNC_INFO << errorMessage;
    setHttpErrorNumber(static_cast<int>(404));
    setStreamErrorMessage(errorMessage);
    setState(NetworkError);
}
