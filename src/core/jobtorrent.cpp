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

#include "jobtorrent.h"

#include <Core/File>
#include <Core/Format>
#include <Core/ResourceItem>
#include <Core/Torrent>
#include <Core/TorrentContext>


JobTorrent::JobTorrent(QObject *parent, ResourceItem *resource)
    : AbstractJob(parent, resource)
    , m_torrent(new Torrent(this))
{
    initWithResource(resource);

    connect(m_torrent, &Torrent::changed, this, &JobTorrent::onTorrentChanged);
}

JobTorrent::~JobTorrent()
{
    // delete m_torrent;
}

/*!
 * \reimp
 * Reimplement this method to detect when the URL of the resource is
 * available, so that TorrentContext can (down-)load the metadata as soon
 * as possible.
 */
void JobTorrent::initWithResource(ResourceItem *resource)
{
    if (!resource) {
        return;
    }
    /*
     * For torrent, mask is "*name*.*ext*" to not make extra sub-directories.
     * Subdirs are already managed by the torrent engine.
     */
    resource->setMask(QLatin1String("*name*.*ext*"));

    // JobFile::setResource(resource);

    m_torrent->setLocalFullFileName(this->localFullFileName());
    m_torrent->setLocalFilePath(this->localFilePath());
    m_torrent->setUrl(m_resource->url());

    QString fileStates = m_resource->torrentPreferredFilePriorities();
    // Download the metadata (the .torrent file) if not already downloaded
    TorrentContext::getInstance().prepareTorrent(m_torrent);

    // At this point, the torrent is loaded.

    // Restore the previous session's data.
    m_torrent->setPreferredFilePriorities(fileStates);
    //emit changed();
}

/******************************************************************************
 ******************************************************************************/
Torrent* JobTorrent::torrent() const
{
    return m_torrent;
}

/******************************************************************************
 ******************************************************************************/
void JobTorrent::onTorrentChanged()
{
    // save file priorities and states
    m_resource->setTorrentPreferredFilePriorities(m_torrent->preferredFilePriorities());

    // info.bytesTotal is > 0 for state 'downloading' only,
    // otherwise info.bytesTotal == 0, even when 'completed'.
    // After completion, we want to see the bytesTotal.
    AbstractJob::State downloadItemState = AbstractJob::Paused;

    if (m_torrent->info().error.type != TorrentError::NoError) {

        logInfo(QString("Error ('%0'), file %1: '%2'.").arg(
              QString::number(m_torrent->info().error.type),
              QString::number(m_torrent->info().error.fileIndex),
              m_torrent->info().error.message));

        QString message;

        switch (static_cast<int>(m_torrent->info().error.type)) {
        case TorrentError::NoError:
            Q_UNREACHABLE();
            break;

            /* Errors when adding the torrent to the queue */
        case TorrentError::MetadataDownloadError:
        case TorrentError::FailedToAddError:
        case TorrentError::NoInfoYetError:
            downloadItemState = AbstractJob::NetworkError;
            break;

            /* Errors when downloading */
        case TorrentError::FileError:
        case TorrentError::SSLContextError:
        case TorrentError::FileMetadataError:
        case TorrentError::FileExceptionError:
        case TorrentError::PartFileError:
        case TorrentError::UnknownError:
            downloadItemState = AbstractJob::FileError;
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

        switch (static_cast<int>(m_torrent->info().state)) {
        case TorrentInfo::stopped:
            downloadItemState = AbstractJob::Paused;

            break;
        case TorrentInfo::checking_files:
            downloadItemState = AbstractJob::Preparing;
            break;

        case TorrentInfo::downloading_metadata:
            downloadItemState = AbstractJob::DownloadingMetadata;

            break;
        case TorrentInfo::downloading:
            downloadItemState = AbstractJob::Downloading;
            updateInfo(m_torrent->info().bytesReceived, m_torrent->info().bytesTotal);

            break;
        case TorrentInfo::finished:
            downloadItemState = AbstractJob::Completed;
            // here, info.bytesTotal == 0
            updateInfo(m_torrent->metaInfo().initialMetaInfo.bytesTotal,
                       m_torrent->metaInfo().initialMetaInfo.bytesTotal);

            break;
        case TorrentInfo::seeding:
            downloadItemState = AbstractJob::Seeding;
            // here, info.bytesTotal == 0
            updateInfo(m_torrent->metaInfo().initialMetaInfo.bytesTotal,
                       m_torrent->metaInfo().initialMetaInfo.bytesTotal);

            break;
        case TorrentInfo::checking_resume_data:
            downloadItemState = AbstractJob::Endgame;

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
void JobTorrent::resume()
{
    if (isPreparing()) {
        return;
    }
    logInfo(QString("Resume '%0' (destination: '%1').")
            .arg(m_resource->url(), // remote/origine/t.torrent
                 localFullFileName())); // localdrive/destination/t.torrent

    this->beginResume();

    /*
     * Contrary to other downloadItem types,
     * we don't check if the torrent already exists.
     */

    m_torrent->setLocalFullFileName(localFullFileName());
    m_torrent->setLocalFilePath(localFilePath());
    m_torrent->setUrl(m_resource->url());
    m_torrent->setPreferredFilePriorities(m_resource->torrentPreferredFilePriorities());

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

void JobTorrent::pause()
{
    logInfo(QString("Pause '%0'.").arg(m_resource->url()));
    if (isSeeding()) {
        if (TorrentContext::getInstance().hasTorrent(m_torrent)) {
            TorrentContext::getInstance().removeTorrent(m_torrent);
        }
        // Pausing a seeding item stops the seeding but keep the item completed.
        AbstractJob::preFinish(true);
        AbstractJob::finish();
        return;
    }
    if (isPreparing()) {
        TorrentContext::getInstance().stopPrepare(m_torrent);

    } else {
        if (TorrentContext::getInstance().hasTorrent(m_torrent)) {
            TorrentContext::getInstance().pauseTorrent(m_torrent);
        }
    }
    AbstractJob::pause();
}

void JobTorrent::stop()
{
    // logInfo(QString("Stop '%0'.").arg(m_resource->url()));
    // m_file->cancel();

    if (isPreparing()) {
        TorrentContext::getInstance().stopPrepare(m_torrent);

    } else {
        if (TorrentContext::getInstance().hasTorrent(m_torrent)) {
            TorrentContext::getInstance().removeTorrent(m_torrent);
        }
    }
    AbstractJob::stop();
}

/******************************************************************************
 ******************************************************************************/
void JobTorrent::rename(const QString &/*newName*/)
{
    /// \todo bool success = TorrentContext::getInstance().rename(m_torrent, newName);
}

/******************************************************************************
 ******************************************************************************/
bool JobTorrent::isPreparing() const
{
    return state() == Preparing
            || state() == DownloadingMetadata;
}

bool JobTorrent::isSeeding() const
{
    return state() == Seeding;
}
