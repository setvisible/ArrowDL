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

#include "dummytorrentfactory.h"

#include <Core/Torrent>

#include <QtCore/QtMath>


static const qint64 KILOBYTES = 1024;
static const qint64 BLOCK_SIZE_BYTES = 16 * KILOBYTES;


static QBitArray createRandomBitArray(qint64 size, int percent);

class BasicTorrent
{
public:
    BasicTorrent(QString _name, qint64 _piece_size_in_KB = 32)
        : name(_name), piece_size_in_KB(_piece_size_in_KB)
    {
        qsrand(QDateTime::currentDateTime().toTime_t());
    }

    void addFile(qint64 size, QString name)
    {
        basicFiles << BasicFile(name, size);
    }

    TorrentPtr toTorrent(QObject *parent);

private:
    QString name;
    qint64 piece_size_in_KB;

    struct BasicFile
    {
        BasicFile(QString _name, qint64 _size_in_KB)
            : name(_name), size_in_KB(_size_in_KB) {}

        QString name;
        qint64 size_in_KB;

    };
    QList<BasicFile> basicFiles;

    TorrentPeerInfo makePeer(const EndPoint &endpoint, const QString &userAgent, qint64 size);
};

/******************************************************************************
 ******************************************************************************/
TorrentPeerInfo BasicTorrent::makePeer(const EndPoint &endpoint, const QString &userAgent, qint64 size)
{
    int percent = int((100 * qrand()) / RAND_MAX);

    TorrentPeerInfo peer;
    peer.endpoint = endpoint;
    peer.userAgent = userAgent;
    peer.availablePieces = createRandomBitArray(size, percent);
    return peer;
}

/******************************************************************************
 ******************************************************************************/
TorrentPtr BasicTorrent::toTorrent(QObject *parent)
{
    TorrentPtr t(new Torrent(parent));

    qint64 total_size_in_KB = 0;
    foreach (auto basicFile, basicFiles) {
        total_size_in_KB += basicFile.size_in_KB;
    }
    qint64 total_pieces_count = qCeil(qreal(total_size_in_KB) / qreal(piece_size_in_KB));
    qint64 last_piece_size_in_KB = total_size_in_KB - (total_pieces_count - 1) * piece_size_in_KB;

    QString infohash = "A1C231234D653E65D2149056688D2EF93210C1858";
    QStringList trackers;
    trackers << "udp://tracker.example.com:6969/announce";
    trackers << "udp://tracker.example.com:1337";

    QString magnetLink;
    magnetLink = QString("magnet:?xt=urn:btih:%0&dn=%1")
            .arg(infohash)
            .arg(name.replace(".zip", "-ZIP"));
    foreach (auto tracker, trackers) {
        magnetLink += QString("&tr=%0")
                .arg(tracker.replace(':', "%3A").replace('/', "%2F"));
    }
    for (int i = 0; i < 30; ++i) {
        magnetLink += QString("saltsaltsaltsaltsaltsaltsaltsaltsaltsaltsaltsaltsalt");
    }

    t->setLocalFullFileName(QString("C:\\Temp\\DZA-torrent-widget-test\\%0").arg(name));
    t->setUrl(magnetLink);

    TorrentInfo info;
    TorrentMetaInfo metaInfo;
    TorrentHandleInfo detail;

    info.error = TorrentError(TorrentError::NoError);
    info.state = TorrentInfo::stopped;

    info.downloadedPieces = QBitArray(total_pieces_count, false);

    info.bytesReceived = 0;
    info.bytesTotal = total_size_in_KB * KILOBYTES;
    info.percent = 0;
    info.blockSizeInByte = BLOCK_SIZE_BYTES;


    auto offset_in_KB = 0;
    foreach (auto file, basicFiles) {

        TorrentFileMetaInfo::Flags flags;
        if (file.name.contains(".exe")) {
            flags.setFlag(TorrentFileMetaInfo::Executable);

        } else if (file.name.contains("/~")) {
            flags.setFlag(TorrentFileMetaInfo::Hidden);

        } else if (file.name.contains("link")) {
            flags.setFlag(TorrentFileMetaInfo::Symlink);

        } else if (file.name.isEmpty()) {
            flags.setFlag(TorrentFileMetaInfo::PadFile);
        }

        TorrentFileMetaInfo fileMetaInfo;
        fileMetaInfo.filePath = file.name;
        fileMetaInfo.bytesOffset = offset_in_KB * KILOBYTES;
        fileMetaInfo.bytesTotal = file.size_in_KB * KILOBYTES;
        fileMetaInfo.flags = flags;

        TorrentFileInfo fileInfo;

        offset_in_KB += file.size_in_KB;

        detail.files << fileInfo;
        metaInfo.initialMetaInfo.files << fileMetaInfo;
    }

    detail.peers << makePeer(EndPoint("175.158.201.29:32725"), "rTorrent v1.2.3", total_pieces_count);
    detail.peers << makePeer(EndPoint("103.217.176.75:44851"), "", total_pieces_count);
    detail.peers << makePeer(EndPoint("217.63.14.13:14082"  ), "bitTorrent", total_pieces_count);
    detail.peers << makePeer(EndPoint("178.214.192.253:6881"), "toto", total_pieces_count);
    detail.peers << makePeer(EndPoint("71.206.231.37:49958" ), "libTorrent", total_pieces_count);
    detail.peers << makePeer(EndPoint("82.69.12.239:59333"  ), "qBitTorrent", total_pieces_count);
    detail.peers << makePeer(EndPoint("86.120.101.138:42624"), "", total_pieces_count);
    detail.peers << makePeer(EndPoint("175.158.201.29:32725"), "", total_pieces_count);

    foreach (auto tracker, trackers) {
        detail.trackers << TorrentTrackerInfo(tracker);
    }

    detail.httpSeeds << "seed-http-1" << "seed-http-2" << "seed-http-3";
    detail.urlSeeds << "seed-url-1" << "seed-url-2";

    metaInfo.seedsInSwarm = 2;
    metaInfo.peersInSwarm = 64;
    metaInfo.downloadsInSwarm = 12;

    metaInfo.defaultPeers << makePeer(EndPoint("126.0.0.12:655"), "qBitTorrent", total_pieces_count);

    metaInfo.bannedPeers << makePeer(EndPoint("99.66.125.255:81"), "_torrent_H4ck", total_pieces_count);
    metaInfo.bannedPeers << makePeer(EndPoint("99.66.125.255:82"), "_torrent_H4ck", total_pieces_count);

    metaInfo.initialMetaInfo.name = name;
    metaInfo.initialMetaInfo.creationDate = QDateTime(QDate(2020, 1, 1), QTime(16, 42, 57));
    metaInfo.initialMetaInfo.creator = "f0o1";
    metaInfo.initialMetaInfo.comment = "My dummy Torrent file";
    metaInfo.initialMetaInfo.infohash = infohash;
    metaInfo.initialMetaInfo.magnetLink = magnetLink;

    metaInfo.initialMetaInfo.bytesMetaData = 0;
    metaInfo.initialMetaInfo.bytesTotal = total_size_in_KB * KILOBYTES;

    metaInfo.initialMetaInfo.pieceCount = total_pieces_count;
    metaInfo.initialMetaInfo.pieceByteSize = piece_size_in_KB * KILOBYTES;
    metaInfo.initialMetaInfo.pieceLastByteSize = last_piece_size_in_KB * KILOBYTES;

    metaInfo.initialMetaInfo.nodes << TorrentNodeInfo("udp://example.com/", 56408);

    t->setMetaInfo(metaInfo);
    t->setInfo(info, true);
    t->setDetail(detail, true);

    return t;
}

