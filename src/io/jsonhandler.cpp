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

#include "jsonhandler.h"

#include <Core/IDownloadItem>

#include <QtCore/QDebug>
#include <QtCore/QDataStream>
//#include <QtCore/QIODevice>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>

JsonHandler::JsonHandler() : IFileHandler()
{
}

bool JsonHandler::canRead() const
{
    return true;
}

bool JsonHandler::canWrite() const
{
    return true;
}

bool JsonHandler::read(DownloadEngine *engine)
{
    if (!engine) {
        qWarning("JsonHandler::read() cannot read into null pointer");
        return false;
    }
    QIODevice *d = device();
    //    QDataStream in(d);
    if (!d->isReadable()) {
        return false;
    }
    QByteArray saveData = d->readAll(); // load data
    QJsonParseError ok;
    QJsonDocument loadDoc( QJsonDocument::fromJson(saveData, &ok) );

    if (ok.error != QJsonParseError::NoError) {
        qCritical("Couldn't parse JSON file.");
        //        QString msg = tr("Cannot parse the JSON file:\n"
        //                         "%1\n\n"
        //                         "At character %2, %3.\n\n"
        //                         "Operation canceled.")
        //                .arg(filename)
        //                .arg(ok.offset)
        //                .arg(ok.errorString());
        //        qCritical(msg);
        return false;
    }

    QJsonObject json = loadDoc.object();

    QList<IDownloadItem *> items;

    QJsonArray jobsArray = json["links"].toArray();
    for (int i = 0; i < jobsArray.size(); ++i) {
        QJsonObject jobObject = jobsArray[i].toObject();

        QUrl url = QUrl(jobObject["url"].toString());
        IDownloadItem *item =   engine->createItem(url);
        //        IDownloadItem *item = new DownloadItem(downloadManager); // BUG parent ??
        //        item->setResource(new ResourceItem()); // weird
        //        readJob(item, jobObject);
        items.append(item);
    }

    engine->append(items, false);



    //    QTextStream in(d);
    //    if (!d->isReadable()) {
    //        return false;
    //    }
    //    QString line;
    //    while (in.readLineInto(&line)) {
    //        const QUrl url(line);
    //        IDownloadItem *item = engine->createItem(url);
    //        if (item) {
    //            QList<IDownloadItem*> items;
    //            items.append(item);
    //            engine->append(items, false);
    //        } else {
    //            qWarning("DownloadEngine::createItem() not overridden. It still returns null pointer!");
    //        }
    //    }
    return true;
}

bool JsonHandler::write(const DownloadEngine &engine)
{
    QIODevice *d = device();
    QTextStream out(d);
    if (!d->isWritable()) {
        return false;
    }
    QJsonObject json;

    // m_downloadManager->write(json);
    QJsonArray jobsArray;
    foreach (auto item, engine.downloadItems()) {
        QUrl url = item->sourceUrl();

        QJsonObject jobObject;
        jobObject["url"] = url.toString();
        //        writeJob(item, jobObject);
        jobsArray.append(jobObject);
    }

    json["links"] = jobsArray;


    QJsonDocument saveDoc(json);
    d->write( saveDoc.toJson() );


    return true;
}
