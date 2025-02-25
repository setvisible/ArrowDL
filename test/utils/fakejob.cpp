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

#include "fakejob.h"

#include <Core/ResourceItem>

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>

FakeJob::FakeJob(QObject *parent, ResourceItem *resource)
    : AbstractJob(parent, resource)
    , m_resourceUrl(QUrl("https://www.example.com/myfolder/myimage.png"))
    , m_resourceLocalFileName(QString("myimage.png"))
    , m_simulationBytesTotal(123 * 1024 * 1024) // We 'download' 123 MB of data
    , m_simulationTimeIncrement(100)            // We receives data chunk every 100 milliseconds
    , m_simulationTimeDuration(5000)            // We 'download' in 5 seconds
    , m_isSimulateFileErrorEnabled(false)
    , m_isSimulateFileErrorAtTheEndEnabled(false)
{
    init();
}

FakeJob::FakeJob(QString localFileName, QObject *parent)
    : AbstractJob(parent, new ResourceItem())
    , m_resourceUrl(QUrl("https://www.example.com/myfolder/myimage.png"))
    , m_resourceLocalFileName(localFileName)
    , m_simulationBytesTotal(123 * 1024 * 1024) // We 'download' 123 MB of data
    , m_simulationTimeIncrement(100)            // We receives data chunk every 100 milliseconds
    , m_simulationTimeDuration(5000)            // We 'download' in 5 seconds
    , m_isSimulateFileErrorEnabled(false)
    , m_isSimulateFileErrorAtTheEndEnabled(false)
{
    init();
}

FakeJob::FakeJob(QUrl url,
                 QString filename,
                 qsizetype bytesTotal,
                 qint64 timeIncrement,
                 qint64 duration,
                 QObject *parent)
    : AbstractJob(parent, new ResourceItem())
    , m_resourceUrl(url)
    , m_resourceLocalFileName(filename)
    , m_simulationBytesTotal(bytesTotal)
    , m_simulationTimeIncrement(timeIncrement)
    , m_simulationTimeDuration(duration)
    , m_isSimulateFileErrorEnabled(false)
    , m_isSimulateFileErrorAtTheEndEnabled(false)
{
    init();
}

/******************************************************************************
 ******************************************************************************/
void FakeJob::init()
{
    connect(&m_fakeStreamTimer, SIGNAL(timeout()), this, SLOT(tickFakeStream()));
}

/******************************************************************************
 ******************************************************************************/
void FakeJob::resume()
{
    this->beginResume();

    if (this->checkResume(!m_isSimulateFileErrorEnabled)) {

        /* Prepare the connection, try to contact the server */
        m_fakeStreamTimer.start(static_cast<int>(m_simulationTimeIncrement)); // milliseconds

        this->tearDownResume();
    }
}

void FakeJob::pause()
{
    // TO DO
    // https://kunalmaemo.blogspot.com/2011/07/simple-download-manager-with-pause.html

    AbstractJob::pause();
}

void FakeJob::stop()
{
    m_fakeStreamTimer.stop(); // or delete ?
    AbstractJob::stop();
}

/******************************************************************************
 ******************************************************************************/
void FakeJob::tickFakeStream()
{
    qsizetype received = bytesReceived();
    if (received < m_simulationBytesTotal) {
        const qint64 incrementCount = static_cast<qint64>(qreal(m_simulationTimeDuration) / m_simulationTimeIncrement);
        qsizetype incrementReceived = static_cast<qsizetype>(qreal(m_simulationBytesTotal) / incrementCount);
        received = qMin(received + incrementReceived, m_simulationBytesTotal);
        updateInfo(received, m_simulationBytesTotal);

    } else {
        preFinish(!m_isSimulateFileErrorAtTheEndEnabled);
        m_fakeStreamTimer.stop();
        finish();
    }
}

void FakeJob::simulateNetworkError()
{
    m_simulateHttpErrorNumber = 404;
    setState(NetworkError);
    finish();
}

/******************************************************************************
 ******************************************************************************/
/**
 * The source Url
 */
QUrl FakeJob::sourceUrl() const
{
    return m_resourceUrl;
}

void FakeJob::setSourceUrl(const QUrl &resourceUrl)
{
    m_resourceUrl = resourceUrl;
}

/**
 * The destination's full file name
 */
QString FakeJob::localFullFileName() const
{
    return m_resourceUrl.toLocalFile();
}

/**
 * The destination's file name
 */
QString FakeJob::localFileName() const
{
    return m_resourceLocalFileName;
}

/**
 * The destination's absolute path
 */
QString FakeJob::localFilePath() const
{
    const QFileInfo fi(m_resourceUrl.toLocalFile());
    return fi.absolutePath();
}

QUrl FakeJob::localFileUrl() const
{
    return m_resourceUrl;
}

QUrl FakeJob::localDirUrl() const
{
    return m_resourceUrl;
}
