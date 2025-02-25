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

#ifndef CORE_TORRENT_MESSAGE_H
#define CORE_TORRENT_MESSAGE_H

#include <QtCore/QBitArray>
#include <QtCore/QDateTime>
#include <QtCore/QFlag>
#include <QtCore/QFlags>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QTime>
#include <QtNetwork/QHostAddress>


/******************************************************************************
 ******************************************************************************/
class TorrentError
{
public:
    enum Type {
        NoError,

        /* Errors when adding the torrent to the queue */
        // Couldn't download Magnet link or torrent file Metadata
        MetadataDownloadError,

        // Magnet link or torrent file is not valid
        FailedToAddError,

        // Torrent isn't loaded yet, primarily because magnet link
        // has not had its metadata resolved yet.
        NoInfoYetError,

        /* Errors when downloading */
        FileError,
        SSLContextError,
        FileMetadataError,
        FileExceptionError,
        PartFileError,

        /* Other */
        UnknownError
    };

    TorrentError() = default;
    explicit TorrentError(Type _type, int _fileIndex = -1);

    auto operator<=>(const TorrentError&) const = default;

    Type type = NoError;
    int fileIndex = -1;
    QString message = {};
};

/******************************************************************************
 ******************************************************************************/
class EndPoint // ex: TCP Socket
{
public:
    EndPoint() = default;
    EndPoint(const QString &address, int port);
    EndPoint(const QString &addressAndPort);

    auto operator<=>(const EndPoint&) const = default;

    QHostAddress ip() const;
    QString ipToString() const;
    int port() const;

    void setAddressAndPort(const QString &addressAndPort);

    QString toString() const;
    QString sortableIp() const;

private:
    QHostAddress m_ip = {};
    int m_port{0};
};

inline size_t qHash(const EndPoint &key, size_t seed)
{
    return qHashMulti(seed, key.ip(), key.port());
}


/******************************************************************************
 ******************************************************************************/
class TorrentFileMetaInfo // immutable data
{
public:
    enum Flag {
        PadFile = 0x0,
        Hidden = 0x1,
        Executable = 0x2,
        Symlink = 0x4
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    TorrentFileMetaInfo() = default;
    explicit TorrentFileMetaInfo(qsizetype _bytesTotal,
                                 qsizetype _bytesOffset,
                                 quint32 _crc32FilePathHash,
                                 const QString &_fileName,
                                 const QString &_filePath);

    auto operator<=>(const TorrentFileMetaInfo&) const = default;

    QString hash = {}; // unique identifier

    QDateTime modifiedTime = {};

    QString symlink = {};
    QString fileName = {};
    QString filePath = {};

    QString shortFilePath() const;

    bool isPathAbsolute = false;
    bool isPadFile = false;

    qsizetype bytesTotal = 0;
    qsizetype bytesOffset = 0;

    quint32 crc32FilePathHash = 0;

    Flags flags;

    void setFlag(Flag flag, bool on = true);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TorrentFileMetaInfo::Flags)

class TorrentFileInfo
{
public:
    enum Priority {
        Ignore,
        Low,
        Normal,
        High
    };

    auto operator<=>(const TorrentFileInfo&) const = default;

    QString priorityString() const;

    qsizetype bytesReceived = 0;
    Priority priority = Normal;
};


/******************************************************************************
 ******************************************************************************/
class TorrentPeerInfo
{
public:
    enum Flag {
        Interesting,
        Choked,
        RemoteInterested,
        RemoteChoked,
        SupportsExtensions,
        LocalConnection,
        Handshake,
        Connecting,
        //Queued,
        OnParole,
        Seed,
        OptimisticUnchoke,
        Snubbed,
        UploadOnly,
        Endgame_Mode,
        Holepunched,
        I2pSocket,
        UtpSocket,
        SslSocket,
        Rc4Encrypted,
        Plaintextencrypted
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    enum SourceFlag {
        FromTracker,
        FromDHT,
        FromPeerExchange,
        FromLocalServiceDiscovery,
        FromFastResumeData,
        FromPeerIncomingData
    };
    Q_DECLARE_FLAGS(SourceFlags, SourceFlag)

    TorrentPeerInfo() = default;
    TorrentPeerInfo(const EndPoint &_endpoint, const QString &_userAgent);

    auto operator<=>(const TorrentPeerInfo&) const = default;

    static QString flagUnicodeSymbol(Flag flag);
    static QString sourceFlagUnicodeSymbol(SourceFlag flag);
    static QString flagComment(Flag flag);
    static QString sourceFlagComment(SourceFlag flag);

    QString flagString() const;
    QString sourceFlagString() const;

    static QString flagTooltip();
    static QString sourceFlagTooltip();

    EndPoint endpoint;
    QString userAgent;

    QBitArray availablePieces; // 1: peer has that piece, 0: peer miss that piece

    qint64 bytesDownloaded = 0;
    qint64 bytesUploaded = 0;

    qint64 lastTimeRequested = 0; // in seconds
    qint64 lastTimeActive = 0;
    qint64 timeDownloadQueue = 0;

