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

#ifndef CORE_DOWNLOAD_FILE_ITEM_H
#define CORE_DOWNLOAD_FILE_ITEM_H

#include <Core/AbstractDownloadItem>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>

class File;
class DownloadManager;
class ResourceItem;

// class QNetworkReply;

class DownloadFileItem : public AbstractDownloadItem
{
    Q_OBJECT

public:
    DownloadFileItem(QObject *parent, ResourceItem *resource);
    ~DownloadFileItem() override;

    void resume() override;
    void pause() override;
    void stop() override;

private slots:
    void onMetaDataChanged();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onRedirected(const QUrl &url);
    void onFinished();
    void onErrorOccurred(QNetworkReply::NetworkError error);
    void onReadyRead();
    void onAboutToClose();

private:
    DownloadManager *m_downloadManager = nullptr;
    QNetworkReply *m_reply = nullptr;

    QString statusToHttp(QNetworkReply::NetworkError error);
};

#endif // CORE_DOWNLOAD_FILE_ITEM_H
