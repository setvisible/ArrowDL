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


#include <Core/JobClient>
#include <Core/ResourceItem>

#include <QtCore/QFile>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif


static void readJob(JobClient *job, const QJsonObject &json)
{
    job->resource()->setUrl(json["url"].toString());
    job->resource()->setDestination(json["destination"].toString());
    job->resource()->setMask(json["mask"].toString());
    job->resource()->setCustomFileName(json["customFileName"].toString());
    job->resource()->setReferringPage(json["referringPage"].toString());
    job->resource()->setDescription(json["description"].toString());
    job->resource()->setCheckSum(json["checkSum"].toString());


    job->setState((JobClient::State) json["state"].toInt());
    job->setBytesReceived(json["bytesReceived"].toInt());
    job->setBytesTotal(json["bytesTotal"].toInt());
    job->setError((QNetworkReply::NetworkError) json["error"].toInt());
    job->setMaxConnectionSegments(json["maxConnectionSegments"].toInt());
    job->setMaxConnections(json["maxConnections"].toInt());

}

static void writeJob(const JobClient *job, QJsonObject &json)
{
    json["url"] = job->resource()->url();
    json["destination"] = job->resource()->destination();
    json["mask"] = job->resource()->mask();
    json["customFileName"] = job->resource()->customFileName();
    json["referringPage"] = job->resource()->referringPage();
    json["description"] = job->resource()->description();
    json["checkSum"] = job->resource()->checkSum();


    json["state"] = (int) job->state();
    json["bytesReceived"] = job->bytesReceived();
    json["bytesTotal"] = job->bytesTotal();
    json["error"] = (int) job->error();
    json["maxConnectionSegments"] = job->maxConnectionSegments();
    json["maxConnections"] = job->maxConnections();

}


/******************************************************************************
 ******************************************************************************/
static void readList(QList<JobClient *> &jobs, const QJsonObject &json)
{
    QJsonArray jobsArray = json["jobs"].toArray();
    for (int i = 0; i < jobsArray.size(); ++i) {
        QJsonObject jobObject = jobsArray[i].toObject();
        JobClient *job = new JobClient(); // BUG parent ??
        job->setResource(new ResourceItem()); // weird
        readJob(job, jobObject);
        jobs.append(job);
    }
}

static void writeList(const QList<JobClient *> jobs, QJsonObject &json)
{
    QJsonArray jobsArray;
    foreach (auto job, jobs) {
        QJsonObject jobObject;
        writeJob(job, jobObject);
        jobsArray.append(jobObject);
    }
    json["jobs"] = jobsArray;
}


/******************************************************************************
 ******************************************************************************/
void Session::read(QList<JobClient *> &jobs, const QString &filename)
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


    readList(jobs, loadDoc.object());
}

void Session::write(const QList<JobClient *> jobs, const QString &filename)
{
    qDebug() << Q_FUNC_INFO << filename;
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }

    QJsonObject json;
    writeList(jobs, json);
    QJsonDocument saveDoc(json);
    file.write( saveDoc.toJson() );
}
