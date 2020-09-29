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

#include "dummytorrentanimator.h"

#include "dummytorrentfactory.h"

#include <Core/Torrent>

#include <QtCore/QDebug>
#include <QtCore/QtMath>

#define TICK_MSEC 10


DummyTorrentAnimator::DummyTorrentAnimator(QObject *parent)
    : QObject(parent)
{
    qsrand(QDateTime::currentDateTime().toTime_t());
}

/******************************************************************************
 ******************************************************************************/
Torrent *DummyTorrentAnimator::torrent() const
{
    return m_torrent;
}

void DummyTorrentAnimator::setTorrent(Torrent *torrent)
{
    m_torrent = torrent;
}

/******************************************************************************
 ******************************************************************************/
/**
 * @brief DummyTorrentAnimator::setProgress
 * @param torrent Torrent
 * @param percent Progress between 0 (stopped) and 100 (completed)
 */
void DummyTorrentAnimator::setProgress(int percent)
{
    Q_ASSERT(m_torrent);

    const TorrentMetaInfo metaInfo = m_torrent->metaInfo();
    TorrentInfo info = m_torrent->info();

    auto pieceCount = metaInfo.initialMetaInfo.pieceCount;
    auto pieceByteSize = metaInfo.initialMetaInfo.pieceByteSize;

    Q_ASSERT(pieceByteSize > 0);

    qint64 bytesReceived = 0;

    // First, create a random piece map
    info.downloadedPieces = createRandomBitArray(static_cast<int>(pieceCount), percent);

    TorrentHandleInfo detail = m_torrent->detail();
    int total = metaInfo.initialMetaInfo.files.count();
    for (int i = 0; i < total; ++i) {
        auto fileMetaInfo = metaInfo.initialMetaInfo.files.at(i);

        qint64 bytesOffset = fileMetaInfo.bytesOffset;
        qint64 bytesTotal = fileMetaInfo.bytesTotal;

        auto firstPieceIndex = qCeil(qreal(bytesOffset) / pieceByteSize);
        auto lastPieceIndex = qCeil(qreal(bytesOffset + bytesTotal) / pieceByteSize);
        auto filePieceCount = 1 + lastPieceIndex - firstPieceIndex;
        filePieceCount = qMin(filePieceCount, pieceCount);

        // Count pieces for each file
        qint64 received = 0;
        for (int j = 0; j < filePieceCount; ++j) {
            if (info.downloadedPieces.testBit(j)) {
                received++;
            }
        }
        received *= pieceByteSize;
        received = qMin(received, bytesTotal);

        detail.files[i].bytesReceived = received;
        bytesReceived += received;
    }

    bytesReceived = qMin(bytesReceived, info.bytesTotal);

    info.bytesReceived = bytesReceived;
    info.bytesSessionDownloaded = bytesReceived;
    info.bytesSessionUploaded = bytesReceived >> 2;

    if (percent <= 0) {
        info.state = TorrentInfo::stopped;
    } else if (percent >= 1) {
        info.state = TorrentInfo::seeding;
    } else {
        info.state = TorrentInfo::downloading;
    }

    m_torrent->setInfo(info, false);
    m_torrent->setDetail(detail, false); // emit changed
}

/******************************************************************************
 ******************************************************************************/
QBitArray DummyTorrentAnimator::createRandomBitArray(int size, int percent)
{
    if (percent <= 0) {
        return QBitArray(size, false);
    }
    if (percent >= 100) {
        return QBitArray(size, true);
    }
    QBitArray ba = QBitArray(size, false);
    if (percent == 50) {
        for (int i = 0; i < size; i+=2) {
            ba.setBit(i);
        }
    } else {
        qsrand(QDateTime::currentDateTime().toTime_t());
        for (int i = 0; i < size; ++i) {
            int v = int((100 * qrand()) / RAND_MAX);
            if (v <= percent) {
                ba.setBit(i);
            }
        }
    }
    return ba;
}

/******************************************************************************
 ******************************************************************************/
bool DummyTorrentAnimator::isStarted() const
{
    return m_timer != Q_NULLPTR;
}


void DummyTorrentAnimator::start()
{
    if (!m_timer) {
        randomize();
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&DummyTorrentAnimator::animate));
        m_timer->start(TICK_MSEC);
        emit started(true);
    }
}

void DummyTorrentAnimator::stop()
{
    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = Q_NULLPTR;
        emit started(false);
    }
}

/******************************************************************************
 ******************************************************************************/
void DummyTorrentAnimator::animate()
{
    if (m_ticks.count() != m_timeouts.count()) {
        m_ticks = m_timeouts;
    }
    for (int i = 0, total = m_ticks.count(); i < total; ++i) {
        m_ticks[i]--;
        if (m_ticks[i] <= 0) {
            m_ticks[i] = m_timeouts[i];
            animateFile(i);
        }
    }
}

void DummyTorrentAnimator::animateFile(int index)
{
    Q_ASSERT(m_torrent);

    // qDebug() << Q_FUNC_INFO << index;
    const qint64 receivedPieceSize = 32*1024*8;

    TorrentMetaInfo metaInfo = m_torrent->metaInfo();
    auto bytesTotal = metaInfo.initialMetaInfo.files.at(index).bytesTotal;

    TorrentHandleInfo detail = m_torrent->detail();
    detail.files[index].bytesReceived += receivedPieceSize;
    detail.files[index].bytesReceived = qMin(detail.files[index].bytesReceived, bytesTotal);

    TorrentInfo info = m_torrent->info();
    info.bytesReceived += receivedPieceSize;
    info.bytesReceived = qMin(info.bytesReceived, info.bytesTotal);
    info.bytesSessionDownloaded += receivedPieceSize;
    info.bytesSessionUploaded += receivedPieceSize >> 2;
    info.state = TorrentInfo::downloading;

    m_torrent->setInfo(info, false);
    m_torrent->setDetail(detail, false); // emit changed
}

/******************************************************************************
 ******************************************************************************/
void DummyTorrentAnimator::randomize()
{
    Q_ASSERT(m_torrent);
    int total = m_torrent->metaInfo().initialMetaInfo.files.count();
    m_timeouts.clear();
    m_ticks.clear();
    for (int i = 0; i < total; ++i) {
        int timeout = TICK_MSEC * (((9 * qrand()) / RAND_MAX) + 1);
        m_timeouts << timeout;
    }
}
