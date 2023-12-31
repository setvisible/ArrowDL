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

#include "torrentmessage.h"

template<typename Enum>
static inline void _q_set_flag(QFlags<Enum> *f, Enum flag, bool on = true)
{
    (*f).setFlag(flag, on);
}

using namespace Qt::Literals::StringLiterals;

static const QString S_GLASSES = QString::fromUtf8("\xf0\x9f\x91\x93");          // 👓
static const QString S_SMILE = QString::fromUtf8("\xF0\x9F\x98\x83");            // 😃
static const QString S_SAD = QString::fromUtf8("\xf0\x9f\x98\x9e ");             // 😞
static const QString S_LOVE = QString::fromUtf8("\xf0\x9f\x92\x8c");             // 💌
static const QString S_BOMB = QString::fromUtf8("\xf0\x9f\x92\xa3");             // 💣
static const QString S_THUMB_UP = QString::fromUtf8("\xf0\x9f\x91\x8d");         // 👍
static const QString S_RED_FLAG = QString::fromUtf8("\xf0\x9f\x9a\xa9");         // 🚩
static const QString S_HAND_SHAKE = QString::fromUtf8("\xf0\x9f\xa4\x9d");       // 🤝
static const QString S_COLLISION = QString::fromUtf8("\xf0\x9f\x92\xa5");        // 💥
static const QString S_UPLOAD = QString::fromUtf8("\xf0\x9f\x93\xa4");           // 📤
static const QString S_DICE_GAME = QString::fromUtf8("\xf0\x9f\x8e\xb2");        // 🎲
static const QString S_ELECTRIC_PLUG = QString::fromUtf8("\xf0\x9f\x94\x8c");    // 🔌
static const QString S_LIGHT_BULB = QString::fromUtf8("\xf0\x9f\x92\xa1");       // 💡
static const QString S_BANANA = QString::fromUtf8("\xf0\x9f\x8d\x8c");           // 🍌
static const QString S_SPEECH = QString::fromUtf8("\xf0\x9f\x92\xac");           // 💬
static const QString S_DOG_FACE = QString::fromUtf8("\xf0\x9f\x90\xb6");         // 🐶
static const QString S_RECEIPT = QString::fromUtf8("\xf0\x9f\xa7\xbe");          // 🧾
static const QString S_HOLE = QString::fromUtf8("\xf0\x9f\x95\xb3\xef\xb8\x8f"); // 🕳️
static const QString S_KEY = QString::fromUtf8("\xf0\x9f\x94\x91");              // 🔑
static const QString S_LOCKED = QString::fromUtf8("\xf0\x9f\x94\x92");           // 🔒
static const QString S_UNLOCKED = QString::fromUtf8("\xf0\x9f\x94\x93");         // 🔓

/******************************************************************************
 ******************************************************************************/
TorrentError::TorrentError(Type _type, int _fileIndex)
    : type(_type)
    , fileIndex(_fileIndex)
{}

/******************************************************************************
 ******************************************************************************/
EndPoint::EndPoint(const QString &address, int port)
    : m_ip(QHostAddress(address))
    , m_port(port)
{

}

EndPoint::EndPoint(const QString &addressAndPort)
{
    setAddressAndPort(addressAndPort);
}


QHostAddress EndPoint::ip() const
{
    return m_ip;
}
QString EndPoint::ipToString() const
{
    return m_ip.toString();
}

int EndPoint::port() const
{
    return m_port;
}

void EndPoint::setAddressAndPort(const QString &addressAndPort)
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

QString EndPoint::toString() const
{
    return QString("%0:%1").arg(m_ip.toString(), QString::number(m_port));
}

QString EndPoint::sortableIp() const
{
    // Used to sort the ip addresses.
    // Make sure that 2.0.0.0 follows 1.0.0.0 instead of 102.0.0.0
    auto ipv6 = m_ip.toIPv6Address();
    QString ret;
    for (int i = 0; i < 16; ++i) {
        const quint8 f = ipv6[i]; // 0 - 255
        ret.append(QString::number(f).rightJustified(3, QLatin1Char('0')));
    }
    return ret;
}

/******************************************************************************
 ******************************************************************************/
TorrentFileMetaInfo::TorrentFileMetaInfo(qsizetype _bytesTotal,
                                         qsizetype _bytesOffset,
                                         quint32 _crc32FilePathHash,
                                         const QString &_fileName,
                                         const QString &_filePath)
    : bytesTotal(_bytesTotal)
    , bytesOffset(_bytesOffset)
    , crc32FilePathHash(_crc32FilePathHash)
    , fileName(_fileName)
    , filePath(_filePath)
{
}

