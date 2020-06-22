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

#include "torrent.h"

#include <Core/Format>
#include <Core/TorrentMessage>

#include <QtCore/QDebug>
#include <QtCore/QtMath>
#include <QtCore/QBitArray>
#ifdef QT_TESTLIB_LIB
#  include <QtTest/QTest>
#endif


Torrent::Torrent(QObject *parent) : QObject(parent)
{
    m_fileModel = new TorrentFileTableModel(this);
    m_peerModel = new TorrentPeerTableModel(this);
    m_trackerModel = new TorrentTrackerTableModel(this);

    clear();
}

Torrent::~Torrent()
{

}


/******************************************************************************
 ******************************************************************************/
void Torrent::clear()
{
}

bool Torrent::isEmpty()
{
    return false;
}

/******************************************************************************
 ******************************************************************************/
QString Torrent::url() const
{
    return m_url;
}

void Torrent::setUrl(const QString &url)
{
    m_url = url;
}

/******************************************************************************
 ******************************************************************************/
QString Torrent::localFullFileName() const
{
    return m_localTorrentFileName;
}

void Torrent::setLocalFullFileName(const QString &filename)
{
    m_localTorrentFileName = filename;
}

/******************************************************************************
 ******************************************************************************/
QString Torrent::localFilePath() const
{
    return m_outputPath;
}

void Torrent::setLocalFilePath(const QString &outputPath)
{
    m_outputPath = outputPath;
}

/******************************************************************************
 ******************************************************************************/
QString Torrent::status() const
{
    return m_info.torrentStateString();
}

/******************************************************************************
 ******************************************************************************/
TorrentMetaInfo Torrent::metaInfo() const
{
    return m_metaInfo;
}

void Torrent::setMetaInfo(TorrentMetaInfo metaInfo)
{
    m_metaInfo = metaInfo;
    m_fileModel->refreshMetaData(m_metaInfo.initialMetaInfo.files);
    m_trackerModel->refreshData(m_metaInfo.initialMetaInfo.trackers);

    // requires a GUI update signal, because metaInfo can change
    // even if the downloadItem is Paused or Stopped
    emit changed();
}


/******************************************************************************
 ******************************************************************************/
TorrentInfo Torrent::info() const
{
    return m_info;
}

void Torrent::setInfo(TorrentInfo info, bool mustRefreshMetaInfo)
{
    m_info = info;
    if (mustRefreshMetaInfo) {
        m_fileModel->refreshMetaData(m_metaInfo.initialMetaInfo.files);
        m_trackerModel->refreshData(m_metaInfo.initialMetaInfo.trackers);
    }
}

/******************************************************************************
 ******************************************************************************/
TorrentHandleInfo Torrent::detail() const
{
    return m_detail;
}

void Torrent::setDetail(TorrentHandleInfo detail, bool mustRefreshMetaInfo)
{
    m_detail = detail;
    if (mustRefreshMetaInfo) {
        m_fileModel->refreshMetaData(m_metaInfo.initialMetaInfo.files);
        m_trackerModel->refreshData(m_metaInfo.initialMetaInfo.trackers);
    }
    m_fileModel->refreshData(m_detail.files);
    m_peerModel->refreshData(m_detail.peers);
    m_trackerModel->refreshData(m_detail.trackers);

    // requires a GUI update signal, because detail can change
    // even if the downloadItem is Paused or Stopped
    emit changed();
}

/******************************************************************************
 ******************************************************************************/
void Torrent::setError(TorrentError::Type errorType, const QString &message)
{
    m_metaInfo.error.type = errorType;
    m_metaInfo.error.message = message;
}

/******************************************************************************
 ******************************************************************************/
int Torrent::fileCount() const
{
    return m_detail.files.count();
}

/******************************************************************************
 ******************************************************************************/
TorrentFileInfo::Priority Torrent::filePriority(int index) const
{
    if (index < 0 || index > m_detail.files.count()) {
        return TorrentFileInfo::Normal;
    }
    return m_detail.files.at(index).priority;
}

void Torrent::setFilePriority(int index, TorrentFileInfo::Priority priority)
{
    if (index < 0 || index > m_detail.files.count()) {
        return;
    }
    m_detail.files[index].priority = priority;
    m_fileModel->refreshData(m_detail.files); // Synchronize
    emit changed();
}

QString Torrent::preferredFilePriorities() const
{
    QString code;
    for (int fi = 0; fi < fileCount(); ++fi) {
        auto priority = filePriority(fi);
        switch (priority) {
        case TorrentFileInfo::Ignore: code.append("-"); break;
        case TorrentFileInfo::Low:    code.append("L"); break;
        case TorrentFileInfo::Normal: code.append("N"); break;
        case TorrentFileInfo::High:   code.append("H"); break;
        default: Q_UNREACHABLE(); break;
        }
    }
    return code;
}

