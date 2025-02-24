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

#include "abstractjob.h"

#include <Constants>
#include <Core/File>
#include <Core/ResourceItem>

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QtMath>
#include <QtCore/QTimer>


/*!
 * \brief Constructor
 */
AbstractJob::AbstractJob(QObject *parent, ResourceItem *resource)
    : QObject(parent)
    , m_resource(resource)
    , m_file(new File(this))
    , m_updateInfoTimer(new QTimer(this))
    , m_updateCountDownTimer(new QTimer(this))
{
    connect(m_updateInfoTimer, SIGNAL(timeout()), this, SLOT(updateInfo()));
    connect(m_updateCountDownTimer, SIGNAL(timeout()), this, SLOT(updateInfo()));
}

AbstractJob::~AbstractJob()
{
    // delete m_resource;
    // if (m_file) {
    //     m_file->deleteLater();
    //     m_file = nullptr;
    // }
}

/******************************************************************************
 ******************************************************************************/
AbstractJob::State AbstractJob::state() const
{
    return m_state;
}

void AbstractJob::setState(State state)
{
    if (m_state != state) {
        m_state = state;
        emit changed();
    }
}

QString AbstractJob::stateToString() const
{
    switch (m_state) {
    case AbstractJob::Idle:                return tr("Idle");
    case AbstractJob::Paused:              return tr("Paused");
    case AbstractJob::Stopped:             return tr("Canceled");
    case AbstractJob::Preparing:           return tr("Preparing");
    case AbstractJob::Connecting:          return tr("Connecting");
    case AbstractJob::DownloadingMetadata: return tr("Downloading Metadata");
    case AbstractJob::Downloading:         return tr("Downloading");
    case AbstractJob::Endgame:             return tr("Finishing");
    case AbstractJob::Completed:           return tr("Complete");
    case AbstractJob::Seeding:             return tr("Seeding");
    case AbstractJob::Skipped:             return tr("Skipped");
    case AbstractJob::NetworkError:        return tr("Server error");
    case AbstractJob::FileError:           return tr("File error");
    }
    Q_UNREACHABLE();
}

/*! C string for printf() */
const char* AbstractJob::state_c_str() const
{
    switch (m_state) {
    case AbstractJob::Idle:                return QLatin1String("idle").data();
    case AbstractJob::Paused:              return QLatin1String("paused").data();
    case AbstractJob::Stopped:             return QLatin1String("canceled").data();
    case AbstractJob::Preparing:           return QLatin1String("preparing").data();
    case AbstractJob::Connecting:          return QLatin1String("connecting").data();
    case AbstractJob::DownloadingMetadata: return QLatin1String("downloading metadata").data();
    case AbstractJob::Downloading:         return QLatin1String("downloading").data();
    case AbstractJob::Endgame:             return QLatin1String("finishing").data();
    case AbstractJob::Completed:           return QLatin1String("complete").data();
    case AbstractJob::Seeding:             return QLatin1String("seeding").data();
    case AbstractJob::Skipped:             return QLatin1String("skipped").data();
    case AbstractJob::NetworkError:        return QLatin1String("server error").data();
    case AbstractJob::FileError:           return QLatin1String("file error").data();
    }
    Q_UNREACHABLE();
}

/******************************************************************************
 ******************************************************************************/
qsizetype AbstractJob::bytesReceived() const
{
    return m_bytesReceived;
}

void AbstractJob::setBytesReceived(qsizetype bytesReceived)
{
    m_bytesReceived = bytesReceived;
}

/******************************************************************************
 ******************************************************************************/
qsizetype AbstractJob::bytesTotal() const
{
    return m_bytesTotal;
}

void AbstractJob::setBytesTotal(qsizetype bytesTotal)
{
    m_bytesTotal = bytesTotal;
}

/******************************************************************************
 ******************************************************************************/
qreal AbstractJob::speed() const
{
    return m_state == Downloading ? m_speed : -1;
}

int AbstractJob::progress() const
{
    if (m_bytesTotal > 0) {
        return qBound(0, qFloor(100 * static_cast<qreal>(m_bytesReceived) / static_cast<qreal>(m_bytesTotal)), 100);
    }
    if (m_state == Idle) {
        return 0;
    }
    if (m_state == Stopped ||
            m_state == Skipped ||
            m_state == NetworkError ||
            m_state == FileError) {
        return 100;
    }
    return -1; // Undefined
}

