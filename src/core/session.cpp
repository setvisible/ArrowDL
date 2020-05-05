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

#include "session.h"

#include <Core/DownloadItem>
#include <Core/DownloadManager>
#include <Core/DownloadStreamItem>
#include <Core/DownloadTorrentItem>
#include <Core/ResourceItem>

#include <QtCore/QDebug>
#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

static inline IDownloadItem::State intToState(int value)
{
    return static_cast<IDownloadItem::State>(value);
}

static inline int stateToInt(IDownloadItem::State state)
{
    /* Do not store error states and intermediary states. */
    switch (state) {
    case IDownloadItem::Stopped:
    case IDownloadItem::Completed:
        return static_cast<int>(state);
    default:
        return static_cast<int>(IDownloadItem::Paused);
    }
}

static inline QString encode64(const QByteArray &bytes)
{
    return QString::fromUtf8(bytes.toBase64());
}

static inline  QByteArray decode64(const QString &str)
{
    return QByteArray::fromBase64(str.toUtf8());
}

static inline DownloadItem* readJob(const QJsonObject &json, DownloadManager *downloadManager)
{
    ResourceItem *resourceItem = new ResourceItem();

    resourceItem->setUrl(json["url"].toString());
    resourceItem->setDestination(json["destination"].toString());
    resourceItem->setMask(json["mask"].toString());
    resourceItem->setCustomFileName(json["customFileName"].toString());
    resourceItem->setReferringPage(json["referringPage"].toString());
    resourceItem->setDescription(json["description"].toString());
    resourceItem->setCheckSum(json["checkSum"].toString());
    resourceItem->setStreamEnabled(json["streamEnabled"].toBool());
    resourceItem->setStreamFileName(json["streamFileName"].toString());
    resourceItem->setStreamFormatId(json["streamFormatId"].toString());
    resourceItem->setStreamFileSize(json["streamFileSize"].toInt());
    resourceItem->setTorrentEnabled(json["torrentEnabled"].toBool());
    resourceItem->setTorrentRawData(decode64(json["torrentRawData"].toString()));

    DownloadItem *item;
    if (resourceItem->isTorrentEnabled()) {
        item = new DownloadTorrentItem(downloadManager, resourceItem->torrentRawData());

    } else if (resourceItem->isStreamEnabled()) {
        item = new DownloadStreamItem(downloadManager);
    } else {
        item = new DownloadItem(downloadManager);
    }
    item->setResource(resourceItem);

    item->setState(intToState(json["state"].toInt()));
    item->setBytesReceived(json["bytesReceived"].toInt());
    item->setBytesTotal(json["bytesTotal"].toInt());
    item->setMaxConnectionSegments(json["maxConnectionSegments"].toInt());
    item->setMaxConnections(json["maxConnections"].toInt());

    return item;
}

static inline void writeJob(const DownloadItem *item, QJsonObject &json)
{
    json["url"] = item->resource()->url();
    json["destination"] = item->resource()->destination();
    json["mask"] = item->resource()->mask();
    json["customFileName"] = item->resource()->customFileName();
    json["referringPage"] = item->resource()->referringPage();
    json["description"] = item->resource()->description();
    json["checkSum"] = item->resource()->checkSum();
    json["streamEnabled"] = item->resource()->isStreamEnabled();
    json["streamFileName"] = item->resource()->streamFileName();
    json["streamFormatId"] = item->resource()->streamFormatId();
    json["streamFileSize"] = item->resource()->streamFileSize();
    json["torrentEnabled"] = item->resource()->isTorrentEnabled();
    json["torrentRawData"] = encode64(item->resource()->torrentRawData());

    json["state"] = stateToInt(item->state());
    json["bytesReceived"] = item->bytesReceived();
    json["bytesTotal"] = item->bytesTotal();
    json["maxConnectionSegments"] = item->maxConnectionSegments();
    json["maxConnections"] = item->maxConnections();
}

/******************************************************************************
 ******************************************************************************/
static inline void readList(QList<DownloadItem *> &downloadItems, const QJsonObject &json, DownloadManager *downloadManager)
{
    QJsonArray jobsArray = json["jobs"].toArray();
    for (int i = 0; i < jobsArray.size(); ++i) {
        QJsonObject jobObject = jobsArray[i].toObject();
        auto item = readJob(jobObject, downloadManager);
        downloadItems.append(item);
    }
}

static inline void writeList(const QList<DownloadItem *> &downloadItems, QJsonObject &json)
{
    QJsonArray jobsArray;
    foreach (auto item, downloadItems) {
        QJsonObject jobObject;
        writeJob(item, jobObject);
        jobsArray.append(jobObject);
    }
    json["jobs"] = jobsArray;
}


/******************************************************************************
 ******************************************************************************/
void Session::read(QList<DownloadItem *> &downloadItems, const QString &filename, DownloadManager *downloadManager)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Couldn't open file.");
        return;
    }
    QByteArray saveData = file.readAll();
    QJsonParseError ok = {};
    QJsonDocument loadDoc( QJsonDocument::fromJson(saveData, &ok) );

    if (ok.error != QJsonParseError::NoError) {
        qCritical("Couldn't parse JSON file.");
        return;
    }

    readList(downloadItems, loadDoc.object(), downloadManager);
}

void Session::write(const QList<DownloadItem *> &downloadItems, const QString &filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }

    QJsonObject json;
    writeList(downloadItems, json);
    QJsonDocument saveDoc(json);
    file.write( saveDoc.toJson() );
}
