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

#include <Core/AbstractDownloadItem>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>

class ResourceItem;

class DownloadManager;
class DownloadItemPrivate;

class DownloadItem : public AbstractDownloadItem
{
    Q_OBJECT

public:
    DownloadItem(DownloadManager *downloadManager);
    ~DownloadItem();

    /* Resource to download */
    ResourceItem* resource() const;
    void setResource(ResourceItem *resource);

    /* Convenient */
    virtual QUrl sourceUrl() const Q_DECL_OVERRIDE;
    virtual QString localFileName() const Q_DECL_OVERRIDE;
    QString localFullFileName() const;
    QString localFilePath() const;
    QUrl localFileUrl() const;
    QUrl localDirUrl() const;

    virtual void resume() Q_DECL_OVERRIDE;
    virtual void pause() Q_DECL_OVERRIDE;
    virtual void stop() Q_DECL_OVERRIDE;

    virtual void rename(const QString &newName) Q_DECL_OVERRIDE;

private slots:
    void onMetaDataChanged();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onRedirected(const QUrl &url);
    void onFinished();
    void onError(QNetworkReply::NetworkError error);
    void onReadyRead();
    void onAboutToClose();

private:
    DownloadItemPrivate *d;
    friend class DownloadItemPrivate;
};

#endif // CORE_DOWNLOAD_ITEM_H