/******************************************************************************
 ******************************************************************************/
TorrentPtr DummyTorrentFactory::createDummyTorrent(QObject *parent)
{
    BasicTorrent b("My.Torrent.zip");

    b.addFile(     10, "My.Torrent/README.txt");
    b.addFile(      2, "My.Torrent/Link.txt");
    b.addFile(  12456, "My.Torrent/data.dat");
    b.addFile(     32, QString());
    b.addFile( 298456, "My.Torrent/bin/Setup.exe");
    b.addFile(   6154, "My.Torrent/bin/config.cfg");
    b.addFile(    120, "My.Torrent/bin/~config.cfg");
    b.addFile(   1560, "My.Torrent/link");
    b.addFile(   4128, "usr/bin/local/dummy");

    return b.toTorrent(parent);
}

/******************************************************************************
 ******************************************************************************/
static QBitArray createRandomBitArray(qint64 size, int percent)
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
/**
 * @brief DummyTorrentFactory::setProgress
 * @param torrent Torrent
 * @param percent Progress between 0 (stopped) and 100 (completed)
 */
void DummyTorrentFactory::setProgress(TorrentPtr torrent, int percent)
{
    Q_ASSERT(torrent);

    const TorrentMetaInfo metaInfo = torrent->metaInfo();
    TorrentInfo info = torrent->info();

    auto pieceCount = metaInfo.initialMetaInfo.pieceCount;
    auto pieceByteSize = metaInfo.initialMetaInfo.pieceByteSize;

    Q_ASSERT(pieceByteSize > 0);

    qint64 bytesReceived = 0;

    // First, create a random piece map
    info.downloadedPieces = createRandomBitArray(pieceCount, percent);

    TorrentHandleInfo detail = torrent->detail();
    int total = metaInfo.initialMetaInfo.files.count();
    for (int i = 0; i < total; ++i) {
        auto fileMetaInfo = metaInfo.initialMetaInfo.files.at(i);

        qint64 bytesOffset = fileMetaInfo.bytesOffset;
        qint64 bytesTotal = fileMetaInfo.bytesTotal;

        auto firstPieceIndex = qCeil(bytesOffset / pieceByteSize);
        auto lastPieceIndex = qCeil((bytesOffset + bytesTotal) / pieceByteSize);
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

    torrent->setInfo(info, false);
    torrent->setDetail(detail, false); // emit changed
}

