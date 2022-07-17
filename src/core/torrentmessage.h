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
#include <QtNetwork/QHostAddress>

static const QString s_glasses = QString::fromUtf8("\xf0\x9f\x91\x93"); //👓
static const QString s_smile = QString::fromUtf8("\xF0\x9F\x98\x83"); //😃
static const QString s_sad = QString::fromUtf8("\xf0\x9f\x98\x9e "); //😞
static const QString s_love = QString::fromUtf8("\xf0\x9f\x92\x8c"); // 💌
static const QString s_bomb = QString::fromUtf8("\xf0\x9f\x92\xa3"); // 💣
static const QString s_thumb_up = QString::fromUtf8("\xf0\x9f\x91\x8d"); // 👍
static const QString s_red_flag = QString::fromUtf8("\xf0\x9f\x9a\xa9"); // 🚩
static const QString s_hand_shake = QString::fromUtf8("\xf0\x9f\xa4\x9d"); // 🤝
static const QString s_collision = QString::fromUtf8("\xf0\x9f\x92\xa5"); // 💥
static const QString s_upload = QString::fromUtf8("\xf0\x9f\x93\xa4"); // 📤
static const QString s_dice_game = QString::fromUtf8("\xf0\x9f\x8e\xb2"); // 🎲
static const QString s_electric_plug = QString::fromUtf8("\xf0\x9f\x94\x8c"); // 🔌
static const QString s_light_bulb = QString::fromUtf8("\xf0\x9f\x92\xa1"); // 💡
static const QString s_banana = QString::fromUtf8("\xf0\x9f\x8d\x8c"); // 🍌
static const QString s_speech = QString::fromUtf8("\xf0\x9f\x92\xac"); // 💬
static const QString s_dog_face = QString::fromUtf8("\xf0\x9f\x90\xb6"); // 🐶
static const QString s_receipt = QString::fromUtf8("\xf0\x9f\xa7\xbe"); // 🧾
static const QString s_hole = QString::fromUtf8("\xf0\x9f\x95\xb3\xef\xb8\x8f"); // 🕳️
static const QString s_key = QString::fromUtf8("\xf0\x9f\x94\x91"); // 🔑
static const QString s_locked = QString::fromUtf8("\xf0\x9f\x94\x92"); // 🔒
static const QString s_unlocked = QString::fromUtf8("\xf0\x9f\x94\x93"); // 🔓


template<typename Enum>
static inline void _q_set_flag(QFlags<Enum> *f, Enum flag, bool on = true)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
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

    TorrentError() = default;
    explicit TorrentError(Type _type, int _fileIndex = -1)
        : type(_type), fileIndex(_fileIndex), message(QString()) {}

    Type type{NoError};
    int fileIndex{-1};
    QString message;
};

/******************************************************************************
 ******************************************************************************/
class EndPoint // ex: TCP Socket
{
public:
    EndPoint() = default;
    EndPoint(const QString &address, int port) : m_ip(QHostAddress(address)), m_port(port)
    {
    }
    EndPoint(const QString &addressAndPort)
    {
        setAddressAndPort(addressAndPort);
    }

    QHostAddress ip() const { return m_ip; }
    QString ipToString() const { return m_ip.toString(); }

    int port() const { return m_port; }

    void setAddressAndPort(const QString &addressAndPort)
    {
        if (!addressAndPort.contains(QLatin1Char(':'))) {
            m_ip = QHostAddress(addressAndPort);
            m_port = 0;
        } else {
            auto pos = addressAndPort.lastIndexOf(QLatin1Char(':'));
            auto ipFragment = addressAndPort.mid(0, pos - 1);
            auto portFragment = addressAndPort.mid(pos + 1);
            m_ip = QHostAddress(ipFragment);
            m_port = portFragment.toInt();
        }
    }

    QString toString() const
    {
        return QString("%0:%1").arg(m_ip.toString(), QString::number(m_port));
    }

