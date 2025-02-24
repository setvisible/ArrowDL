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

#include "session.h"

#include <Core/AbstractDownloadItem>
#include <Core/DownloadManager>
#include <Core/JobFile>
#include <Core/JobStream>
#include <Core/JobTorrent>
#include <Core/ResourceItem>

#include <QtCore/QDebug>
#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

static inline AbstractDownloadItem::State intToState(int value)
{
    return static_cast<AbstractDownloadItem::State>(value);
}

static inline int stateToInt(AbstractDownloadItem::State state)
{
    /* Do not store error states and intermediary states. */
    switch (state) {
    case AbstractDownloadItem::Stopped:
        return static_cast<int>(AbstractDownloadItem::Stopped);

    case AbstractDownloadItem::Completed:
    case AbstractDownloadItem::Seeding:
        return static_cast<int>(AbstractDownloadItem::Completed);

    default:
        return static_cast<int>(AbstractDownloadItem::Paused);
    }
}

static inline StreamObject::Config readStreamConfig(const QJsonObject &json)
{
    StreamObject::Config config;
    {
        auto j = json["overview"].toObject();
        config.overview.skipVideo = j["skipVideo"].toBool();
        config.overview.markWatched = j["markWatched"].toBool();
    }
    {
        auto j = json["subtitle"].toObject();
        config.subtitle.writeSubtitle = j["writeSubtitle"].toBool();
        config.subtitle.isAutoGenerated = j["isAutoGenerated"].toBool();
        config.subtitle.extensions = j["extensions"].toString();
        config.subtitle.languages = j["languages"].toString();
        config.subtitle.convert = j["convert"].toString();
    }
    {
        auto j = json["chapter"].toObject();
        config.chapter.writeChapters = j["writeChapters"].toBool();
    }
    {
        auto j = json["thumbnail"].toObject();
        config.thumbnail.writeDefaultThumbnail = j["writeDefaultThumbnail"].toBool();
    }
    {
        auto j = json["comment"].toObject();
        config.comment.writeComment = j["writeComment"].toBool();
    }
    {
        auto j = json["metadata"].toObject();
        config.metadata.writeDescription = j["writeDescription"].toBool();
        config.metadata.writeMetadata = j["writeMetadata"].toBool();
        config.metadata.writeInternetShortcut = j["writeInternetShortcut"].toBool();
    }
    {
        auto j = json["processing"].toObject();
    }
    {
        auto j = json["sponsor"].toObject();
    }
    return config;
}

static inline QJsonObject writeStreamConfig(const StreamObject::Config &config)
{
    QJsonObject json;
    {
        QJsonObject j;
        j["skipVideo"] = config.overview.skipVideo;
        j["markWatched"] = config.overview.markWatched;
        json["overview"] = j;
    }
    {
        QJsonObject j;
        j["writeSubtitle"] = config.subtitle.writeSubtitle;
        j["isAutoGenerated"] = config.subtitle.isAutoGenerated;
        j["extensions"] = config.subtitle.extensions;
        j["languages"] = config.subtitle.languages;
        j["convert"] = config.subtitle.convert;
        json["subtitle"] = j;
    }
    {
        QJsonObject j;
        j["writeChapters"] = config.chapter.writeChapters;
        json["chapter"] = j;
    }
    {
        QJsonObject j;
        j["writeDefaultThumbnail"] = config.thumbnail.writeDefaultThumbnail;
        json["thumbnail"] = j;
    }
    {
        QJsonObject j;
        j["writeComment"] = config.comment.writeComment;
        json["comment"] = j;
    }
    {
        QJsonObject j;
        j["writeDescription"] = config.metadata.writeDescription;
        j["writeMetadata"] = config.metadata.writeMetadata;
        j["writeInternetShortcut"] = config.metadata.writeInternetShortcut;
        json["metadata"] = j;
    }
    return json;
}

static inline AbstractDownloadItem* readJob(const QJsonObject &json, DownloadManager *downloadManager)
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
    resourceItem->setStreamFileSize(static_cast<qsizetype>(json["streamFileSize"].toInteger()));

    auto config = readStreamConfig(json["streamConfig"].toObject());
    resourceItem->setStreamConfig(config);

    resourceItem->setTorrentPreferredFilePriorities(json["torrentPreferredFilePriorities"].toString());

    AbstractDownloadItem *item;
    switch (resourceItem->type()) {
    case ResourceItem::Type::Stream:
        item = new JobStream(downloadManager, resourceItem);
        break;
    case ResourceItem::Type::Torrent:
        item = new JobTorrent(downloadManager, resourceItem);
        break;
    default:
        item = new JobFile(downloadManager, resourceItem);
        break;
    }
    item->setState(intToState(json["state"].toInt()));
    item->setBytesReceived(static_cast<qsizetype>(json["bytesReceived"].toInteger()));
    item->setBytesTotal(static_cast<qsizetype>(json["bytesTotal"].toInteger()));
    item->setMaxConnections(json["maxConnections"].toInt());
    item->setLog(json["log"].toString());

    return item;
}

static inline void writeJob(const AbstractDownloadItem *item, QJsonObject &json)
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
    json["streamFileSize"] = static_cast<qsizetype>(item->resource()->streamFileSize());

    auto config = item->resource()->streamConfig();
    json["streamConfig"] = writeStreamConfig(config);

    json["torrentPreferredFilePriorities"] = item->resource()->torrentPreferredFilePriorities();

    json["state"] = stateToInt(item->state());
    json["bytesReceived"] = static_cast<qsizetype>(item->bytesReceived());
    json["bytesTotal"] = static_cast<qsizetype>(item->bytesTotal());
    json["maxConnections"] = item->maxConnections();
    json["log"] = item->log();
}

/******************************************************************************
 ******************************************************************************/
static inline void readList(QList<AbstractDownloadItem *> &downloadItems, const QJsonObject &json, DownloadManager *downloadManager)
{
    QJsonArray jobs = json["jobs"].toArray();
    for (auto job : jobs) {
        QJsonObject jobObject = job.toObject();
        auto item = readJob(jobObject, downloadManager);
        downloadItems.append(item);
    }
}

static inline void writeList(const QList<AbstractDownloadItem *> &downloadItems, QJsonObject &json)
{
    QJsonArray jobs;
    for (auto item : downloadItems) {
        QJsonObject jobObject;
        writeJob(item, jobObject);
        jobs.append(jobObject);
    }
    json["jobs"] = jobs;
}


/******************************************************************************
 ******************************************************************************/
void Session::read(QList<AbstractDownloadItem *> &downloadItems, const QString &filename, DownloadManager *downloadManager)
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

void Session::write(const QList<AbstractDownloadItem *> &downloadItems, const QString &filename)
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
