/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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

#ifndef CORE_TORRENT_H
#define CORE_TORRENT_H

#include <Core/IDownloadItem>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QDateTime>
#include <QtCore/QFlag>
#include <QtCore/QFlags>
#include <QtCore/QHash>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QTime>

#include "libtorrent/sha1_hash.hpp"

template<typename Enum>
static inline void _q_set_flag(QFlags<Enum> *f, Enum flag, bool on = true)
{
#if QT_VERSION >= 0x050700
    (*f).setFlag(flag, on);
#else
    on ? (*f |= flag) : (*f &= ~flag);
#endif
}



/******************************************************************************
 ******************************************************************************/
class TorrentError
{
public:
    enum Type {
        UnknownError,
        FileError,
        SSLContextError,
        FileMetadataError,
        FileExceptionError,
        PartFileError
    };
    TorrentError() {}
    explicit TorrentError(Type _type, int _fileIndex = -1)
        : type(_type), fileIndex(_fileIndex)
    {}

    Type type = UnknownError;
    int fileIndex = -1;
};

/******************************************************************************
 ******************************************************************************/
class EndPoint // ex: TCP Socket
{
public:
    QString ip;
    int port = 0;

    bool operator==(const EndPoint &other) const { return (ip == other.ip && port == other.port); }
    bool operator!=(const EndPoint &other) const { return !(*this == other); }

};

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

    QString hash; // unique identifier

    QDateTime modifiedTime;

    QString symlink;
    QString fileName;
    QString filePath;
    bool isPathAbsolute = false;
    bool isPadFile = false;

    qint64 bytesTotal = 0;
    qint64 bytesOffset = 0;

    quint32 crc32FilePathHash = 0;

    Flags flags;
    void setFlag(Flag flag, bool on = true)
    { _q_set_flag<TorrentFileMetaInfo::Flag>(&flags, flag, on); }
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
    QString priorityString() const
    {
        switch (priority) {
        case Ignore:    return QObject::tr("ignore");
        case Low:       return QObject::tr("low");
        case High:      return QObject::tr("high");
        case Normal:
        default:        return QObject::tr("normal");
        }
    }

    qint64 bytesReceived = 0;
    Priority priority = Normal;
};


/******************************************************************************
 ******************************************************************************/
class TorrentPeerInfo
{
public:
    enum Flag {
        Interesting          ,
        Choked               ,
        RemoteInterested     ,
        RemoteChoked         ,
        SupportsExtensions   ,
        LocalConnection      ,
        Handshake            ,
        Connecting           ,
        //Queued               ,
        OnParole             ,
        Seed                 ,
        OptimisticUnchoke    ,
        Snubbed              ,
        UploadOnly           ,
        Endgame_Mode         ,
        Holepunched          ,
        I2pSocket            ,
        UtpSocket            ,
        SslSocket            ,
        Rc4Encrypted         ,
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

    QString flagString() const
    {
        QString ret;
        for (int i = Interesting; i < Plaintextencrypted; ++i) {
            ret += flags.testFlag(Flag(i)) ? QLatin1String("X") : QLatin1String("-");
        }
        return ret;
    }
    QString sourceFlagString() const
    {
        QString ret;
        for (int i = FromTracker; i < FromPeerIncomingData; ++i) {
            ret += flags.testFlag(Flag(i)) ? QLatin1String("X") : QLatin1String("-");
        }
        return ret;
    }

    EndPoint endpoint;
    QString client;

    /// \todo piecesHoldByThePeer

    qint64 bytesDownloaded = 0;
    qint64 bytesUploaded = 0;

    qint64 lastTimeRequested = 0; // in seconds
    qint64 lastTimeActive = 0;
    qint64 timeDownloadQueue = 0;

    Flags flags;
    SourceFlags sourceFlags;

    void setFlag(Flag flag, bool on = true)
    { _q_set_flag<TorrentPeerInfo::Flag>(&flags, flag, on); }
    void setSourceFlag(SourceFlag flag, bool on = true)
    { _q_set_flag<TorrentPeerInfo::SourceFlag>(&sourceFlags, flag, on); }

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
    QString sourceString() const
    {
        switch (source) {
        case TorrentFile:     return QObject::tr(".torrent file");
        case Client:          return QObject::tr("program settings");
        case MagnetLink:      return QObject::tr("magnet link");
        case TrackerExchange: return QObject::tr("tracker exchange");
        case NoSource:
        default:              return QObject::tr("no source");
        }
    }

    QString url;
    QString trackerId; // '&trackerid=' argument passed to the tracker

    QList<EndPoint> endpoints; // local listen socket (endpoint) announced to the tracker

    int tier = 0;
    int failLimit = 0; // 0 means unlimited
    Source source = NoSource;
    bool isVerified = 0;
};

/******************************************************************************
 ******************************************************************************/
class TorrentHandleInfo // Torrent
{
public:

    int uploadBandwidthLimit = -1; // bytes per second
    int downloadBandwidthLimit = -1;

    int maxUploads = -1; // -1 means unlimited
    int maxConnections = -1;


    QList<TorrentFileInfo> files;
    QList<TorrentPeerInfo> peers;
    QList<TorrentTrackerInfo> trackers;