void Torrent::setPreferredFilePriorities(const QString &priorities)
{
    for (int fi = 0; fi < fileCount(); ++fi) {
        if (fi > priorities.length() - 1) {
            return;
        }
        auto priority = TorrentFileInfo::Normal;
        switch (priorities.at(fi).toLatin1()) {
        case '-': priority = TorrentFileInfo::Ignore; break;
        case 'L': priority = TorrentFileInfo::Low; break;
        case 'N': priority = TorrentFileInfo::Normal; break;
        case 'H': priority = TorrentFileInfo::High; break;
        default: break;
        }
        setFilePriority(fi, priority);
    }
}

/******************************************************************************
 ******************************************************************************/
void Torrent::addPeer(const QString &input)
{
    qDebug() << Q_FUNC_INFO;

    emit changed();
}


/******************************************************************************
 ******************************************************************************/
int Torrent::trackerCount() const
{
    return m_detail.trackers.size();
}

void Torrent::addTracker(const QString &url)
{
    m_detail.trackers << TorrentTrackerInfo(url);
    emit changed();
}

void Torrent::removeTrackerAt(int i)
{
    if (i >= 0 && i < m_detail.trackers.size()) {
        m_detail.trackers.removeAt(i);
        emit changed();
    }
}

/******************************************************************************
 ******************************************************************************/
QAbstractTableModel* Torrent::fileModel() const
{
    return m_fileModel;
}

QAbstractTableModel* Torrent::peerModel() const
{
    return m_peerModel;
}

QAbstractTableModel* Torrent::trackerModel() const
{
    return m_trackerModel;
}

/******************************************************************************
 ******************************************************************************/
int Torrent::progress() const
{
    qint64 bytesReceived = 0;
    qint64 bytesTotal = 0;
    switch (static_cast<int>(m_info.state)) {
    case TorrentInfo::stopped:
    case TorrentInfo::checking_files:
    case TorrentInfo::downloading_metadata:
        return 0;
    case TorrentInfo::downloading:
        bytesReceived = m_info.bytesReceived;
        bytesTotal = m_info.bytesTotal;
        break;
    case TorrentInfo::finished:
        bytesReceived = m_metaInfo.initialMetaInfo.bytesTotal;
        bytesTotal = m_metaInfo.initialMetaInfo.bytesTotal;
        break;
    case TorrentInfo::seeding:
    case TorrentInfo::allocating:
    case TorrentInfo::checking_resume_data:
        return 100;
    default:
        Q_UNREACHABLE();
        break;
    }
    if (bytesTotal > 0) {
        return qMin(qFloor(100.0 * bytesReceived / bytesTotal), 100);
    }
    return -1; // Undefined
}

/******************************************************************************
 ******************************************************************************/
void Torrent::retranslateUi()
{
    m_fileModel->retranslateUi();
    m_peerModel->retranslateUi();
    m_trackerModel->retranslateUi();
}

/******************************************************************************
 ******************************************************************************/
AbstractTorrentTableModel::AbstractTorrentTableModel(Torrent *parent)
    : QAbstractTableModel(parent)
{
}

int AbstractTorrentTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_headers.count();
}

QVariant AbstractTorrentTableModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section >= 0 && section < m_headers.count()) {
            return m_headers.at(section);
        } else {
            return QString();
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

/******************************************************************************
 ******************************************************************************/
TorrentFileTableModel::TorrentFileTableModel(Torrent *parent)
    : AbstractTorrentTableModel(parent)
{
    retranslateUi();
}

void TorrentFileTableModel::retranslateUi()
{    
    m_headers = QStringList()
            << tr("Name")
            << tr("Path")
            << tr("Size")
            << tr("Done")
            << tr("Percent")
            << tr("First Piece")
            << tr("# Pieces")
            << tr("Pieces")
            << tr("Priority")
            << tr("Modification date")
            << tr("SHA-1")
            << tr("CRC-32");
}

int TorrentFileTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_filesMeta.count();
}

int TorrentFileTableModel::percent(const TorrentFileMetaInfo &mi,
                                   const TorrentFileInfo &ti) const
{
    const qint64 percentageDownloaded
            = mi.bytesTotal != 0 ? 100 * ti.bytesReceived / mi.bytesTotal : 0;
    const int percent = static_cast<int>(percentageDownloaded);
    return percent;
}

int TorrentFileTableModel::firstPieceIndex(const TorrentFileMetaInfo &mi) const
{
    if (m_pieceByteSize != 0) {
        return qCeil(mi.bytesOffset / m_pieceByteSize);
    }
    return 0;
}

int TorrentFileTableModel::lastPieceIndex(const TorrentFileMetaInfo &mi) const
{
    if (m_pieceByteSize != 0) {
        return qCeil((mi.bytesOffset + mi.bytesTotal) / m_pieceByteSize);
    }
    return 0;
}

