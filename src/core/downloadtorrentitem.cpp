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

#include "downloadtorrentitem.h"

#include <Core/DownloadManager>
#include <Core/File>
#include <Core/Format>
#include <Core/ResourceItem>
#include <Core/Torrent>
#include <Core/TorrentContext>


DownloadTorrentItem::DownloadTorrentItem(DownloadManager *downloadManager)
    : DownloadItem(downloadManager)
    , m_torrent(new Torrent(this))
{
    connect(m_torrent, &Torrent::changed, this, &DownloadTorrentItem::onTorrentChanged);

}

/******************************************************************************
 ******************************************************************************/
/*!
 * \reimp
 * Reimplement this method to detect when the URL of the resource is
 * available, so that TorrentContext can (down-)load the metadata as soon
 * as possible.
 */
void DownloadTorrentItem::setResource(ResourceItem *resource)
{
    if (!resource) {
        return;
    }
    /*
     * For torrent, mask is "*name*.*ext*" to not make extra sub-directories.
     * Subdirs are already managed by the torrent engine.
     */
    resource->setMask(QLatin1String("*name*.*ext*"));

    DownloadItem::setResource(resource);


    m_torrent->setLocalFullFileName(this->localFullFileName());
    m_torrent->setLocalFilePath(this->localFilePath());
    m_torrent->setUrl(this->resource()->url());

    QString fileStates = this->resource()->torrentPreferredFilePriorities();
    // Download the metadata (the .torrent file) if not already downloaded
    TorrentContext::getInstance().prepareTorrent(m_torrent);

    // At this point, the torrent is loaded.

    // Restore the previous session's data.
    m_torrent->setPreferredFilePriorities(fileStates);
    emit changed();
}

/******************************************************************************
 ******************************************************************************/
Torrent* DownloadTorrentItem::torrent() const
{
    return m_torrent;
}

/******************************************************************************
 ******************************************************************************/
void DownloadTorrentItem::onTorrentChanged()
{
    // save file priorities and states
    this->resource()->setTorrentPreferredFilePriorities(m_torrent->preferredFilePriorities());

    // info.bytesTotal is > 0 for state 'downloading' only,
    // otherwise info.bytesTotal == 0, even when 'completed'.
    // After completion, we want to see the bytesTotal.
    IDownloadItem::State downloadItemState = IDownloadItem::Paused;

    if (m_torrent->info().error.type != TorrentError::NoError) {

        qInfo("Error ('%i'), file %i: '%s'.",
              m_torrent->info().error.type,
              m_torrent->info().error.fileIndex,
              m_torrent->info().error.message.toLatin1().data());

        QString message;

        switch (static_cast<int>(m_torrent->info().error.type)) {
        case TorrentError::NoError:
            Q_UNREACHABLE();
            break;

            /* Errors when adding the torrent to the queue */
        case TorrentError::MetadataDownloadError:
        case TorrentError::FailedToAddError:
        case TorrentError::NoInfoYetError:
            downloadItemState = IDownloadItem::NetworkError;
            break;

            /* Errors when downloading */
        case TorrentError::FileError:
        case TorrentError::SSLContextError:
        case TorrentError::FileMetadataError:
        case TorrentError::FileExceptionError:
        case TorrentError::PartFileError:
        case TorrentError::UnknownError:
            downloadItemState = IDownloadItem::FileError;
            break;

        default:
            Q_UNREACHABLE();
            break;
        }

        int fileIndex = m_torrent->info().error.fileIndex;
        QString filename;
        QList<TorrentFileMetaInfo> files = m_torrent->metaInfo().initialMetaInfo.files;
        if (fileIndex >= 0 && fileIndex < files.count()) {
            filename = files.at(fileIndex).shortFilePath();
        }
        switch (static_cast<int>(m_torrent->info().error.type)) {
        case TorrentError::NoError: Q_UNREACHABLE(); break;

            /* Errors when adding the torrent to the queue */
        case TorrentError::MetadataDownloadError: message = tr("Couldn't download metadata"); break;
        case TorrentError::FailedToAddError: message = tr("Couldn't download, bad .torrent format"); break;
        case TorrentError::NoInfoYetError: message = tr("Couldn't resolve metadata"); break;

            /* Errors when downloading */
        case TorrentError::FileError: message = tr("Error in file '%0'").arg(filename); break;
        case TorrentError::SSLContextError: message = tr("Bad SSL context"); break;
        case TorrentError::FileMetadataError: message = tr("Bad .torrent metadata"); break;
        case TorrentError::FileExceptionError: message = tr("Bad .torrent access permission"); break;
        case TorrentError::PartFileError: message = tr("Bad part-file"); break;

            /* Other */
        case TorrentError::UnknownError: message = tr("Unknown error"); break;
        default:
            Q_UNREACHABLE();
            break;
        }

        setErrorMessage(message);

        updateInfo(0, // or m_torrent->info().bytesReceived ?
                   m_torrent->metaInfo().initialMetaInfo.bytesTotal);
        //stop();

    } else {

        if (m_torrent->info().state == TorrentInfo::stopped) {
            qInfo("%s '%s' (%lli of %lli bytes).",
                  m_torrent->info().torrentState_c_str(),
                  m_torrent->metaInfo().initialMetaInfo.name.toLatin1().data(),
                  m_torrent->info().bytesTotal,
                  m_torrent->info().bytesReceived);
        } else {
            qInfo("%s '%s'.",
                  m_torrent->info().torrentState_c_str(),
                  m_torrent->metaInfo().initialMetaInfo.name.toLatin1().data());
        }

        switch (static_cast<int>(m_torrent->info().state)) {
        case TorrentInfo::stopped:
            downloadItemState = IDownloadItem::Paused;

            break;
        case TorrentInfo::checking_files:
            downloadItemState = IDownloadItem::Preparing;
            break;

        case TorrentInfo::downloading_metadata:
            downloadItemState = IDownloadItem::DownloadingMetadata;

            break;
        case TorrentInfo::downloading:
            downloadItemState = IDownloadItem::Downloading;
            updateInfo(m_torrent->info().bytesReceived, m_torrent->info().bytesTotal);

            break;
        case TorrentInfo::finished:
            downloadItemState = IDownloadItem::Completed;
            // here, info.bytesTotal == 0
            updateInfo(m_torrent->metaInfo().initialMetaInfo.bytesTotal,
                       m_torrent->metaInfo().initialMetaInfo.bytesTotal);

            break;
        case TorrentInfo::seeding:
            downloadItemState = IDownloadItem::Seeding;
            // here, info.bytesTotal == 0
            updateInfo(m_torrent->metaInfo().initialMetaInfo.bytesTotal,
                       m_torrent->metaInfo().initialMetaInfo.bytesTotal);

            break;
        case TorrentInfo::allocating:
        case TorrentInfo::checking_resume_data:
            downloadItemState = IDownloadItem::Endgame;

            break;
        default:
            Q_UNREACHABLE();
            break;
        }
    }

    setState(downloadItemState);
}

