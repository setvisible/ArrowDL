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

#ifndef CORE_DOWNLOAD_ITEM_P_H
#define CORE_DOWNLOAD_ITEM_P_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTime>
#include <QtCore/QTimer>

#include "downloaditem.h"

class ResourceItem;
class QNetworkAccessManager;
class QNetworkReply;
class QSaveFile;

class DownloadItemPrivate
{    
public:
    DownloadItemPrivate(DownloadItem *qq);

    QNetworkAccessManager *networkManager;
    ResourceItem* resource;
    QNetworkReply* reply;
    QSaveFile* file;

    DownloadItem::State state;

    double speed;
    qint64 bytesReceived;
    qint64 bytesTotal;

    QNetworkReply::NetworkError error;

    int maxConnectionSegments;
    int maxConnections;

    QTime downloadTime;
    QTime remainingTime;
    QTimer updateInfoTimer;

    DownloadItem *q;
};

#endif // CORE_DOWNLOAD_ITEM_P_H