int TorrentFileTableModel::startBlockInFirstPiece(const TorrentFileMetaInfo &mi) const
{
    return int(mi.bytesOffset % m_pieceByteSize);
}

int TorrentFileTableModel::pieceCount(const TorrentFileMetaInfo &mi) const
{
    return 1 + lastPieceIndex(mi) - firstPieceIndex(mi);
}

QBitArray TorrentFileTableModel::pieceSegments(const TorrentFileMetaInfo &mi) const
{
    if (m_downloadedPieces.isEmpty()) {
        return QBitArray();
    }
    auto offset = firstPieceIndex(mi);
    auto size = pieceCount(mi);
    if (offset < 0 || size < 0 || m_downloadedPieces.size() < offset + size) {
        return QBitArray();
    }
    QBitArray ba(size, false);
    for (int i = 0; i < size; ++i) {
        if (m_downloadedPieces.testBit(offset + i)) {
            ba.setBit(i);
        }
    }
    return ba;
}

QVariant TorrentFileTableModel::data(const QModelIndex &item, int role) const
{
    if (!item.isValid()) {
        return QVariant();
    }
    if (item.row() >= rowCount() || item.row() < 0) {
        return QVariant();
    }
    TorrentFileMetaInfo mi = m_filesMeta.at(item.row());
    TorrentFileInfo ti;
    if (item.row() < m_files.count()) {
        ti = m_files.at(item.row());
    }
    if (role == Qt::TextAlignmentRole) {
        switch (item.column()) {
        case  0:
        case  1:
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        case  2:
        case  3:
        case  4:
        case  5:
        case  6:
            return int(Qt::AlignRight | Qt::AlignVCenter);
        case  7:
        case  8:
        case  9:
        case 10:
        case 11:
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        default:
            break;
        }

    } else if (role == ProgressRole) { /*&& item.column() == 7*/
        return percent(mi, ti);

    } else if (role == SegmentRole) {
        return pieceSegments(mi);

    } else if (role == Qt::DisplayRole) {
        switch (item.column()) {
        case  0: return mi.fileName;
        case  1: return mi.shortFilePath();
        case  2: return Format::fileSizeToString(mi.bytesTotal);
        case  3: return Format::fileSizeToString(ti.bytesReceived);
        case  4: return QString("%0%").arg(QString::number(percent(mi, ti)));
        case  5: return firstPieceIndex(mi);
        case  6: return pieceCount(mi);
        case  7: return QVariant(); // Progress bar
        case  8: return ti.priorityString();
        case  9: return mi.modifiedTime;
        case 10: return mi.hash;
        case 11: return mi.crc32FilePathHash;
        default:
            break;
        }
    }
    return QVariant();
}

void TorrentFileTableModel::refreshMetaData(QList<TorrentFileMetaInfo> files)
{
    beginResetModel();
    m_filesMeta = files;
    m_files.clear();

    Torrent *torrent = static_cast<Torrent*>(parent());
    if (torrent) {
        m_pieceByteSize = torrent->metaInfo().initialMetaInfo.pieceByteSize;
    }

    endResetModel();
}

void TorrentFileTableModel::refreshData(QList<TorrentFileInfo> files)
{
    m_files = files;
    Torrent *torrent = static_cast<Torrent*>(parent());
    if (torrent) {
        m_downloadedPieces = torrent->info().downloadedPieces;
    }
    emit dataChanged(index(0,0), index(rowCount(), columnCount()), {Qt::DisplayRole});
}

/******************************************************************************
 ******************************************************************************/
TorrentPeerTableModel::TorrentPeerTableModel(Torrent *parent)
    : AbstractTorrentTableModel(parent)
{
    retranslateUi();
}

void TorrentPeerTableModel::retranslateUi()
{
    m_headers = QStringList()
            << tr("IP")
            << tr("Port")
            << tr("Client")
            << tr("Downloaded")
            << tr("Uploaded")
            << tr("Request Time")
            << tr("Active Time")
            << tr("Queue Time")
            << tr("Flags")
            << tr("Source Flags");
}

int TorrentPeerTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_peers.count();
}