/******************************************************************************
 ******************************************************************************/
void DownloadTorrentItem::resume()
{
    if (isPreparing()) {
        return;
    }
    qInfo("Resume '%s' (destination: '%s').",
          resource()->url().toLatin1().data(), // remote/origine/t.torrent
          localFullFileName().toLatin1().data()); // localdrive/destination/t.torrent

    this->beginResume();

    /*
     * Contrary to other downloadItem types,
     * we don't check if the torrent already exists.
     */

    m_torrent->setLocalFullFileName(this->localFullFileName());
    m_torrent->setLocalFilePath(this->localFilePath());
    m_torrent->setUrl(resource()->url());
    m_torrent->setPreferredFilePriorities(this->resource()->torrentPreferredFilePriorities());

    if (!TorrentContext::getInstance().hasTorrent(m_torrent)) {
        bool ok = TorrentContext::getInstance().addTorrent(m_torrent);
        if (!ok) {
            stop();
            return;
        }

    }
    TorrentContext::getInstance().resumeTorrent(m_torrent);
    this->tearDownResume();
}

void DownloadTorrentItem::pause()
{
    qInfo("Pause '%s'.", resource()->url().toLatin1().data());
    if (isSeeding()) {
        if (TorrentContext::getInstance().hasTorrent(m_torrent)) {
            TorrentContext::getInstance().removeTorrent(m_torrent);
        }
        // Pausing a seeding item stops the seeding but keep the item completed.
        AbstractDownloadItem::preFinish(true);
        AbstractDownloadItem::finish();
        return;
    }
    if (isPreparing()) {
        TorrentContext::getInstance().stopPrepare(m_torrent);

    } else {
        if (TorrentContext::getInstance().hasTorrent(m_torrent)) {
            TorrentContext::getInstance().pauseTorrent(m_torrent);
        }
    }
    AbstractDownloadItem::pause();
}

void DownloadTorrentItem::stop()
{
    qInfo("Stop '%s'.", resource()->url().toLatin1().data());
    file()->cancel();

    if (isPreparing()) {
        TorrentContext::getInstance().stopPrepare(m_torrent);

    } else {
        if (TorrentContext::getInstance().hasTorrent(m_torrent)) {
            TorrentContext::getInstance().removeTorrent(m_torrent);
        }
    }
    AbstractDownloadItem::stop();
}

/******************************************************************************
 ******************************************************************************/
void DownloadTorrentItem::rename(const QString &/*newName*/)
{
    /// \todo bool success = TorrentContext::getInstance().rename(m_torrent, newName);
}

/******************************************************************************
 ******************************************************************************/
bool DownloadTorrentItem::isPreparing() const
{
    return state() == Preparing
            || state() == DownloadingMetadata;
}

bool DownloadTorrentItem::isSeeding() const
{
    return state() == Seeding;
}
