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

#ifndef CORE_TORRENT_MESSAGE_H
#define CORE_TORRENT_MESSAGE_H

#include <Core/IDownloadItem>

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

    TorrentError() {}
    explicit TorrentError(Type _type, int _fileIndex = -1)
        : type(_type), fileIndex(_fileIndex), message(QString()) {}

    Type type = NoError;
    int fileIndex = -1;
    QString message;
};

/******************************************************************************
 ******************************************************************************/
class EndPoint // ex: TCP Socket
{
public:
    QString ip;
    int port = 0;

    EndPoint() {}
    EndPoint(QString _ip, int _port) : ip(_ip), port(_port)
    {
    }
    EndPoint(QString input)
    {
        fromString(input);
    }

    void fromString(const QString &input)
    {
        auto fragments = input.split(':');
        ip = fragments.at(0);
        port = fragments.at(1).toInt();
    }

    QString toString() const
    {
        return QString("%0:%1").arg(ip).arg(port);
    }

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

    QString shortFilePath() const
    {
        // Remove the name of the torrent file from the path to make the path shorter
        int i = filePath.indexOf(QChar('\\'));
        if (i < 0)
            i = filePath.lastIndexOf(QChar('/'));
        if (i >= 0)
            i++;
        return filePath.mid(i);
    }


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

    TorrentPeerInfo() {}
    TorrentPeerInfo(EndPoint _endpoint, QString _userAgent) : endpoint(_endpoint), userAgent(_userAgent) {}

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

    TorrentTrackerInfo(QString _url) : url(_url) {}

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
    enum TorrentState {
        stopped,
        checking_files         ,
        downloading_metadata   ,
        downloading            ,
        finished               ,
        seeding                ,
        allocating             ,
        checking_resume_data
    };
    QString torrentStateString() const
    {
        switch (state) {
        case TorrentInfo::stopped                : return QObject::tr("Stopped");
        case TorrentInfo::checking_files         : return QObject::tr("Checking Files...");
        case TorrentInfo::downloading_metadata   : return QObject::tr("Downloading Metadata...");
        case TorrentInfo::downloading            : return QObject::tr("Downloading...");
        case TorrentInfo::finished               : return QObject::tr("Finished");
        case TorrentInfo::seeding                : return QObject::tr("Seeding...");
        case TorrentInfo::allocating             : return QObject::tr("Allocating...");
        case TorrentInfo::checking_resume_data   : return QObject::tr("Checking Resume Data...");
        default: return QString();
        }
    }

    TorrentError error;

    TorrentState state;

    QString lastWorkingTrackerUrl;

    qint64 bytesSessionDownloaded = 0;
    qint64 bytesSessionUploaded = 0;

    qint64 bytesSessionPayloadDownload = 0;
    qint64 bytesSessionPayloadUpload = 0;

    qint64 bytesFailed = 0;
    qint64 bytesRedundant = 0;

    QBitArray downloadedPieces;
    QBitArray verifiedPieces; // seed mode only

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
class TorrentInitialMetaInfo
{
public:
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
};

/******************************************************************************
 ******************************************************************************/
class TorrentMetaInfo
{
public:
    TorrentError error;

    QString status;

    TorrentInitialMetaInfo initialMetaInfo;

    QList<QString> trackers2;
    QList<TorrentNodeInfo> dhtNodes;

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

    QList<TorrentPeerInfo> defaultPeers;
    QList<TorrentPeerInfo> bannedPeers;

    QBitArray downloadedPieces;
    QBitArray verifiedPieces; // seed mode only

    QDateTime lastTimeDownload;
    QDateTime lastTimeUpload;

};

/******************************************************************************
 ******************************************************************************/
typedef QString UniqueId;

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

#endif // CORE_TORRENT_MESSAGE_H
