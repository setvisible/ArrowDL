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

#include "torrenthandler.h"

#include <Core/AbstractJob>
#include <Core/TorrentMessage>

#include <QtCore/QDebug>
#include <QtCore/QFile>


bool TorrentHandler::canRead() const
{
    return true;
}

bool TorrentHandler::canWrite() const
{
    return false;
}

bool TorrentHandler::read(IDownloadManager *downloadManager)
{
    if (!downloadManager) {
        qWarning("TorrentHandler::read() cannot read into null pointer");
        return false;
    }
    QIODevice *d = device();
    if (!d->isReadable()) {
        return false;
    }
    /*
     * Rem: The .torrent file is not read at this point.
     * The address is simply passed to the JobTorrent.
     * The JobTorrent is in charge of (down)loading the .torrent file
     * (eventually from magnet link), that contains metadata,
     * and finally download the data of the file itself.
     */
    QUrl url;
    auto f = dynamic_cast<QFile*>(d);
    if (f) {
        auto filename = f->fileName();
        url = QUrl(filename);
    }
    AbstractJob *job = downloadManager->createJobTorrent(url);
    if (!job) {
        qWarning("DownloadManager::createJobFile() not overridden."
                 " It still returns null pointer!");
        return false;
    }
    QList<AbstractJob*> jobs;
    jobs.append(job);
    downloadManager->append(jobs, false);
    return true;
}

bool TorrentHandler::write(const IDownloadManager &/*downloadManager*/)
{
    return false;
}