    Flags flags;
    SourceFlags sourceFlags;

    void setFlag(Flag flag, bool on = true);
    void setSourceFlag(SourceFlag flag, bool on = true);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TorrentPeerInfo::Flags)
Q_DECLARE_OPERATORS_FOR_FLAGS(TorrentPeerInfo::SourceFlags)

/******************************************************************************
 ******************************************************************************/
class TorrentTrackerInfo
{
public:
    enum Source {
        NoSource = 0,
        TorrentFile = 1,
        Client = 2,
        MagnetLink = 4,
        TrackerExchange = 8
    };

    TorrentTrackerInfo(const QString &_url,
                       int _tier = 0,
                       TorrentTrackerInfo::Source _source = TorrentTrackerInfo::NoSource);

    auto operator<=>(const TorrentTrackerInfo&) const = default;

    QString sourceString() const;

    QString url = {};
    QString trackerId = {}; // '&trackerid=' argument passed to the tracker

    QList<EndPoint> endpoints; // local listen socket (endpoint) announced to the tracker

    int tier = 0;
    int failLimit = 0; // 0 means unlimited
    Source source = NoSource;
    bool isVerified = false;
};

/******************************************************************************
 ******************************************************************************/
class TorrentHandleInfo // Torrent
{
public:
    auto operator<=>(const TorrentHandleInfo&) const = default;

    int uploadBandwidthLimit = -1; // bytes per second
    int downloadBandwidthLimit = -1;

    int maxUploads = -1; // -1 means unlimited
    int maxConnections = -1;

    QList<TorrentFileInfo> files;
    QList<TorrentPeerInfo> peers;
    QList<TorrentTrackerInfo> trackers;

    QList<QString> httpSeeds; // if not empty, this list overrides the ones given in .torrent file
    QList<QString> urlSeeds; /// \todo unify seeds QLists

    QVector<int> pieceAvailability; // number of peers having the piece. 0 = unavailable piece.
    QVector<TorrentFileInfo::Priority> piecePriority;
};

/******************************************************************************
 ******************************************************************************/
class TorrentInfo // TorrentStatusInfo
{
public:
    enum TorrentState {
        stopped,
        checking_files         ,
        downloading_metadata   ,
        downloading            ,
        finished               ,
        seeding                ,
        checking_resume_data
    };

    auto operator<=>(const TorrentInfo&) const = default;

    QString torrentStateString() const;
    const char* torrentState_c_str() const;

    TorrentError error = {};

    TorrentState state = stopped;

    QString lastWorkingTrackerUrl = {};

    qsizetype bytesSessionDownloaded = 0;
    qsizetype bytesSessionUploaded = 0;

    qsizetype bytesSessionPayloadDownload = 0;
    qsizetype bytesSessionPayloadUpload = 0;

    qsizetype bytesFailed = 0;
    qsizetype bytesRedundant = 0;

    QBitArray downloadedPieces = {};
    QBitArray verifiedPieces = {}; // seed mode only

    qsizetype bytesReceived = 0;
    qsizetype bytesTotal = 0;

    qsizetype bytesWantedReceived = 0; // == bytesReceived - padding bytes
    qsizetype bytesWantedTotal = 0; // == bytesTotal - padding bytes

    qsizetype bytesAllSessionsPayloadDownload = 0;
    qsizetype bytesAllSessionsPayloadUpload = 0;

    QDateTime addedTime = {};   /// \todo maybe it's duplicate?
    QDateTime completedTime = {};
    QDateTime lastSeenCompletedTime = {};

    qreal percent = 0; // between 0 and 100

    qreal downloadSpeed = 0; // bytes per second
    qreal uploadSpeed = 0;

    int download_payload_rate = 0; // better speed to calc the ETAs
    int upload_payload_rate = 0;

    QTime elapsedTime = {}; /// \todo ETA
    QTime remaingTime = {}; /// \todo ETA

    int connectedSeedsCount = 0;
    int connectedPeersCount = 0;

    int completePeersCount = 0; // number of peers having the whole file
    int incompletePeersCount = 0; // number of peers still downloading

    int seedsCount = 0;
    int peersCount = 0;

    int candidatePeersCount = 0;

    int downloadedPiecesCount = 0;

    int distributedFullCopiesCount = 0; // number of distributed copies of the torrent
    int distributedFraction = 0;
    qreal distributedCopiesFraction = 0;

    qsizetype blockSizeInByte = 0;

    int peersUnchokedCount = 0;
    int peersConnectionCount = 0;

    int uploadSlotsLimit = 0;
    int connectionsNumberLimit = 0;

    int upBandwidthQuotaQueue = 0; // number of peers waiting for more bandwidth quota
    int downBandwidthQuotaQueue = 0;

    int seedRank = 0;

    bool isSeeding = false;
    bool isFinished = false;
    bool hasMetadata = false;
    bool hasIncoming = false;
    bool isMovingStorage = false;

    bool isAnnouncingToTrackers = false;
    bool isAnnouncingToLSD = false;
    bool isAnnouncingToDHT = false;

