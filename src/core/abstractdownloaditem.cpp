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

#include "abstractdownloaditem.h"

#include <Constants>

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QtMath>

/*!
 * \class AbstractDownloadItem
 *
 * The class AbstractDownloadItem implements the most common methods of
 * IDownloadItem and the Signal/Slot mechanism.
 *
 */


/*!
 * \brief Constructor
 */
AbstractDownloadItem::AbstractDownloadItem(QObject *parent) : QObject(parent)
{
    connect(&m_updateInfoTimer, SIGNAL(timeout()), this, SLOT(updateInfo()));
    connect(&m_updateCountDownTimer, SIGNAL(timeout()), this, SLOT(updateInfo()));
}

/******************************************************************************
 ******************************************************************************/
IDownloadItem::State AbstractDownloadItem::state() const
{
    return m_state;
}

void AbstractDownloadItem::setState(State state)
{
    if (m_state != state) {
        m_state = state;
        emit changed();
    }
}

QString AbstractDownloadItem::stateToString() const
{
    switch (m_state) {
    case IDownloadItem::Idle:                return tr("Idle");
    case IDownloadItem::Paused:              return tr("Paused");
    case IDownloadItem::Stopped:             return tr("Canceled");
    case IDownloadItem::Preparing:           return tr("Preparing");
    case IDownloadItem::Connecting:          return tr("Connecting");
    case IDownloadItem::DownloadingMetadata: return tr("Downloading Metadata");
    case IDownloadItem::Downloading:         return tr("Downloading");
    case IDownloadItem::Endgame:             return tr("Finishing");
    case IDownloadItem::Completed:           return tr("Complete");
    case IDownloadItem::Seeding:             return tr("Seeding");
    case IDownloadItem::Skipped:             return tr("Skipped");
    case IDownloadItem::NetworkError:        return tr("Server error");
    case IDownloadItem::FileError:           return tr("File error");
    }
    Q_UNREACHABLE();
}

/*! C string for printf() */
const char* AbstractDownloadItem::state_c_str() const
{
    switch (m_state) {
    case IDownloadItem::Idle:                return QLatin1String("idle").data();
    case IDownloadItem::Paused:              return QLatin1String("paused").data();
    case IDownloadItem::Stopped:             return QLatin1String("canceled").data();
    case IDownloadItem::Preparing:           return QLatin1String("preparing").data();
    case IDownloadItem::Connecting:          return QLatin1String("connecting").data();
    case IDownloadItem::DownloadingMetadata: return QLatin1String("downloading metadata").data();
    case IDownloadItem::Downloading:         return QLatin1String("downloading").data();
    case IDownloadItem::Endgame:             return QLatin1String("finishing").data();
    case IDownloadItem::Completed:           return QLatin1String("complete").data();
    case IDownloadItem::Seeding:             return QLatin1String("seeding").data();
    case IDownloadItem::Skipped:             return QLatin1String("skipped").data();
    case IDownloadItem::NetworkError:        return QLatin1String("server error").data();
    case IDownloadItem::FileError:           return QLatin1String("file error").data();
    }
    Q_UNREACHABLE();
}

/******************************************************************************
 ******************************************************************************/
qsizetype AbstractDownloadItem::bytesReceived() const
{
    return m_bytesReceived;
}

void AbstractDownloadItem::setBytesReceived(qsizetype bytesReceived)
{
    m_bytesReceived = bytesReceived;
}

/******************************************************************************
 ******************************************************************************/
qsizetype AbstractDownloadItem::bytesTotal() const
{
    return m_bytesTotal;
}

void AbstractDownloadItem::setBytesTotal(qsizetype bytesTotal)
{
    m_bytesTotal = bytesTotal;
}

/******************************************************************************
 ******************************************************************************/
qreal AbstractDownloadItem::speed() const
{
    return m_state == Downloading ? m_speed : -1;
}

int AbstractDownloadItem::progress() const
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
QString AbstractDownloadItem::errorMessage() const
{
    return m_errorMessage;
}

void AbstractDownloadItem::setErrorMessage(const QString &message)
{
    m_errorMessage = message;
}

/******************************************************************************
 ******************************************************************************/
