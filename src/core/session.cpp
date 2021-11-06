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
        return static_cast<int>(IDownloadItem::Stopped);

    case IDownloadItem::Completed:
    case IDownloadItem::Seeding:
        return static_cast<int>(IDownloadItem::Completed);

    default:
        return static_cast<int>(IDownloadItem::Paused);
    }
}


static inline DownloadItem* readJob(const QJsonObject &json, DownloadManager *downloadManager)
{
    auto resourceItem = new ResourceItem();

    resourceItem->setType(ResourceItem::fromString(json["type"].toString()));

    /// \deprecated since 2.0.8
    if (json.contains("streamEnabled")) {
        qWarning("Deprecated tag in session file: 'streamEnabled'. Use tag 'type' instead.");
        if (json["streamEnabled"].toBool()) {
            resourceItem->setType(ResourceItem::Type::Stream);
        }
    }
    /// \deprecated since 2.0.8
    if (json.contains("torrentEnabled")) {
        qWarning("Deprecated tag in session file: 'torrentEnabled'. Use tag 'type' instead.");
        if (json["torrentEnabled"].toBool()) {
            resourceItem->setType(ResourceItem::Type::Torrent);
        }
    }

    resourceItem->setUrl(json["url"].toString());
    resourceItem->setDestination(json["destination"].toString());
    resourceItem->setMask(json["mask"].toString());
    resourceItem->setCustomFileName(json["customFileName"].toString());
    resourceItem->setReferringPage(json["referringPage"].toString());
    resourceItem->setDescription(json["description"].toString());
    resourceItem->setCheckSum(json["checkSum"].toString());

    resourceItem->setStreamFileName(json["streamFileName"].toString());
    resourceItem->setStreamFormatId(json["streamFormatId"].toString());
    resourceItem->setStreamFileSize(json["streamFileSize"].toInt());

    resourceItem->setTorrentPreferredFilePriorities(json["torrentPreferredFilePriorities"].toString());

    DownloadItem *item;
    switch (resourceItem->type()) {
    case ResourceItem::Type::Stream:
        item = new DownloadStreamItem(downloadManager);
        break;
    case ResourceItem::Type::Torrent:
        item = new DownloadTorrentItem(downloadManager);
        break;
    default:
        item = new DownloadItem(downloadManager);
        break;
    }
    item->setResource(resourceItem);

    item->setState(intToState(json["state"].toInt()));
    item->setBytesReceived(json["bytesReceived"].toInt());
    item->setBytesTotal(json["bytesTotal"].toInt());
    item->setMaxConnectionSegments(json["maxConnectionSegments"].toInt());
    item->setMaxConnections(json["maxConnections"].toInt());
    item->setLog(json["log"].toString());

    return item;
}

static inline void writeJob(const DownloadItem *item, QJsonObject &json)
{
    json["type"] = ResourceItem::toString(item->resource()->type());
    json["url"] = item->resource()->url();
    json["destination"] = item->resource()->destination();
    json["mask"] = item->resource()->mask();
    json["customFileName"] = item->resource()->customFileName();
    json["referringPage"] = item->resource()->referringPage();
    json["description"] = item->resource()->description();
    json["checkSum"] = item->resource()->checkSum();

    json["streamFileName"] = item->resource()->streamFileName();
    json["streamFormatId"] = item->resource()->streamFormatId();
    json["streamFileSize"] = item->resource()->streamFileSize();

    json["torrentPreferredFilePriorities"] = item->resource()->torrentPreferredFilePriorities();

    json["state"] = stateToInt(item->state());
    json["bytesReceived"] = item->bytesReceived();
    json["bytesTotal"] = item->bytesTotal();
    json["maxConnectionSegments"] = item->maxConnectionSegments();
    json["maxConnections"] = item->maxConnections();
    json["log"] = item->log();
}

/******************************************************************************
 ******************************************************************************/
static inline void readList(QList<DownloadItem *> &downloadItems, const QJsonObject &json, DownloadManager *downloadManager)
{
    QJsonArray jobs = json["jobs"].toArray();
    foreach (auto job, jobs) {
        QJsonObject jobObject = job.toObject();
        auto item = readJob(jobObject, downloadManager);
        downloadItems.append(item);
    }
}

static inline void writeList(const QList<DownloadItem *> &downloadItems, QJsonObject &json)
{
    QJsonArray jobs;
    foreach (auto item, downloadItems) {
        QJsonObject jobObject;
        writeJob(item, jobObject);
        jobs.append(jobObject);
    }
    json["jobs"] = jobs;
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
