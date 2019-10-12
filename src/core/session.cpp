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
#include <Core/ResourceItem>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

static inline IDownloadItem::State intToState(int value)
{
    return (IDownloadItem::State)value;
}

static inline int stateToInt(IDownloadItem::State state)
{
    /* Do not store error states and intermediary states. */
    switch (state) {
    case IDownloadItem::Stopped:
    case IDownloadItem::Completed:
        return (int) state;
        break;
    default:
        return (int) IDownloadItem::Paused;
        break;
    }
}

static inline void readJob(DownloadItem *item, const QJsonObject &json)
{
    item->resource()->setUrl(json["url"].toString());
    item->resource()->setDestination(json["destination"].toString());
    item->resource()->setMask(json["mask"].toString());
    item->resource()->setCustomFileName(json["customFileName"].toString());
    item->resource()->setReferringPage(json["referringPage"].toString());
    item->resource()->setDescription(json["description"].toString());
    item->resource()->setCheckSum(json["checkSum"].toString());

    item->setState(intToState(json["state"].toInt()));
    item->setBytesReceived(json["bytesReceived"].toInt());
    item->setBytesTotal(json["bytesTotal"].toInt());
    item->setMaxConnectionSegments(json["maxConnectionSegments"].toInt());
    item->setMaxConnections(json["maxConnections"].toInt());

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
        DownloadItem *item = new DownloadItem(downloadManager); // BUG parent ??
        item->setResource(new ResourceItem()); // weird
        readJob(item, jobObject);
        downloadItems.append(item);
    }
}

static inline void writeList(const QList<DownloadItem *> downloadItems, QJsonObject &json)
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
    QJsonParseError ok;
    QJsonDocument loadDoc( QJsonDocument::fromJson(saveData, &ok) );

    if (ok.error != QJsonParseError::NoError) {
        qCritical("Couldn't parse JSON file.");
        return;
    }

    readList(downloadItems, loadDoc.object(), downloadManager);
}

void Session::write(const QList<DownloadItem *> downloadItems, const QString &filename)
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