/******************************************************************************
 ******************************************************************************/
QString AbstractJob::errorMessage() const
{
    return m_errorMessage;
}

void AbstractJob::setErrorMessage(const QString &message)
{
    m_errorMessage = message;
}

/******************************************************************************
 ******************************************************************************/
int AbstractJob::maxConnections() const
{
    return m_maxConnections;
}

void AbstractJob::setMaxConnections(int connections)
{
    m_maxConnections = connections;
}

/******************************************************************************
 ******************************************************************************/
QString AbstractJob::log() const
{
    return m_log;
}

void AbstractJob::setLog(const QString &log)
{
    m_log = log;
}

/*!
 * Apped the given message to the log.
 */
void AbstractJob::logInfo(const QString &message)
{
    QDateTime local(QDateTime::currentDateTime());
    auto timestamp = local.toString(QLatin1String("yyyy-MM-dd HH:mm:ss.zzz"));
    m_log.append("[" + timestamp + "] " + message + "\n");
    qInfo() << message;
}

/******************************************************************************
 ******************************************************************************/
/**
 * The source Url
 */
QUrl AbstractJob::sourceUrl() const
{
    return QUrl(m_resource->url());
}

void AbstractJob::setSourceUrl(const QUrl &url)
{
    Q_UNUSED(url);
}

/**
 * The destination's full file name
 */
QString AbstractJob::localFullFileName() const
{
    auto target = m_resource->localFileUrl();
    return target.toLocalFile();
}

/**
 * The destination's file name
 */
QString AbstractJob::localFileName() const
{
    auto target = m_resource->localFileUrl();
    const QFileInfo fi(target.toLocalFile());
    return fi.fileName();
}

/**
 * The destination's absolute path
 */
QString AbstractJob::localFilePath() const
{
    auto target = m_resource->localFileUrl();
    const QFileInfo fi(target.toLocalFile());
    return fi.absolutePath();
}

QUrl AbstractJob::localFileUrl() const
{
    return m_resource->localFileUrl();
}

QUrl AbstractJob::localDirUrl() const
{
    return QUrl::fromLocalFile(localFilePath());
}

/******************************************************************************
 ******************************************************************************/
bool AbstractJob::isResumable() const
{
    return m_state == Idle
            || m_state == Paused
            || m_state == Stopped
            || m_state == Skipped
            || m_state == NetworkError
            || m_state == FileError;
}

bool AbstractJob::isPausable() const
{
    return m_state == Idle
            || isDownloading()
            || m_state == Seeding;
}

bool AbstractJob::isCancelable() const
{
    return m_state == Idle
            || m_state == Paused
            || isDownloading()
            || m_state == Completed
            || m_state == Seeding;
}

bool AbstractJob::isDownloading() const
{
    return m_state == Preparing
            || m_state == Connecting
            || m_state == DownloadingMetadata
            || m_state == Downloading
            || m_state == Endgame;
}

/******************************************************************************
 ******************************************************************************/
QTime AbstractJob::remainingTime() const
{
    return m_remainingTime;
}

/******************************************************************************
 ******************************************************************************/
void AbstractJob::setReadyToResume()
{
    m_state = Idle;
    emit changed();
}

void AbstractJob::resume()
{
}

void AbstractJob::pause()
{
    /// \todo implement?
    /// https://kunalmaemo.blogspot.com/2011/07/simple-download-manager-with-pause.html
    logInfo(QString("Pause '%0'.").arg(m_resource->url()));

    stop();

    m_state = Paused;
    emit changed();
}

// cancel?
void AbstractJob::stop()
{
    logInfo(QString("Stop '%0'.").arg(m_resource->url()));
    m_file->cancel();

    m_state = Stopped;
    m_speed = -1;
    m_bytesReceived = 0;
    m_bytesTotal = 0;

    emit changed();
    finish();
}


/******************************************************************************
 ******************************************************************************/