QVariant TorrentPeerTableModel::data(const QModelIndex &item, int role) const
{
    if (!item.isValid()) {
        return QVariant();
    }
    if (item.row() >= rowCount() || item.row() < 0) {
        return QVariant();
    }
    if (role == Qt::TextAlignmentRole) {
        switch (item.column()) {
        case  0:
        case  1:
        case  2:
            return Qt::AlignLeft;
        case  3:
        case  4:
            return Qt::AlignRight;
        case  5:
        case  6:
        case  7:
        case  8:
        case  9:
            return Qt::AlignLeft;
        default:
            break;
        }

    } else if (role == Qt::DisplayRole) {
        const TorrentPeerInfo peer = m_peers.at(item.row());

        switch (item.column()) {
        case 0: return peer.endpoint.ip;
        case 1: return peer.endpoint.port;
        case 2: return peer.client;
        case 3: return Format::fileSizeToString(peer.bytesDownloaded);
        case 4: return Format::fileSizeToString(peer.bytesUploaded);
        case 5: return Format::timeToString(peer.lastTimeRequested);
        case 6: return Format::timeToString(peer.lastTimeActive);
        case 7: return Format::timeToString(peer.timeDownloadQueue);
        case 8: return peer.flagString();
        case 9: return peer.sourceFlagString();
        default:
            break;
        }
    }
    return QVariant();
}

void TorrentPeerTableModel::refreshData(QList<TorrentPeerInfo> peers)
{    
    /// \todo  trier les peers suivant leur status: peer_info::connecting, peer_info::handshake...

    if (peers.isEmpty()) {
        return;
    }

    QModelIndex parent = QModelIndex(); // root (empty)

    QList<TorrentPeerInfo> newItems;

    for (int i = 0, count = peers.count(); i < count; ++i) {
        auto newItem = peers.at(i);
        bool replaced = false;
        for (int j = 0, count2 = m_peers.count(); j < count2; ++j) {
            auto item = m_peers.at(j);

            // Try update
            if (item.endpoint == newItem.endpoint) {
                m_peers.removeAt(j);
                m_peers.insert(j, newItem);
                emit dataChanged(index(j, 0), index(j, columnCount()), {Qt::DisplayRole});
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            newItems.append(newItem);
        }
    }
    // Otherwise append
    if (!newItems.isEmpty()) {
        const int first = m_peers.count();
        const int last = first + newItems.count() - 1;
        beginInsertRows(parent, first, last);
        m_peers.append(newItems);
        endInsertRows();
    }

}

/******************************************************************************
 ******************************************************************************/
TorrentTrackerTableModel::TorrentTrackerTableModel(Torrent *parent)
    : AbstractTorrentTableModel(parent)
{
    retranslateUi();
}

void TorrentTrackerTableModel::retranslateUi()
{
    m_headers = QStringList()
            << tr("Url")
            << tr("Id")
            << tr("Number of listened sockets (endpoints)")
            << tr("Tier this tracker belongs to")
            << tr("Max number of failures")
            << tr("Source")
            << tr("Verified?");
}

int TorrentTrackerTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_trackers.count();
}

QVariant TorrentTrackerTableModel::data(const QModelIndex &item, int role) const
{
    if (!item.isValid()) {
        return QVariant();
    }
    if (item.row() >= rowCount() || item.row() < 0) {
        return QVariant();
    }
    if (role == Qt::TextAlignmentRole) {
        switch (item.column()) {
        case  0:
        case  1:
            return Qt::AlignLeft;
        case  2:
        case  3:
        case  4:
            return Qt::AlignRight;
        case  5:
        case  6:
            return Qt::AlignLeft;
        default:
            break;
        }

    } else if (role == Qt::DisplayRole) {
        TorrentTrackerInfo tracker = m_trackers.at(item.row());
        switch (item.column()) {
        case 0: return tracker.url;
        case 1: return tracker.trackerId;
        case 2: return tracker.endpoints.size();
        case 3: return tracker.tier;
        case 4: return tracker.failLimit != 0
                    ? QString::number(tracker.failLimit) : Format::infinity();
        case 5: return tracker.sourceString();
        case 6: return tracker.isVerified ? tr("verified") : tr("not verified");
        default:
            break;
        }
    }
    return QVariant();
}

void TorrentTrackerTableModel::refreshData(QList<TorrentTrackerInfo> trackers)
{
    if (trackers.isEmpty()) {
        return;
    }

    QModelIndex parent = QModelIndex(); // empty is always root

    QList<TorrentTrackerInfo> newItems;

    for (int i = 0, count = trackers.count(); i < count; ++i) {
        auto newItem = trackers.at(i);
        bool replaced = false;
        for (int j = 0, count2 = m_trackers.count(); j < count2; ++j) {
            auto item = m_trackers.at(j);

            // Try update
            if (item.url == newItem.url) {
                m_trackers.removeAt(j);
                m_trackers.insert(j, newItem);
                emit dataChanged(index(j, 0), index(j, columnCount()), {Qt::DisplayRole});
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            newItems.append(newItem);
        }
    }
    // Otherwise append
    if (!newItems.isEmpty()) {
        const int first = m_trackers.count();
        const int last = first + newItems.count() - 1;
        beginInsertRows(parent, first, last);
        m_trackers.append(newItems);
        endInsertRows();
    }
}
