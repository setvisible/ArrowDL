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

DownloadTorrentItem::~DownloadTorrentItem()
{
}

/******************************************************************************
 ******************************************************************************/
void DownloadTorrentItem::setResource(ResourceItem *resource)
{
    /*
     * For torrent, mask is "*name*.*ext*" to not make extra sub-directories.
     * Subdirs are already managed by the torrent engine.
     */
    resource->setMask(QLatin1String("*name*.*ext*"));

    DownloadItem::setResource(resource);

    /*
     * Reimplement this method to detect when the URL of the resource is
     * available, so that TorrentContext can (down-)load the metadata as soon
     * as possible.
     */

    setState(IDownloadItem::Preparing);
    emit changed();

    // Download the metadata (the .torrent file) if not already downloaded
    TorrentContext::getInstance().prepareTorrent(this);
    emit changed();
}

/******************************************************************************
 ******************************************************************************/
QString DownloadTorrentItem::status() const
{
    return d->info.torrentStateString();
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

    // requires a GUI update signal, because metaInfo can change
    // even if the downloadItem is Paused or Stopped
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
    // qDebug() << Q_FUNC_INFO << info.torrentStateString() << info.bytesTotal << info.bytesReceived;
    d->info = info;

    IDownloadItem::State downloadItemState;

    switch (static_cast<int>(info.state)) {
    case TorrentInfo::stopped:
        downloadItemState = IDownloadItem::Paused;

        break;
    case TorrentInfo::checking_files:
    case TorrentInfo::downloading_metadata:
        downloadItemState = IDownloadItem::Preparing;

        break;
    case TorrentInfo::downloading:
        downloadItemState = IDownloadItem::Downloading;
        updateInfo(info.bytesReceived, info.bytesTotal);

        break;
    case TorrentInfo::finished:
    case TorrentInfo::seeding:
    case TorrentInfo::allocating:
    case TorrentInfo::checking_resume_data:
        downloadItemState = IDownloadItem::Completed;

        // here, info.bytesTotal == 0 !
        updateInfo(d->metaInfo.initialMetaInfo.bytesTotal,
                   d->metaInfo.initialMetaInfo.bytesTotal);

        break;
    default:
        Q_UNREACHABLE();
        break;
    }

    setState(downloadItemState);
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
void DownloadTorrentItem::resume()
{
    if (isPreparing()) {
        return;
    }

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
        bool ok = TorrentContext::getInstance().addTorrent(this);
        if (!ok) {
            d->metaInfo.error.type = TorrentError::FailedToAddError;
            d->metaInfo.error.message = tr("Bad .torrent format: Can't download it.");
            stop();
            return;
        }

    }
    TorrentContext::getInstance().resumeTorrent(this);
    this->tearDownResume();
}

void DownloadTorrentItem::pause()
{
    if (isPreparing()) {
        TorrentContext::getInstance().stopPrepare(this);

    } else {
        if (TorrentContext::getInstance().hasTorrent(this)) {
            TorrentContext::getInstance().pauseTorrent(this);
        }
    }
    AbstractDownloadItem::pause();
}

void DownloadTorrentItem::stop()
{
    file()->cancel();

    if (isPreparing()) {
        TorrentContext::getInstance().stopPrepare(this);

    } else {
        if (TorrentContext::getInstance().hasTorrent(this)) {
            TorrentContext::getInstance().removeTorrent(this);
        }
    }
    AbstractDownloadItem::stop();
}

/******************************************************************************
 ******************************************************************************/
bool DownloadTorrentItem::isPreparing() const
{
    return state() == Preparing;
}
