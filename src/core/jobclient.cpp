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

#include "jobclient_p.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

JobClientPrivate::JobClientPrivate(JobClient *qq)
    : q(qq)
{
    resource = Q_NULLPTR;

    state = JobClient::State::Idle;
    bytesReceived = 0;
    bytesTotal = 0;

    error = QNetworkReply::NoError;

    maxConnectionSegments = 4;
    maxConnections = 1;
}

/******************************************************************************
 ******************************************************************************/
JobClient::JobClient(QObject *parent) : QObject(parent)
  , d(new JobClientPrivate(this))
{
}

JobClient::~JobClient()
{
}

/******************************************************************************
 ******************************************************************************/
ResourceItem* JobClient::resource() const
{
    return d->resource;
}

void JobClient::setResource(ResourceItem *resource)
{
    d->resource = resource;
}

/******************************************************************************
 ******************************************************************************/
JobClient::State JobClient::state() const
{
    return d->state;
}

void JobClient::setState(const State status)
{
    d->state = status;
}
/******************************************************************************
 ******************************************************************************/
qint64 JobClient::bytesReceived() const
{
    return d->bytesReceived;
}

void JobClient::setBytesReceived(qint64 bytesReceived)
{
    d->bytesReceived = bytesReceived;
}

qint64 JobClient::bytesTotal() const
{
    return d->bytesTotal;
}

void JobClient::setBytesTotal(qint64 bytesTotal)
{
    d->bytesTotal = bytesTotal;
}

int JobClient::progress() const
{
    if (d->bytesTotal > 0) {
        return (int)((qreal)d->bytesReceived / (qreal)d->bytesTotal * 100.0);
    } else {
        return 0;
    }
}

/******************************************************************************
 ******************************************************************************/
QNetworkReply::NetworkError JobClient::error() const
{
    return d->error;
}

void JobClient::setError(QNetworkReply::NetworkError error)
{
    d->error = error;
}

/******************************************************************************
 ******************************************************************************/
int JobClient::maxConnectionSegments() const
{
    return d->maxConnectionSegments;
}

void JobClient::setMaxConnectionSegments(int connectionSegments)
{
    if (connectionSegments > 0 && connectionSegments <= 10) {
        d->maxConnectionSegments = connectionSegments;
    }
}

/******************************************************************************
 ******************************************************************************/
int JobClient::maxConnections() const
{
    return d->maxConnections;
}

void JobClient::setMaxConnections(int connections)
{
    d->maxConnections = connections;
}

/******************************************************************************
 ******************************************************************************/
/**
 * The source Url
 */
QUrl JobClient::sourceUrl() const
{
    return d->resource->url();
}

/**
 * The destination's full file name
 */
QString JobClient::localFullFileName() const
{
    const QUrl target = d->resource->localFileUrl();
    return target.toLocalFile();
}

/**
 * The destination's file name
 */
QString JobClient::localFileName() const
{
    const QUrl target = d->resource->localFileUrl();
    const QFileInfo fi(target.toLocalFile());
    return fi.fileName();
}

/**
 * The destination's absolute path
 */
QString JobClient::localFilePath() const
{
    const QUrl target = d->resource->localFileUrl();
    const QFileInfo fi(target.toLocalFile());
    return fi.absolutePath();
}

QUrl JobClient::localFileUrl() const
{
    return d->resource->localFileUrl();
}

QUrl JobClient::localDirUrl() const
{
    return QUrl::fromLocalFile(localFilePath());
}

/******************************************************************************
 ******************************************************************************/
bool JobClient::isResumable() const
{
    return d->state == JobClient::Idle ||
            d->state == JobClient::Paused ||
            d->state == JobClient::Stopped ||
            d->state == JobClient::Skipped ||
            d->state == JobClient::NetworkError ||
            d->state == JobClient::FileError;
}

bool JobClient::isPausable() const
{
    return d->state == JobClient::Idle ||
            d->state == JobClient::Preparing ||
            d->state == JobClient::Connecting ||
            d->state == JobClient::Downloading ||
            d->state == JobClient::Endgame;
}

bool JobClient::isCancelable() const
{
    return d->state == JobClient::Idle ||
            d->state == JobClient::Paused ||
            d->state == JobClient::Preparing ||
            d->state == JobClient::Connecting ||
            d->state == JobClient::Downloading ||
            d->state == JobClient::Endgame ||
            d->state == JobClient::Completed;
}
