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

#include "jsonhandler.h"

#include <Core/AbstractJob>

#include <QtCore/QDebug>
#include <QtCore/QIODevice>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>


bool JsonHandler::canRead() const
{
    return true;
}

bool JsonHandler::canWrite() const
{
    return true;
}

bool JsonHandler::read(IScheduler *scheduler)
{
    if (!scheduler) {
        qWarning("JsonHandler::read() Can't read into null pointer");
        return false;
    }
    QIODevice *d = device();
    if (!d->isReadable()) {
        return false;
    }
    QByteArray saveData = d->readAll();
    QJsonParseError ok{};
    QJsonDocument loadDoc( QJsonDocument::fromJson(saveData, &ok) );

    if (ok.error != QJsonParseError::NoError) {
        qCritical("Couldn't parse JSON file.");
        return false;
    }

    QJsonObject json = loadDoc.object();

    QList<AbstractJob *> jobs;

    QJsonArray jobsArray = json["links"].toArray();
    for (int i = 0; i < jobsArray.size(); ++i) {
        QJsonObject jobObject = jobsArray[i].toObject();

        QUrl url = QUrl(jobObject["url"].toString());
        AbstractJob *job = scheduler->createJobFile(url);
        jobs.append(job);
    }

    scheduler->append(jobs, false);
    return true;
}

bool JsonHandler::write(const IScheduler &scheduler)
{
    QIODevice *d = device();
    QTextStream out(d);
    if (!d->isWritable()) {
        return false;
    }
    QJsonObject json;
    QJsonArray jobsArray;
    for (auto job : scheduler.jobs()) {
        QUrl url = job->sourceUrl();

        QJsonObject jobObject;
        jobObject["url"] = url.toString();
        jobsArray.append(jobObject);
    }
    json["links"] = jobsArray;

    QJsonDocument saveDoc(json);
    d->write( saveDoc.toJson() );

    return true;
}
