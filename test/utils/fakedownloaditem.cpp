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

#include "fakedownloaditem.h"

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>


FakeDownloadItem::FakeDownloadItem(QObject *parent) : AbstractDownloadItem(parent)
  , m_resourceUrl(QUrl("https://www.example.com/myfolder/myimage.png"))
  , m_resourceLocalFileName(QString("myimage.png"))
  , m_simulationBytesTotal(123*1024*1024) // We 'download' 123 MB of data
  , m_simulationTimeIncrement(100) // We receives data chunk every 100 milliseconds
  , m_simulationTimeDuration(5000) // We 'download' in 5 seconds
  , m_isSimulateFileErrorEnabled(false)
  , m_isSimulateFileErrorAtTheEndEnabled(false)
{
    init();
}

FakeDownloadItem::FakeDownloadItem(QString localFileName, QObject *parent)
    : AbstractDownloadItem(parent)
    , m_resourceUrl(QUrl("https://www.example.com/myfolder/myimage.png"))
    , m_resourceLocalFileName(localFileName)
    , m_simulationBytesTotal(123*1024*1024) // We 'download' 123 MB of data
    , m_simulationTimeIncrement(100) // We receives data chunk every 100 milliseconds
    , m_simulationTimeDuration(5000) // We 'download' in 5 seconds
    , m_isSimulateFileErrorEnabled(false)
    , m_isSimulateFileErrorAtTheEndEnabled(false)
{
    init();
}

FakeDownloadItem::FakeDownloadItem(QUrl url, QString filename,
                                   qint64 bytesTotal,
                                   qint64 timeIncrement,
                                   qint64 duration,
                                   QObject *parent) : AbstractDownloadItem(parent)
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

FakeDownloadItem::~FakeDownloadItem()
{
}

/******************************************************************************
 ******************************************************************************/
void FakeDownloadItem::init()
{
    connect(&m_fakeStreamTimer, SIGNAL(timeout()), this, SLOT(tickFakeStream()));
}

/******************************************************************************
 ******************************************************************************/
void FakeDownloadItem::resume()
{
    this->beginResume();

    if (this->checkResume(!m_isSimulateFileErrorEnabled)) {

        /* Prepare the connection, try to contact the server */
        m_fakeStreamTimer.start(static_cast<int>(m_simulationTimeIncrement)); // milliseconds

        this->tearDownResume();
    }
}

void FakeDownloadItem::pause()
{
    // TO DO
    // https://kunalmaemo.blogspot.com/2011/07/simple-download-manager-with-pause.html

    AbstractDownloadItem::pause();
}

void FakeDownloadItem::stop()
{
    m_fakeStreamTimer.stop(); // or delete ?
    AbstractDownloadItem::stop();
}

/******************************************************************************
 ******************************************************************************/
void FakeDownloadItem::tickFakeStream()
{
    qint64 received = bytesReceived();
    if (received < m_simulationBytesTotal) {
        const qint64 incrementCount = m_simulationTimeDuration / m_simulationTimeIncrement;
        received += m_simulationBytesTotal / incrementCount;

        updateInfo(qMin(m_simulationBytesTotal, received), m_simulationBytesTotal);

    } else {
        preFinish(!m_isSimulateFileErrorAtTheEndEnabled);
        m_fakeStreamTimer.stop();
        finish();
    }
}

void FakeDownloadItem::simulateNetworkError()
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
QUrl FakeDownloadItem::sourceUrl() const
{
    return m_resourceUrl;
}

void FakeDownloadItem::setSourceUrl(const QUrl &resourceUrl)
{
    m_resourceUrl = resourceUrl;
}

/**
 * The destination's full file name
 */
QString FakeDownloadItem::localFullFileName() const
{
    return m_resourceUrl.toLocalFile();
}

/**
 * The destination's file name
 */
QString FakeDownloadItem::localFileName() const
{
    //    const QFileInfo fi(m_resourceUrl.toLocalFile());
    //    return fi.fileName();
    return m_resourceLocalFileName;
}

/**
 * The destination's absolute path
 */
QString FakeDownloadItem::localFilePath() const
{
    const QFileInfo fi(m_resourceUrl.toLocalFile());
    return fi.absolutePath();
    //    return fi.absolutePath();
}

QUrl FakeDownloadItem::localFileUrl() const
{
    return m_resourceUrl;
}

QUrl FakeDownloadItem::localDirUrl() const
{
    return m_resourceUrl;
    //    return QUrl::fromLocalFile(m_resourceUrl);
}