void AbstractJob::beginResume()
{
    m_state = Idle;
    emit changed();

    m_downloadElapsedTimer.start();

    /* Ensure the destination directory exists */
    m_state = Preparing;
    emit changed();
}

bool AbstractJob::checkResume(bool connected)
{
    if (connected) {
        m_state = Connecting;
        emit changed();
    } else {
        m_state = FileError;
        emit changed();
        finish();
    }
    return connected;
}

void AbstractJob::tearDownResume()
{
    /*
     * This timer ticks each second, in order to update the remaining time information (countdown)
     */
    m_updateCountDownTimer->start(TIMEOUT_COUNT_DOWN);

    /*
     * This timer updates the speed/progress info.
     * It can be quicker than the countdown timer.
     */
    m_updateInfoTimer->start(TIMEOUT_INFO);

    /* Start downloading now. */
    m_state = Downloading;
    emit changed();
}

void AbstractJob::preFinish(bool commited)
{
    m_state = Endgame;
    emit changed();

    if (commited) {
        m_state = Completed;
    } else {
        m_state = FileError;
    }
    emit changed();
}

/******************************************************************************
 ******************************************************************************/
void AbstractJob::finish()
{
    m_updateCountDownTimer->stop();
    m_updateInfoTimer->stop();
    emit changed();
    emit finished();
}

/******************************************************************************
 ******************************************************************************/
void AbstractJob::rename(const QString &newName)
{
    QString newCustomFileName;
    if (!newName.trimmed().isEmpty()) {
        newCustomFileName = newName;
    }
    auto oldPath = m_resource->localFileFullPath(m_resource->customFileName());
    auto newPath = m_resource->localFileFullPath(newCustomFileName);

    auto oldFileName = m_resource->fileName();

    if (oldPath == newPath) {
        return;
    }
    bool success = true;
    if (QFile::exists(newPath)) {
        success = false; /* File error */
    }
    if (QFile::exists(oldPath) && !QFile::rename(oldPath, newPath)) {
        success = false; /* File error */
    }
    if (success) {
        m_resource->setCustomFileName(newCustomFileName);
        if (m_file->isOpen()) {
            m_file->rename(m_resource);
        }
    }
    auto newFileName = success ? m_resource->fileName() : newName;
    emit renamed(oldFileName, newFileName, success);
    emit changed();
}

void AbstractJob::moveToTrash()
{
    stop();
    auto fileName = localFullFileName();
    if (!QFile::exists(fileName)) {
        return;
    }
    if (!QFile::moveToTrash(fileName)) {
        /// \todo if not moved, do something else, like rename 'myfile' to '~myfile'?
    }
    emit changed();
}

/******************************************************************************
 ******************************************************************************/
ResourceItem* AbstractJob::resource() const
{
    return m_resource;
}

// void AbstractJob::setResource(ResourceItem *resource)
// {
//     m_resource = resource;
// }

/******************************************************************************
 ******************************************************************************/
void AbstractJob::updateInfo(qsizetype bytesReceived, qsizetype bytesTotal)
{
    m_bytesReceived = bytesReceived;
    m_bytesTotal = bytesTotal;
    auto elapsed = m_downloadElapsedTimer.elapsed();
    if (elapsed > 0) {
        m_speed = 1000 * static_cast<qreal>(bytesReceived) / static_cast<qreal>(m_downloadElapsedTimer.elapsed());
    } else {
        m_speed = qreal(-1);
    }
    /*
     * It's very tempting to add 'emit changed();' here, but don't do that.
     *
     * Indeed, 'emit changed()' informs the GUI of a change, i.e. every 150 msec.
     *
     * But updateInfo(int, int) is called more often by the download engine,
     * typically every time a chunk of data is downloaded.
     */
}

void AbstractJob::updateInfo()
{
    if (m_speed > 0 && m_bytesReceived > 0 && m_bytesTotal > 0) {
        auto estimatedTime = qCeil(static_cast<qreal>(m_bytesTotal - m_bytesReceived) / m_speed);
        QTime time(0, 0, 0);
        time = time.addSecs(static_cast<int>(estimatedTime));
        m_remainingTime = time;
    } else {
        m_remainingTime = {};
    }
    emit changed();
}