void TorrentFileMetaInfo::setFlag(Flag flag, bool on)
{
    _q_set_flag<TorrentFileMetaInfo::Flag>(&flags, flag, on);
}

/******************************************************************************
 ******************************************************************************/
QString TorrentFileMetaInfo::shortFilePath() const
{
    // Remove the name of the torrent file from the path to make the path shorter
    int i = filePath.indexOf(QChar('\\'));
    if (i < 0)
        i = filePath.lastIndexOf(QChar('/'));
    if (i >= 0)
        i++;
    return filePath.mid(i);
}

/******************************************************************************
 ******************************************************************************/
QString TorrentFileInfo::priorityString() const
{
    switch (priority) {
    case Ignore:    return QObject::tr("ignore");
    case Low:       return QObject::tr("low");
    case High:      return QObject::tr("high");
    case Normal:    return QObject::tr("normal");
    }
    Q_UNREACHABLE();
}

/******************************************************************************
 ******************************************************************************/
TorrentPeerInfo::TorrentPeerInfo(const EndPoint &_endpoint, const QString &_userAgent)
    : endpoint(_endpoint)
    , userAgent(_userAgent)
{
}

QString TorrentPeerInfo::flagUnicodeSymbol(Flag flag)
{
    switch (flag) {
    case Interesting          : return S_GLASSES;
    case Choked               : return S_BOMB;
    case RemoteInterested     : return S_LOVE;
    case RemoteChoked         : return S_RED_FLAG;
    case SupportsExtensions   : return S_RECEIPT;
    case LocalConnection      : return S_SMILE;
    case Handshake            : return S_HAND_SHAKE;
    case Connecting           : return S_THUMB_UP;
        //case Queued
    case OnParole             : return S_SPEECH;
    case Seed                 : return S_DOG_FACE;
    case OptimisticUnchoke    : return S_SAD;
    case Snubbed              : return S_COLLISION;
    case UploadOnly           : return S_UPLOAD;
    case Endgame_Mode         : return S_DICE_GAME;
    case Holepunched          : return S_HOLE;
    case I2pSocket            : return S_ELECTRIC_PLUG;
    case UtpSocket            : return S_LIGHT_BULB;
    case SslSocket            : return S_BANANA;
    case Rc4Encrypted         : return S_KEY;
    case Plaintextencrypted   : return S_LOCKED;
    }
    Q_UNREACHABLE();
}

QString TorrentPeerInfo::sourceFlagUnicodeSymbol(SourceFlag flag)
{
    switch (flag) {
    case FromTracker                : return S_BANANA;
    case FromDHT                    : return S_BOMB;
    case FromPeerExchange           : return S_LOVE;
    case FromLocalServiceDiscovery  : return S_RED_FLAG;
    case FromFastResumeData         : return S_DICE_GAME;
    case FromPeerIncomingData       : return S_HAND_SHAKE;
    }
    Q_UNREACHABLE();
}

QString TorrentPeerInfo::flagComment(Flag flag)
{
    switch (flag) {
    case Interesting          : return "We are interested in pieces from this peer."_L1;
    case Choked               : return "We have choked this peer."_L1;
    case RemoteInterested     : return "The peer is interested in us"_L1;
    case RemoteChoked         : return "The peer has choked us."_L1;
    case SupportsExtensions   : return "The peer supports the extension protocol."_L1;
    case LocalConnection      : return "The peer connection was opened by us."_L1;
    case Handshake            : return "The handshake is done."_L1;
    case Connecting           : return "The connection is in a half-open state."_L1;
    //case Queued             : return "The connection is currently queued for a connection attempt."_L1;
    case OnParole             : return "The peer has failed the hash check."_L1;
    case Seed                 : return "The peer is a seed (it has all the pieces)."_L1;
    case OptimisticUnchoke    : return "The peer is subject to an optimistic unchoke."_L1;
    case Snubbed              : return "The peer has recently failed to send a block."_L1;
    case UploadOnly           : return "The peer told us that it will not downloading anything more."_L1;
    case Endgame_Mode         : return "All pieces this peer has were already requested from other peers."_L1;
    case Holepunched          : return "The peer is in holepunch mode (NAT holepunch mechanism)."_L1;
    case I2pSocket            : return "The socket is running on I2P transport."_L1;
    case UtpSocket            : return "The socket is a uTP socket."_L1;
    case SslSocket            : return "The socket is running on SSL (TLS) channel."_L1;
    case Rc4Encrypted         : return "The connection is obfuscated with RC4."_L1;
    case Plaintextencrypted   : return "The connection handshake was obfuscated with a Diffie-Hellman exchange."_L1;
    }
    Q_UNREACHABLE();
}