    // Used to sort the ip addresses.
    // Make sure that 2.0.0.0 follows 1.0.0.0 instead of 102.0.0.0
    QString sortableIp() const
    {
        auto ipv6 = m_ip.toIPv6Address();
        QString ret;
        for (int i = 0; i < 16; ++i) {
            const quint8 f = ipv6[i]; // 0 - 255
            ret.append(QString::number(f).rightJustified(3, QLatin1Char('0')));
        }
        return ret;
    }


private:
    QHostAddress m_ip;
    int m_port{0};
};

inline bool operator==(const EndPoint &e1, const EndPoint &e2)
{
    return e1.ip() == e2.ip() && e1.port() == e2.port();
}

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
        case Normal:    return QObject::tr("normal");
        }
        Q_UNREACHABLE();
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

    static QString flagUnicodeSymbol(Flag flag) {
        switch (flag) {
        case Interesting          : return s_glasses;
        case Choked               : return s_bomb;
        case RemoteInterested     : return s_love;
        case RemoteChoked         : return s_red_flag;
        case SupportsExtensions   : return s_receipt;
        case LocalConnection      : return s_smile;
        case Handshake            : return s_hand_shake;
        case Connecting           : return s_thumb_up;
            //case Queued
        case OnParole             : return s_speech;
        case Seed                 : return s_dog_face;
        case OptimisticUnchoke    : return s_sad;
        case Snubbed              : return s_collision;
        case UploadOnly           : return s_upload;
        case Endgame_Mode         : return s_dice_game;
        case Holepunched          : return s_hole;
        case I2pSocket            : return s_electric_plug;
        case UtpSocket            : return s_light_bulb;
        case SslSocket            : return s_banana;
        case Rc4Encrypted         : return s_key;
        case Plaintextencrypted   : return s_locked;
        }
        Q_UNREACHABLE();
    }

    static QString sourceFlagUnicodeSymbol(SourceFlag flag) {
        switch (flag) {
        case FromTracker                : return s_banana;
        case FromDHT                    : return s_bomb;
        case FromPeerExchange           : return s_love;
        case FromLocalServiceDiscovery  : return s_red_flag;
        case FromFastResumeData         : return s_dice_game;
        case FromPeerIncomingData       : return s_hand_shake;
        }
        Q_UNREACHABLE();
    }

    static QString flagComment(Flag flag) {
        switch (flag) {
        case Interesting          : return QLatin1String("We are interested in pieces from this peer.");
        case Choked               : return QLatin1String("We have choked this peer.");
        case RemoteInterested     : return QLatin1String("The peer is interested in us");
        case RemoteChoked         : return QLatin1String("The peer has choked us.");
        case SupportsExtensions   : return QLatin1String("The peer supports the extension protocol.");
        case LocalConnection      : return QLatin1String("The peer connection was opened by us.");
        case Handshake            : return QLatin1String("The handshake is done.");
        case Connecting           : return QLatin1String("The connection is in a half-open state.");
            //case Queued              : return QLatin1String("The connection is currently queued for a connection attempt.");
        case OnParole             : return QLatin1String("The peer has failed the hash check.");
        case Seed                 : return QLatin1String("The peer is a seed (it has all the pieces).");
        case OptimisticUnchoke    : return QLatin1String("The peer is subject to an optimistic unchoke.");
        case Snubbed              : return QLatin1String("The peer has recently failed to send a block.");
        case UploadOnly           : return QLatin1String("The peer told us that it will not downloading anything more.");
        case Endgame_Mode         : return QLatin1String("All pieces this peer has were already requested from other peers.");
        case Holepunched          : return QLatin1String("The peer is in holepunch mode (NAT holepunch mechanism).");
        case I2pSocket            : return QLatin1String("The socket is running on I2P transport.");
        case UtpSocket            : return QLatin1String("The socket is a uTP socket.");
        case SslSocket            : return QLatin1String("The socket is running on SSL (TLS) channel.");
        case Rc4Encrypted         : return QLatin1String("The connection is obfuscated with RC4.");
        case Plaintextencrypted   : return QLatin1String("The connection handshake was obfuscated with a Diffie-Hellman exchange.");
        }
        Q_UNREACHABLE();
    }

    static QString sourceFlagComment(SourceFlag flag) {
        switch (flag) {
        case FromTracker               : return QLatin1String("The peer was received from the tracker.");
        case FromDHT                   : return QLatin1String("The peer was received from the kademlia DHT.");
        case FromPeerExchange          : return QLatin1String("The peer was received from the peer exchange extension.");
        case FromLocalServiceDiscovery : return QLatin1String("The peer was received from the local service discovery (The peer is on the local network).");
        case FromFastResumeData        : return QLatin1String("The peer was added from the fast resume data.");
        case FromPeerIncomingData      : return QLatin1String("We received an incoming connection from this peer.");
        }
        Q_UNREACHABLE();
    }

    QString flagString() const
    {
        QString ret;
        for (int i = Interesting; i < Plaintextencrypted; ++i) {
            if (flags.testFlag(Flag(i))) {
                ret += TorrentPeerInfo::flagUnicodeSymbol(Flag(i));
            }
        }
        return ret;
    }
    QString sourceFlagString() const
    {
        QString ret;
        for (int i = FromTracker; i < FromPeerIncomingData; ++i) {
            if (sourceFlags.testFlag(SourceFlag(i))) {
                ret += TorrentPeerInfo::sourceFlagUnicodeSymbol(SourceFlag(i));
            }
        }
        return ret;
    }

    static QString flagTooltip() {
        QString ret = QLatin1String("Flags:");
        for (int i = Interesting; i < Plaintextencrypted; ++i) {
            ret += QString("\n- %0 : %1").arg(
                        TorrentPeerInfo::flagUnicodeSymbol(Flag(i)),
                        TorrentPeerInfo::flagComment(Flag(i)));
        }
        return ret;
    }

    static QString sourceFlagTooltip() {
        QString ret = QLatin1String("Source Flags:");
        for (int i = FromTracker; i < FromPeerIncomingData; ++i) {
            ret += QString("\n- %0 : %1").arg(
                        TorrentPeerInfo::sourceFlagUnicodeSymbol(SourceFlag(i)),
                        TorrentPeerInfo::sourceFlagComment(SourceFlag(i)));
        }
        return ret;
    }

    TorrentPeerInfo() = default;
    TorrentPeerInfo(const EndPoint &_endpoint, const QString &_userAgent)
        : endpoint(_endpoint), userAgent(_userAgent) {}

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
        case NoSource:        return QObject::tr("no source");
        }
        Q_UNREACHABLE();
    }

    TorrentTrackerInfo(const QString &_url) : url(_url) {}

    QString url;
    QString trackerId; // '&trackerid=' argument passed to the tracker

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
        }
        Q_UNREACHABLE();
    }

    /*! C string for printf() */
    const char* torrentState_c_str() const
    {
        switch (state) {
        case TorrentInfo::stopped                : return QLatin1String("Stopped").data();
        case TorrentInfo::checking_files         : return QLatin1String("Checking files").data();
        case TorrentInfo::downloading_metadata   : return QLatin1String("Downloading metadata").data();
        case TorrentInfo::downloading            : return QLatin1String("Downloaded").data();
        case TorrentInfo::finished               : return QLatin1String("Finished").data();
        case TorrentInfo::seeding                : return QLatin1String("Seeding").data();
        case TorrentInfo::allocating             : return QLatin1String("Allocating").data();
        case TorrentInfo::checking_resume_data   : return QLatin1String("Checking resume data").data();
        }
        Q_UNREACHABLE();
    }

    TorrentError error;

    TorrentState state{stopped};

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
    TorrentNodeInfo() = default;
    explicit TorrentNodeInfo(const QString &_host, int _port)
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

    QBitArray unfinishedPieces;
    QBitArray downloadedPieces;
    QBitArray verifiedPieces; // seed mode only

    QDateTime lastTimeDownload;
    QDateTime lastTimeUpload;

};

/******************************************************************************
 ******************************************************************************/
using UniqueId = QString;

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

/* Enable the type to be used with QVariant. */
Q_DECLARE_METATYPE(TorrentData)
Q_DECLARE_METATYPE(TorrentStatus)

#endif // CORE_TORRENT_MESSAGE_H
