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

#include "engine.h"

#include <Core/JobClient>
#include <Core/ResourceItem>
#include <Core/Session>
#include <Core/Settings>

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QtCore/QSaveFile>
#include <QtGui/QDesktopServices>

#include <QtNetwork/QtNetwork>

#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

Engine::Engine(QObject *parent) : QObject(parent)
  , m_networkManager(new QNetworkAccessManager(this))
  , m_currentSimultaneousDownloads(0)
  , m_maxSimultaneousDownloads(4)
{
    connect(this, SIGNAL(jobAppended(JobClient*)), this, SLOT(onQueueChanged(JobClient*)));
    connect(this, SIGNAL(jobRemoved(JobClient*)), this, SLOT(onQueueChanged(JobClient*)));
    connect(this, SIGNAL(jobStateChanged(JobClient*)), this, SLOT(onQueueChanged(JobClient*)));
}

Engine::~Engine()
{
    clear();
}

/******************************************************************************
 ******************************************************************************/
void Engine::clear()
{
    foreach (auto job, m_jobs) {
        remove(job);
    }
}

/******************************************************************************
 ******************************************************************************/
QList<JobClient*> Engine::jobs() const
{
    return m_jobs;
}

JobClient* Engine::jobAt(int index) const
{
    Q_ASSERT(index >=0 && index < m_jobs.count());
    return m_jobs.at(index);
}

/******************************************************************************
 ******************************************************************************/
void Engine::append(JobClient *job, const bool started)
{
    if (started) {
        if (job->isResumable()) {
            job->setState(JobClient::Idle);
        }
    } else {
        if (job->isPausable()) {
            job->setState(JobClient::Paused);
        }
    }
    m_jobs << job;
    emit jobAppended(job);
}

void Engine::remove(JobClient *job)
{
    cancel(job); // stop the reply first
    m_jobs.removeAll(job);
    emit jobRemoved(job);
}

/******************************************************************************
 ******************************************************************************/
void Engine::onQueueChanged(JobClient */*job*/)
{
    if (m_currentSimultaneousDownloads < m_maxSimultaneousDownloads) {

        foreach (auto job, m_jobs) {
            if (job->state() == JobClient::Idle) {
                m_currentSimultaneousDownloads++;
                start(job);
                break;
            }
        }
    }
}

inline void Engine::start(JobClient *job)
{
    qDebug() << Q_FUNC_INFO << job->localFullFileName();

    /* Ensure the destination directory exists */
    job->setState(JobClient::Preparing);
    emit jobStateChanged(job);
    {
        QDir().mkpath(job->localFilePath());

        QSaveFile *file = new QSaveFile(job);
        file->setFileName(job->localFullFileName());

        if (!file->open(QIODevice::WriteOnly)) { // QIODevice::Append ?
            job->setState(JobClient::FileError);
            emit jobStateChanged(job);
            return;
        }

        addFile(file, job);
    }

    /* Prepare the connection, try to contact the server */
    job->setState(JobClient::Connecting);
    emit jobStateChanged(job);


    {
        QNetworkRequest request;
        request.setUrl(job->sourceUrl());

        QNetworkReply *reply = m_networkManager->get(request);
        addReply(reply, job);

        /* Signals/Slots of QNetworkReply */
        connect(reply, SIGNAL(metaDataChanged()),
                this, SLOT(onMetaDataChanged()));
        connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                this, SLOT(onDownloadProgress(qint64,qint64)));
        connect(reply, SIGNAL(redirected(QUrl)),
                this, SLOT(onRedirected(QUrl)));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(onError(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(finished()),
                this, SLOT(onFinished()));

        /* Signals/Slots of QIODevice */
        connect(reply, SIGNAL(readyRead()),
                this, SLOT(onReadyRead()));  // Des qu'un morceau est lisable (pas tout le fichier)
        connect(reply, SIGNAL(aboutToClose()),
                this, SLOT(onAboutToClose()));
    }

    /* The job starts to download */
    job->setState(JobClient::Downloading);
    emit jobStateChanged(job);
}

