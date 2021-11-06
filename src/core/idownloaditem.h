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

#ifndef CORE_I_DOWNLOAD_ITEM_H
#define CORE_I_DOWNLOAD_ITEM_H

#include <QtCore/QString>
#include <QtCore/QUrl>

class IDownloadItem
{
public:
    enum State {
        Idle,
        Paused,
        Stopped,
        Preparing,
        Connecting,
        DownloadingMetadata,
        Downloading,
        Endgame,
        Completed,
        Seeding,
        Skipped,
        NetworkError,
        FileError
    };

    IDownloadItem() = default;
    virtual ~IDownloadItem() noexcept = default; /* Pure virtual interface */

    virtual State state() const = 0;

    virtual qint64 bytesReceived() const = 0; /*!< in bytes */
    virtual qint64 bytesTotal() const = 0; /*!< in bytes */

    virtual double speed() const = 0; /*!< Returns the speed in byte per second */
    virtual int progress() const = 0; /*!< Return a value between 0 and 100, or -1 if undefined */

    virtual int maxConnectionSegments() const = 0;
    virtual int maxConnections() const = 0;
    virtual QString log() const = 0;

    virtual QUrl sourceUrl() const = 0;
    virtual QString localFullFileName() const = 0;
    virtual QString localFileName() const = 0;
    virtual QString localFilePath() const = 0;
    virtual QUrl localFileUrl() const = 0;
    virtual QUrl localDirUrl() const = 0;

    virtual bool isResumable() const = 0;
    virtual bool isPausable() const = 0;
    virtual bool isCancelable() const = 0;
    virtual bool isDownloading() const = 0;

    virtual void setReadyToResume() = 0;
    virtual void resume() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;

};

#endif // CORE_I_DOWNLOAD_ITEM_H