int AbstractDownloadItem::maxConnectionSegments() const
{
    return m_maxConnectionSegments;
}

void AbstractDownloadItem::setMaxConnectionSegments(int connectionSegments)
{
    if (connectionSegments > 0 && connectionSegments <= MAX_CONNECTION_SEGMENTS) {
        m_maxConnectionSegments = connectionSegments;
    }
}

/******************************************************************************
 ******************************************************************************/
int AbstractDownloadItem::maxConnections() const
{
    return m_maxConnections;
}

void AbstractDownloadItem::setMaxConnections(int connections)
{
    m_maxConnections = connections;
}

/******************************************************************************
 ******************************************************************************/
QString AbstractDownloadItem::log() const
{
    return m_log;
}

void AbstractDownloadItem::setLog(const QString &log)
{
    m_log = log;
}

/*!
 * Apped the given message to the log.
 */
void AbstractDownloadItem::logInfo(const QString &message)
{
    QDateTime local(QDateTime::currentDateTime());
    auto timestamp = local.toString(QLatin1String("yyyy-MM-dd HH:mm:ss.zzz"));
    m_log.append("[" + timestamp + "] " + message + "\n");
    qInfo() << message;
}

/******************************************************************************
 ******************************************************************************/
bool AbstractDownloadItem::isResumable() const
{
    return m_state == Idle
            || m_state == Paused
            || m_state == Stopped
            || m_state == Skipped
            || m_state == NetworkError
            || m_state == FileError;
}

bool AbstractDownloadItem::isPausable() const
{
    return m_state == Idle
            || isDownloading()
            || m_state == Seeding;
}

bool AbstractDownloadItem::isCancelable() const
{
    return m_state == Idle
            || m_state == Paused
            || isDownloading()
            || m_state == Completed
            || m_state == Seeding;
}

bool AbstractDownloadItem::isDownloading() const
{
    return m_state == Preparing
            || m_state == Connecting
            || m_state == DownloadingMetadata
            || m_state == Downloading
            || m_state == Endgame;
}

/******************************************************************************
 ******************************************************************************/
QTime AbstractDownloadItem::remainingTime()
{
    return m_remainingTime;
}

/******************************************************************************
 ******************************************************************************/
void AbstractDownloadItem::setReadyToResume()
{
    m_state = Idle;
    emit changed();
}

void AbstractDownloadItem::pause()
{
    // TO DO
    // https://kunalmaemo.blogspot.com/2011/07/simple-download-manager-with-pause.html

    stop();

    m_state = Paused;
    emit changed();
}

// cancel?
void AbstractDownloadItem::stop()
{
    m_state = Stopped;
    m_speed = -1;
    m_bytesReceived = 0;
    m_bytesTotal = 0;

    emit changed();
    finish();
}

/******************************************************************************
 ******************************************************************************/
void AbstractDownloadItem::beginResume()
{
    m_state = Idle;
    emit changed();

    m_downloadElapsedTimer.start();

    /* Ensure the destination directory exists */
    m_state = Preparing;
    emit changed();
}

bool AbstractDownloadItem::checkResume(bool connected)
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

void AbstractDownloadItem::tearDownResume()
{
    /*
     * This timer ticks each second, in order to update the remaining time information (countdown)
     */
    m_updateCountDownTimer.start(TIMEOUT_COUNT_DOWN);

    /*
     * This timer updates the speed/progress info.
     * It can be quicker than the countdown timer.
     */
    m_updateInfoTimer.start(TIMEOUT_INFO);

    /* Start downloading now. */
    m_state = Downloading;
    emit changed();
}

void AbstractDownloadItem::preFinish(bool commited)
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
void AbstractDownloadItem::finish()
{
    m_updateCountDownTimer.stop();
    m_updateInfoTimer.stop();
    emit finished();
}

/******************************************************************************
 ******************************************************************************/
void AbstractDownloadItem::rename(const QString &newName)
{
    Q_UNUSED(newName)
}

/******************************************************************************
 ******************************************************************************/
void AbstractDownloadItem::updateInfo(qsizetype bytesReceived, qsizetype bytesTotal)
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

void AbstractDownloadItem::updateInfo()
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
