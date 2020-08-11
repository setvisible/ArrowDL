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

#ifndef CORE_DOWNLOAD_ITEM_H
#define CORE_DOWNLOAD_ITEM_H

#include <Core/AbstractDownloadItem>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>

class File;
class DownloadItemPrivate;
class DownloadManager;
class ResourceItem;

class DownloadItem : public AbstractDownloadItem
{
    Q_OBJECT

public:
    DownloadItem(DownloadManager *downloadManager);
    ~DownloadItem() Q_DECL_OVERRIDE;

    /* Resource to download */
    ResourceItem* resource() const;
    virtual void setResource(ResourceItem *resource);

    /* Convenient */
    QUrl sourceUrl() const Q_DECL_OVERRIDE;
    QString localFileName() const Q_DECL_OVERRIDE;
    QString localFullFileName() const Q_DECL_OVERRIDE;
    QString localFilePath() const Q_DECL_OVERRIDE;
    QUrl localFileUrl() const Q_DECL_OVERRIDE;
    QUrl localDirUrl() const Q_DECL_OVERRIDE;

    void resume() Q_DECL_OVERRIDE;
    void pause() Q_DECL_OVERRIDE;
    void stop() Q_DECL_OVERRIDE;

    void rename(const QString &newName) Q_DECL_OVERRIDE;

private slots:
    void onMetaDataChanged();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onRedirected(const QUrl &url);
    void onFinished();
    void onError(QNetworkReply::NetworkError error);
    void onReadyRead();
    void onAboutToClose();

protected:
    File* file() const;

private:
    DownloadItemPrivate *d;
    friend class DownloadItemPrivate;

    QString statusToHttp(QNetworkReply::NetworkError error);
};

#endif // CORE_DOWNLOAD_ITEM_H