QString TorrentPeerInfo::sourceFlagComment(SourceFlag flag)
{
    switch (flag) {
    case FromTracker               : return "The peer was received from the tracker."_L1;
    case FromDHT                   : return "The peer was received from the kademlia DHT."_L1;
    case FromPeerExchange          : return "The peer was received from the peer exchange extension."_L1;
    case FromLocalServiceDiscovery : return "The peer was received from the local service discovery (The peer is on the local network)."_L1;
    case FromFastResumeData        : return "The peer was added from the fast resume data."_L1;
    case FromPeerIncomingData      : return "We received an incoming connection from this peer."_L1;
    }
    Q_UNREACHABLE();
}

QString TorrentPeerInfo::flagString() const
{
    QString ret;
    for (int i = Interesting; i < Plaintextencrypted; ++i) {
        if (flags.testFlag(Flag(i))) {
            ret += TorrentPeerInfo::flagUnicodeSymbol(Flag(i));
        }
    }
    return ret;
}

QString TorrentPeerInfo::sourceFlagString() const
{
    QString ret;
    for (int i = FromTracker; i < FromPeerIncomingData; ++i) {
        if (sourceFlags.testFlag(SourceFlag(i))) {
            ret += TorrentPeerInfo::sourceFlagUnicodeSymbol(SourceFlag(i));
        }
    }
    return ret;
}

QString TorrentPeerInfo::flagTooltip()
{
    QString ret = "Flags:"_L1;
    for (int i = Interesting; i < Plaintextencrypted; ++i) {
        ret += QString("\n- %0 : %1").arg(
            TorrentPeerInfo::flagUnicodeSymbol(Flag(i)),
            TorrentPeerInfo::flagComment(Flag(i)));
    }
    return ret;
}

QString TorrentPeerInfo::sourceFlagTooltip()
{
    QString ret = "Source Flags:"_L1;
    for (int i = FromTracker; i < FromPeerIncomingData; ++i) {
        ret += QString("\n- %0 : %1").arg(
            TorrentPeerInfo::sourceFlagUnicodeSymbol(SourceFlag(i)),
            TorrentPeerInfo::sourceFlagComment(SourceFlag(i)));
    }
    return ret;
}

void TorrentPeerInfo::setFlag(Flag flag, bool on)
{
    _q_set_flag<TorrentPeerInfo::Flag>(&flags, flag, on);
}

void TorrentPeerInfo::setSourceFlag(SourceFlag flag, bool on)
{
    _q_set_flag<TorrentPeerInfo::SourceFlag>(&sourceFlags, flag, on);
}

/******************************************************************************
 ******************************************************************************/
TorrentTrackerInfo::TorrentTrackerInfo(
    const QString &_url,
    int _tier,
    TorrentTrackerInfo::Source _source)
    : url(_url)
    , tier(_tier)
    , source(_source)
{
}

QString TorrentTrackerInfo::sourceString() const
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

/******************************************************************************
 ******************************************************************************/
QString TorrentInfo::torrentStateString() const
{
    switch (state) {
    case TorrentInfo::stopped                : return QObject::tr("Stopped");
    case TorrentInfo::checking_files         : return QObject::tr("Checking Files...");
    case TorrentInfo::downloading_metadata   : return QObject::tr("Downloading Metadata...");
    case TorrentInfo::downloading            : return QObject::tr("Downloading...");
    case TorrentInfo::finished               : return QObject::tr("Finished");
    case TorrentInfo::seeding                : return QObject::tr("Seeding...");
    case TorrentInfo::checking_resume_data   : return QObject::tr("Checking Resume Data...");
    }
    Q_UNREACHABLE();
}

/*! C string for printf() */
const char* TorrentInfo::torrentState_c_str() const
{
    switch (state) {
    case TorrentInfo::stopped                : return QLatin1String("Stopped").data();
    case TorrentInfo::checking_files         : return QLatin1String("Checking files").data();
    case TorrentInfo::downloading_metadata   : return QLatin1String("Downloading metadata").data();
    case TorrentInfo::downloading            : return QLatin1String("Downloaded").data();
    case TorrentInfo::finished               : return QLatin1String("Finished").data();
    case TorrentInfo::seeding                : return QLatin1String("Seeding").data();
    case TorrentInfo::checking_resume_data   : return QLatin1String("Checking resume data").data();
    }
    Q_UNREACHABLE();
}

/******************************************************************************
 ******************************************************************************/
TorrentNodeInfo::TorrentNodeInfo(const QString &_host, int _port)
    : host(_host)
    , port(_port)
{
}
