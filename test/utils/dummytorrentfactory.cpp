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

/*!
 * \class TorrentSkeleton
 *  \brief Contains minimal data to generate a valid and reproduceable torrent.
 */
class TorrentSkeleton
{
public:
    TorrentSkeleton(const QString &name, qint64 piece_size_in_KB = 32);

    void addFile(qint64 size, const QString &name);

    TorrentPtr toTorrent(QObject *parent);

private:
    QString m_name;
    qint64 m_piece_size_in_KB;

    struct BasicFile
    {
        BasicFile(QString _name, qint64 _size_in_KB)
            : name(_name)
            , size_in_KB(_size_in_KB)
        {}

        QString name;
        qint64 size_in_KB;

    };
    QList<BasicFile> m_basicFiles;

    TorrentPeerInfo toPeer(const EndPoint &endpoint,
                             const QString &pieceSketch,
                             const QString &userAgent, qint64 size);
    QBitArray toAvailablePieces(int size, const QString &pieceSketch);
};

/******************************************************************************
 ******************************************************************************/
TorrentSkeleton::TorrentSkeleton(const QString &name, qint64 piece_size_in_KB)
    : m_name(name)
    , m_piece_size_in_KB(piece_size_in_KB)
{
}

void TorrentSkeleton::addFile(qint64 size, const QString &name)
{
    m_basicFiles << BasicFile(name, size);
}

/******************************************************************************
 ******************************************************************************/
/*! ex: "XXX---AA"
 *
 * 'X' means all the pieces in the section are available
 * 'A' means half the pieces in the section are available ('alternate')
 * '-' means no piece in the section is available
 *
 */
TorrentPeerInfo TorrentSkeleton::toPeer(const EndPoint &endpoint,
                                          const QString &pieceSketch,
                                          const QString &userAgent,
                                          qint64 size)
{
    TorrentPeerInfo peer;
    peer.endpoint = endpoint;
    peer.userAgent = userAgent;
    peer.availablePieces = toAvailablePieces(static_cast<int>(size), pieceSketch);
    return peer;
}

QBitArray TorrentSkeleton::toAvailablePieces(int size, const QString &pieceSketch)
{
    QBitArray ba = QBitArray(size, false);
    const int count = pieceSketch.count();
    const int sectionSize = qCeil(qreal(size) / count);
    for (int i = 0; i < count; ++i) {
        auto sectionBegin = i * sectionSize;
        auto sectionEnd = qMin(size, (i + 1) * sectionSize);
        auto ch = pieceSketch.at(i);
        if (ch == QLatin1Char('X')) {
            for (int j = sectionBegin; j < sectionEnd; ++j) {
                ba.setBit(j);
            }
        } else if (ch == QLatin1Char('A')) {
            for (int j = sectionBegin; j < sectionEnd; j += 2) {
                ba.setBit(j);
            }
        }
    }
    return ba;
}

/******************************************************************************
 ******************************************************************************/
TorrentPtr TorrentSkeleton::toTorrent(QObject *parent)
{
    TorrentPtr t(new Torrent(parent));

    qint64 total_size_in_KB = 0;
    foreach (auto basicFile, m_basicFiles) {
        total_size_in_KB += basicFile.size_in_KB;
    }
    qint64 total_pieces_count = qCeil(qreal(total_size_in_KB) / qreal(m_piece_size_in_KB));
    qint64 last_piece_size_in_KB = total_size_in_KB - (total_pieces_count - 1) * m_piece_size_in_KB;

    QString infohash = "A1C231234D653E65D2149056688D2EF93210C1858";
    QStringList trackers;
    trackers << "udp://tracker.example.com:6969/announce";
    trackers << "udp://tracker.example.com:1337";

    QString magnetLink;
    magnetLink = QString("magnet:?xt=urn:btih:%0&dn=%1")
            .arg(infohash)
            .arg(m_name.replace(".zip", "-ZIP"));
    foreach (auto tracker, trackers) {
        magnetLink += QString("&tr=%0")
                .arg(tracker.replace(':', "%3A").replace('/', "%2F"));
    }
    for (int i = 0; i < 30; ++i) {
        magnetLink += QString("saltsaltsaltsaltsaltsaltsaltsaltsaltsaltsaltsaltsalt");
    }

    t->setLocalFullFileName(QString("C:\\Temp\\DZA-torrent-widget-test\\%0").arg(m_name));
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
    foreach (auto file, m_basicFiles) {

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

    detail.peers << toPeer(EndPoint("164.10.201.129:30025"), "XA-AA----A", "rTorrent v1.2.3", total_pieces_count);
    detail.peers << toPeer(EndPoint("103.217.176.75:44851"), "XXXXXXXXXX", "", total_pieces_count);
    detail.peers << toPeer(EndPoint("217.63.14.13:14082"  ), "-X-A-----A", "bitTorrent", total_pieces_count);
    detail.peers << toPeer(EndPoint("178.214.192.253:6881"), "-----X----", "toto", total_pieces_count);
    detail.peers << toPeer(EndPoint("71.206.231.37:49958" ), "-A-------A", "libTorrent", total_pieces_count);
    detail.peers << toPeer(EndPoint("82.69.12.239:59333"  ), "----------", "qBitTorrent", total_pieces_count);
    detail.peers << toPeer(EndPoint("86.120.101.138:42624"), "XA--------", "", total_pieces_count);
    detail.peers << toPeer(EndPoint("175.158.201.29:32725"), "XXXXXX--X-", "", total_pieces_count);

    foreach (auto tracker, trackers) {
        detail.trackers << TorrentTrackerInfo(tracker);
    }

    detail.httpSeeds << "seed-http-1" << "seed-http-2" << "seed-http-3";
    detail.urlSeeds << "seed-url-1" << "seed-url-2";

    metaInfo.seedsInSwarm = 2;
    metaInfo.peersInSwarm = 64;
    metaInfo.downloadsInSwarm = 12;

    metaInfo.defaultPeers << toPeer(EndPoint("126.0.0.12:655"), "XXXXXX--XX", "qBitTorrent", total_pieces_count);

    metaInfo.bannedPeers << toPeer(EndPoint("99.66.125.255:81"), "XXXXXX--XX", "_torrent_H4ck", total_pieces_count);
    metaInfo.bannedPeers << toPeer(EndPoint("99.66.125.255:82"), "-XXXA----x", "_torrent_H4ck", total_pieces_count);

    metaInfo.initialMetaInfo.name = m_name;
    metaInfo.initialMetaInfo.creationDate = QDateTime(QDate(2020, 1, 1), QTime(16, 42, 57));
    metaInfo.initialMetaInfo.creator = "f0o1";
    metaInfo.initialMetaInfo.comment = "My dummy Torrent file";
    metaInfo.initialMetaInfo.infohash = infohash;
    metaInfo.initialMetaInfo.magnetLink = magnetLink;

    metaInfo.initialMetaInfo.bytesMetaData = 0;
    metaInfo.initialMetaInfo.bytesTotal = total_size_in_KB * KILOBYTES;

    metaInfo.initialMetaInfo.pieceCount = total_pieces_count;
    metaInfo.initialMetaInfo.pieceByteSize = m_piece_size_in_KB * KILOBYTES;
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
    TorrentSkeleton b("My.Torrent.zip");

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