    QList<QString> httpSeeds; // if not empty, this list overrides the ones given in .torrent file
    QList<QString> urlSeeds; /// \todo unify seeds QLists
};


/******************************************************************************
 ******************************************************************************/
class TorrentInfo // TorrentStatusInfo
{
public:
    bool hasError = false;
    TorrentError error;

    IDownloadItem::State status;

    QString lastWorkingTrackerUrl;

    qint64 bytesSessionDownloaded = 0;
    qint64 bytesSessionUploaded = 0;

    qint64 bytesSessionPayloadDownload = 0;
    qint64 bytesSessionPayloadUpload = 0;

    qint64 bytesFailed = 0;
    qint64 bytesRedundant = 0;

    qint64 bytesReceived = 0;
    qint64 bytesTotal = 0;

    qint64 bytesWantedReceived = 0; // == bytesReceived - padding bytes
    qint64 bytesWantedTotal = 0; // == bytesTotal - padding bytes

    qint64 bytesAllSessionsPayloadDownload = 0;
    qint64 bytesAllSessionsPayloadUpload = 0;

    QDateTime addedTime;   /// \todo maybe it's duplicate?
    QDateTime completedTime;
    QDateTime lastSeenCompletedTime;

    qreal percent = 0; // between 0 and 100

    qreal downloadSpeed = 0; // bytes per second
    qreal uploadSpeed = 0;

    int download_payload_rate = 0; // better speed to calc the ETAs
    int upload_payload_rate = 0;

    QTime elapsedTime; /// \todo ETA
    QTime remaingTime; /// \todo ETA

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

    int blockSizeInByte = 0;

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

    QString infohash;


    qint64 activeTimeDuration = 0; // in seconds
    qint64 finishedTimeDuration = 0; /// \todo duplicate?
    qint64 seedingTimeDuration = 0;

    QDateTime lastTimeDownload; /// \todo duplicate?
    QDateTime lastTimeUpload;

};


/******************************************************************************
 ******************************************************************************/
class TorrentNodeInfo
{
public:
    TorrentNodeInfo() {}
    explicit TorrentNodeInfo(QString _host, int _port)
        : host(_host), port(_port)
    {}
    QString host;
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
    QString url;
    QString auth;
    QList<QPair<QString, QString> > extraHeaders;
    Type type = UrlSeed;
};

/******************************************************************************
 ******************************************************************************/
class TorrentPeerMetaInfo /// \todo remove
{
public:
    EndPoint endpoint;
};

/******************************************************************************
 ******************************************************************************/
class TorrentMetaInfo
{
public:
    bool hasError = false; // true if magnet link or torrent file is not valid

    QString status;

    QString name;
    QDateTime creationDate;
    QString creator;
    QString comment;
    QString infohash;
    QString magnetLink;

    qint64 bytesMetaData = 0;

    qint64 bytesTotal = 0;
    int pieceCount = 0;
    int pieceByteSize = 0; // piece's size in byte (generally 16 kB)
    int pieceLastByteSize = 0; // last piece's size in byte, can be less than 16 kB

    QString sslRootCertificate; // public certificate in x509 format

    bool isPrivate = false;
    bool isI2P = false;

    QList<TorrentNodeInfo> nodes;

    QList<TorrentFileMetaInfo> files;
    QList<TorrentTrackerInfo> trackers;

    QList<QString> similarTorrents;
    QList<QString> collections;

    QList<TorrentWebSeedMetaInfo> webSeeds;

    QList<QString> trackers2;
    QList<TorrentNodeInfo> dhtNodes;

    QString outputName;
    QString outputPath;

    QString defaultTrackerId;
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

    QDateTime addedTime;
    QDateTime completedTime;
    QDateTime lastSeenCompletedTime;

    // -1 indicates we don't know, or we have not received any scrape data.
    int seedsInSwarm = -1; // number of peers in the swarm that are seeds, or have every piece in the torrent.
    int peersInSwarm = -1; // number of peers in the swarm that do not have  every piece.
    int downloadsInSwarm = -1; // number of times the torrent has been downloaded (not initiated, but the number of times a download has completed).


    QList<QString> httpSeeds; // if not empty, this list overrides the ones given in .torrent file
    QList<QString> urlSeeds; /// \todo unify seeds QLists

    QList<TorrentPeerMetaInfo> defaultPeers;
    QList<TorrentPeerMetaInfo> bannedPeers;

    QDateTime lastTimeDownload;
    QDateTime lastTimeUpload;

};

/******************************************************************************
 ******************************************************************************/
typedef lt::sha1_hash UniqueId;

/*
 * Note: qHash() must be declared inside the object's namespace
 */
namespace libtorrent {
inline uint qHash(const sha1_hash &key, uint seed) {
    return qHash(QString::fromStdString(key.to_string()), seed);
}
}

struct TorrentData
{
    UniqueId unique_id;
    TorrentMetaInfo metaInfo;
    TorrentHandleInfo detail;
};

struct TorrentStatus
{
    UniqueId unique_id;
    TorrentInfo info;
    TorrentHandleInfo detail;
};

Q_DECLARE_METATYPE(TorrentData)
Q_DECLARE_METATYPE(TorrentStatus)

#endif // CORE_TORRENT_H