/******************************************************************************
 ******************************************************************************/
void Engine::resume(JobClient *job)
{
    if (job->isResumable()) {
        job->setState(JobClient::Idle);
        emit jobStateChanged(job);
    }
}

void Engine::pause(JobClient *job)
{
    if (job->isPausable()) {
        job->setState(JobClient::Paused);
        QNetworkReply *reply = getReply(job);
        if (reply) {
            reply->abort(); // emits finished() signal
        }
        emit jobStateChanged(job);
    }
}

void Engine::cancel(JobClient *job)
{
    if (job->isCancelable()) {
        job->setState(JobClient::Stopped);
        job->setBytesReceived(0);
        job->setBytesTotal(0);

        QSaveFile *file = getFile(job);
        if (file) {
            file->cancelWriting();
        }
        QNetworkReply *reply = getReply(job);
        if (reply) {
            reply->abort(); // emits finished() signal
        }
        emit jobStateChanged(job);
    }
}

/******************************************************************************
 ******************************************************************************/
void Engine::onMetaDataChanged()
{
    qDebug() << Q_FUNC_INFO;
}

void Engine::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug() << Q_FUNC_INFO << bytesReceived << bytesTotal;
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    JobClient *job = getJob(reply);
    job->setBytesReceived(bytesReceived);
    job->setBytesTotal(bytesTotal);
    emit jobStateChanged(job);
}

void Engine::onRedirected(const QUrl &url)
{
    qDebug() << Q_FUNC_INFO << url;
}

void Engine::onFinished()
{
    qDebug() << Q_FUNC_INFO ;
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    JobClient *job = getJob(reply);
    QSaveFile *file = getFile(job);
    if (job->state() == JobClient::Downloading) {
        job->setState(JobClient::Endgame);
        if (file->commit()) {
            job->setState(JobClient::Completed);
        } else {
            job->setState(JobClient::FileError);
        }
    }

    removeReply(reply);
    reply->deleteLater();

    removeFile(file);
    file->deleteLater();

    m_currentSimultaneousDownloads--;
    emit jobStateChanged(job);
    emit downloadFinished(true);
}

void Engine::onError(QNetworkReply::NetworkError error)
{
    qDebug() << Q_FUNC_INFO << error;
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    JobClient *job = getJob(reply);
    job->setError(error);
    job->setState(JobClient::NetworkError);
    emit jobStateChanged(job);
    emit downloadFinished(false);
}

void Engine::onReadyRead()
{
    qDebug() << Q_FUNC_INFO;
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    qint64 bytes = reply->bytesAvailable();
    QByteArray data = reply->readAll();

    JobClient *job = getJob(reply);
    QSaveFile *file = getFile(job);



    qint64 writtenBytes = file->write(data, bytes);
    if (writtenBytes != bytes) {
        qDebug() << Q_FUNC_INFO << "ERROR" << writtenBytes << bytes;
    }

}

void Engine::onAboutToClose()
{
    qDebug() << Q_FUNC_INFO ;
}

/******************************************************************************
 ******************************************************************************/
void Engine::addReply(QNetworkReply* item, JobClient *job)
{
    m_map.insert(item, job);
}

void Engine::removeReply(QNetworkReply* item)
{
    m_map.remove(item);
}

QNetworkReply* Engine::getReply(JobClient *job)
{
    return m_map.key(job, 0);
}

JobClient* Engine::getJob(QNetworkReply* item)
{
    return m_map.value(item, 0);
}

/******************************************************************************
 ******************************************************************************/
void Engine::addFile(QSaveFile* file, JobClient *job)
{
    m_fileMap.insert(file, job);
}

void Engine::removeFile(QSaveFile* file)
{
    m_fileMap.remove(file);
}

QSaveFile* Engine::getFile(JobClient *job)
{
    return m_fileMap.key(job, 0);
}

JobClient* Engine::getJob(QSaveFile* file)
{
    return m_fileMap.value(file, 0);
}
