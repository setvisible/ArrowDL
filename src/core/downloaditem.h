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

#ifndef CORE_DOWNLOAD_ITEM_H
#define CORE_DOWNLOAD_ITEM_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>

class ResourceItem;

class DownloadManager;
class DownloadItemPrivate;

class DownloadItem : public QObject
{
    Q_OBJECT

public:
    enum State {
        Idle,
        Paused,
        Stopped,
        Preparing,
        Connecting,
        Downloading,
        Endgame,
        Completed,
        Skipped,
        NetworkError,
        FileError};

    DownloadItem(DownloadManager *downloadManager);
    ~DownloadItem();

    /* Resource to download */
    ResourceItem* resource() const;
    void setResource(ResourceItem *resource);

    /* State */
    State state() const;
    void setState(const State state);


    double speed() const;
    qint64 bytesReceived() const;
    void setBytesReceived(qint64 bytesReceived);

    qint64 bytesTotal() const;
    void setBytesTotal(qint64 bytesTotal);

    int progress() const;

    QNetworkReply::NetworkError error() const;
    void setError(QNetworkReply::NetworkError error);

    /* Options */
    int maxConnectionSegments() const;
    void setMaxConnectionSegments(int connectionSegments);

    int maxConnections() const;
    void setMaxConnections(int connections);

    /* Convenient */
    QUrl sourceUrl() const;

    QString localFullFileName() const;
    QString localFileName() const;
    QString localFilePath() const;

    QUrl localFileUrl() const;
    QUrl localDirUrl() const;

    bool isResumable() const;
    bool isPausable() const;
    bool isCancelable() const;
    bool isDownloading() const;

    QTime remainingTime();

    static QString remaingTimeToString(QTime time);
    static QString currentSpeedToString(double speed);
    static QString fileSizeToString(qint64 size);

signals:
    void changed();
    void finished();

public slots:
    void resume();
    void pause();
    void stop();

private slots:
    void onMetaDataChanged();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onRedirected(const QUrl &url);
    void onFinished();
    void onError(QNetworkReply::NetworkError error);
    void onReadyRead();
    void onAboutToClose();

    void updateInfo();

private:
    DownloadItemPrivate *d;
    friend class DownloadItemPrivate;

};

#endif // CORE_DOWNLOAD_ITEM_H