    QString infohash = {};

    qint64 activeTimeDuration = 0; // in seconds
    qint64 finishedTimeDuration = 0; /// \todo duplicate?
    qint64 seedingTimeDuration = 0;

    QDateTime lastTimeDownload = {}; /// \todo duplicate?
    QDateTime lastTimeUpload = {};
};


/******************************************************************************
 ******************************************************************************/
class TorrentNodeInfo
{
public:
    TorrentNodeInfo() = default;
    explicit TorrentNodeInfo(const QString &_host, int _port);

    auto operator<=>(const TorrentNodeInfo&) const = default;

    QString host = {};
    int port = 0;
};


/******************************************************************************
 ******************************************************************************/
class TorrentWebSeedMetaInfo
{
public:
    enum Type {
        UrlSeed,
        HttpSeed
    };

    auto operator<=>(const TorrentWebSeedMetaInfo&) const = default;

    QString url = {};
    QString auth = {};
    QList<QPair<QString, QString> > extraHeaders = {};
    Type type = UrlSeed;
};

/******************************************************************************
 ******************************************************************************/
class TorrentInitialMetaInfo
{
public:
    auto operator<=>(const TorrentInitialMetaInfo&) const = default;

    QString name = {};
    QDateTime creationDate = {};
    QString creator = {};
    QString comment = {};
    QString infohash = {};
    QString magnetLink = {};

    qint64 bytesMetaData = 0;

    qint64 bytesTotal = 0;
    qint64 pieceCount = 0;
    qint64 pieceByteSize = 0; // piece's size in byte (generally 16 kB)
    qint64 pieceLastByteSize = 0; // last piece's size in byte, can be less than 16 kB

    QString sslRootCertificate = {}; // public certificate in x509 format

    bool isPrivate = false;
    bool isI2P = false;

    QList<TorrentNodeInfo> nodes = {};

    QList<TorrentFileMetaInfo> files = {};
    QList<TorrentTrackerInfo> trackers = {};

    QList<QString> similarTorrents = {};
    QList<QString> collections = {};

    QList<TorrentWebSeedMetaInfo> webSeeds = {};
};

/******************************************************************************
 ******************************************************************************/
class TorrentMetaInfo
{
public:
    auto operator<=>(const TorrentMetaInfo&) const = default;

    TorrentError error = {};

    QString status = {};

    TorrentInitialMetaInfo initialMetaInfo = {};

    QList<QString> trackers2 = {};
    QList<TorrentNodeInfo> dhtNodes = {};

    QString outputPath = {};

    QString defaultTrackerId = {};
    /// \todo flags torrent_flags_t OPTIONS torrent_flags.hpp
    /// \todo QString info_hash; // in case we don't have magnet link nor torrent file

    int maxUploads = -1;    // -1 means unlimited
    int maxConnections = -1;

    int uploadBandwidthLimit = -1; // bytes per second
    int downloadBandwidthLimit = -1;

    // the total number of bytes uploaded and downloaded by this torrent so far.
    qint64 bytesTotalUploaded = 0;
    qint64 bytesTotalDownloaded = 0;

    // the number of seconds this torrent has spent in started, finished and
    // seeding state so far, respectively.
    int activeTimeDuration = 0; // in seconds
    int finishedTimeDuration = 0;
    int seedingTimeDuration = 0;

    QDateTime addedTime = {};
    QDateTime completedTime = {};
    QDateTime lastSeenCompletedTime = {};

    // -1 indicates we don't know, or we have not received any scrape data.
    int seedsInSwarm = -1; // number of peers in the swarm that are seeds, or have every piece in the torrent.
    int peersInSwarm = -1; // number of peers in the swarm that do not have  every piece.
    int downloadsInSwarm = -1; // number of times the torrent has been downloaded (not initiated, but the number of times a download has completed).


    QList<QString> httpSeeds; // if not empty, this list overrides the ones given in .torrent file
    QList<QString> urlSeeds; /// \todo unify seeds QLists

    QList<TorrentPeerInfo> defaultPeers = {};
    QList<TorrentPeerInfo> bannedPeers = {};

    QBitArray unfinishedPieces = {};
    QBitArray downloadedPieces = {};
    QBitArray verifiedPieces = {}; // seed mode only

    QDateTime lastTimeDownload = {};
    QDateTime lastTimeUpload = {};

};

/******************************************************************************
 ******************************************************************************/
using UniqueId = QString;

struct TorrentData
{
    auto operator<=>(const TorrentData&) const = default;

    UniqueId unique_id = {};
    TorrentMetaInfo metaInfo = {};
    TorrentHandleInfo detail = {};
};

struct TorrentStatus
{
    auto operator<=>(const TorrentStatus&) const = default;

    UniqueId unique_id = {};
    TorrentInfo info = {};
    TorrentHandleInfo detail = {};
};

/* Enable the type to be used with QVariant. */
Q_DECLARE_METATYPE(TorrentData)

Q_DECLARE_METATYPE(TorrentStatus)


#endif // CORE_TORRENT_MESSAGE_H
