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

#include "torrent.h"

#include <Constants>
#include <Core/Format>
#include <Core/TorrentMessage>

#include <QtCore/QDebug>
#include <QtCore/QtMath>
#include <QtCore/QBitArray>
#ifdef QT_TESTLIB_LIB
#  include <QtTest/QTest>
#endif


Torrent::Torrent(QObject *parent) : QObject(parent),
    m_fileModel(new TorrentFileTableModel(this)),
    m_peerModel(new TorrentPeerTableModel(this)),
    m_trackerModel(new TorrentTrackerTableModel(this))
{
    clear();
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

void Torrent::setMetaInfo(const TorrentMetaInfo &metaInfo)
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

void Torrent::setInfo(const TorrentInfo &info, bool mustRefreshMetaInfo)
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

void Torrent::setDetail(const TorrentHandleInfo &detail, bool mustRefreshMetaInfo)
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
qsizetype Torrent::fileCount() const
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
    for (auto fi = 0; fi < fileCount(); ++fi) {
        auto priority = filePriority(fi);
        switch (priority) {
        case TorrentFileInfo::Ignore: code.append("-"); break;
        case TorrentFileInfo::Low:    code.append("L"); break;
        case TorrentFileInfo::Normal: code.append("N"); break;
        case TorrentFileInfo::High:   code.append("H"); break;
        }
    }
    return code;
}

void Torrent::setPreferredFilePriorities(const QString &priorities)
{
    bool hasChanged = false;
    for (auto fi = 0; fi < fileCount(); ++fi) {
        if (fi < priorities.length()) {
            auto priority = TorrentFileInfo::Normal;
            switch (priorities.at(fi).toLatin1()) {
            case '-': priority = TorrentFileInfo::Ignore; break;
            case 'L': priority = TorrentFileInfo::Low; break;
            case 'N': priority = TorrentFileInfo::Normal; break;
            case 'H': priority = TorrentFileInfo::High; break;
            default: break;
            }
            QSignalBlocker blocker(this);
            setFilePriority(fi, priority);
            hasChanged = true;
        }
    }
    if (hasChanged) {
        emit changed();
    }
}

/******************************************************************************
 ******************************************************************************/
void Torrent::addPeer(const QString &/*input*/)
{
    qWarning("todo: addPeer() not implemented yet.");
    emit changed();
}

void Torrent::removeUnconnectedPeers()
{
    m_peerModel->removeUnconnectedPeers();
    emit changed();
}


/******************************************************************************
 ******************************************************************************/
qsizetype Torrent::trackerCount() const
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
    case TorrentInfo::checking_resume_data:
        return 100;
    default:
        Q_UNREACHABLE();
        break;
    }
    if (bytesTotal > 0) {
        return qMin(qFloor(100 * static_cast<qreal>(bytesReceived) / static_cast<qreal>(bytesTotal)), 100);
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
SortFilterProxyModel::SortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(AbstractTorrentTableModel::SortRole);
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

QVariant AbstractTorrentTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section >= 0 && section < m_headers.count()) {
            return m_headers.at(section);
        }
        return {};
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
            << tr("#")
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
    return parent.isValid() ? 0 : static_cast<int>(m_filesMeta.count());
}

/// \todo move to torrentmessage?
int TorrentFileTableModel::percent(const TorrentFileMetaInfo &mi, const TorrentFileInfo &ti) const
{
    if (mi.bytesTotal != 0) {
        return qFloor(100 * static_cast<qreal>(ti.bytesReceived) / static_cast<qreal>(mi.bytesTotal));
    }
    return 0;
}

qint64 TorrentFileTableModel::firstPieceIndex(const TorrentFileMetaInfo &mi) const
{
    if (m_pieceByteSize != 0) {
        return static_cast<qint64>(static_cast<qreal>(mi.bytesOffset) / static_cast<qreal>(m_pieceByteSize));
    }
    return 0;
}

qint64 TorrentFileTableModel::lastPieceIndex(const TorrentFileMetaInfo &mi) const
{
    if (m_pieceByteSize != 0) {
        return static_cast<qint64>(static_cast<qreal>(mi.bytesOffset + mi.bytesTotal) / static_cast<qreal>(m_pieceByteSize));
    }
    return 0;
}

qint64 TorrentFileTableModel::startBlockInFirstPiece(const TorrentFileMetaInfo &mi) const
{
    return static_cast<qint64>(mi.bytesOffset % m_pieceByteSize);
}

qint64 TorrentFileTableModel::pieceCount(const TorrentFileMetaInfo &mi) const
{
    return 1 + lastPieceIndex(mi) - firstPieceIndex(mi);
}

QBitArray TorrentFileTableModel::pieceSegments(const TorrentFileMetaInfo &mi) const
{
    if (m_downloadedPieces.isEmpty()) {
        return {};
    }
    auto offset = firstPieceIndex(mi);
    auto size = pieceCount(mi);
    if (offset < 0 || size < 0 || m_downloadedPieces.size() < offset + size) {
        return {};
    }
    QBitArray ba(size, false);
    for (auto i = 0; i < size; ++i) {
        if (m_downloadedPieces.testBit(offset + i)) {
            ba.setBit(i);
        }
    }
    return ba;
}

QVariant TorrentFileTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.row() >= rowCount() || index.row() < 0) {
        return {};
    }
    auto fileIndex = index.row();
    auto mi = m_filesMeta.at(fileIndex);
    TorrentFileInfo ti;
    if (fileIndex < m_files.count()) {
        ti = m_files.at(fileIndex);
    }
    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case  0:
            return int(Qt::AlignRight| Qt::AlignVCenter);
        case  1:
        case  2:
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        case  3:
        case  4:
        case  5:
        case  6:
        case  7:
            return int(Qt::AlignRight | Qt::AlignVCenter);
        case  8:
        case  9:
        case 10:
        case 11:
        case 12:
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        default:
            break;
        }

    } else if (role == ProgressRole) {
        return percent(mi, ti);

    } else if (role == SegmentRole) {
        return pieceSegments(mi);

    } else if (role == SortRole) {
        switch (index.column()) {
        case  0: return fileIndex;
        case  1: return mi.fileName.toLower();
        case  2: return mi.shortFilePath().toLower();
        case  3: return mi.bytesTotal;
        case  4: return ti.bytesReceived;
        case  5: return percent(mi, ti);
        case  6: return firstPieceIndex(mi);
        case  7: return pieceCount(mi);
        case  8: return percent(mi, ti);
        case  9: return ti.priority;
        case 10: return mi.modifiedTime;
        case 11: return mi.hash;
        case 12: return mi.crc32FilePathHash;
        default:
            break;
        }

    } else if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case  0: return fileIndex;
        case  1: return mi.fileName;
        case  2: return mi.shortFilePath();
        case  3: return Format::fileSizeToString(mi.bytesTotal);
        case  4: return Format::fileSizeToString(ti.bytesReceived);
        case  5: return QString("%0%").arg(QString::number(percent(mi, ti)));
        case  6: return firstPieceIndex(mi);
        case  7: return pieceCount(mi);
        case  8: return {}; // Progress bar
        case  9: return ti.priorityString();
        case 10: return mi.modifiedTime;
        case 11: return mi.hash;
        case 12: return mi.crc32FilePathHash;
        default:
            break;
        }

    } else if (role == Qt::ToolTipRole) {
        switch (index.column()) {
        case  0:
        case  1:
        case  2:
        case  3:
        case  4:
        case  5:
        case  6:return {};
        case  7:
        case  8: {
            auto done = QString::number(percent(mi, ti));
            auto total = QString::number(pieceCount(mi));
            return tr("%0% of %1 pieces").arg(done, total); // Progress bar
        }
        case  9:
        case 10:
        case 11:
        case 12: return {};
        default:
            break;
        }
    }
    return {};
}

void TorrentFileTableModel::refreshMetaData(const QList<TorrentFileMetaInfo> &files)
{
    beginResetModel();
    m_filesMeta = files;
    m_files.clear();

    auto torrent = dynamic_cast<Torrent*>(parent());
    if (torrent) {
        m_pieceByteSize = torrent->metaInfo().initialMetaInfo.pieceByteSize;
    }

    endResetModel();
}

void TorrentFileTableModel::refreshData(const QList<TorrentFileInfo> &files)
{
    m_files = files;
    auto torrent = dynamic_cast<Torrent*>(parent());
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
    m_peers.reserve(MAX_PEER_LIST_COUNT);
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
            << tr("Pieces")
            << tr("Request Time")
            << tr("Active Time")
            << tr("Queue Time")
            << tr("Flags")
            << tr("Source Flags");
}

int TorrentPeerTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_peers.count());
}

QVariant TorrentPeerTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.row() >= rowCount() || index.row() < 0) {
        return {};
    }
    auto peer = m_peers.at(index.row());
    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case  0:
        case  1:
        case  2:
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        case  3:
        case  4:
            return int(Qt::AlignRight | Qt::AlignVCenter);
        case  5:
        case  6:
        case  7:
        case  8:
        case  9:
        case 10:
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        default:
            break;
        }

    } else if (role == ProgressRole) {
        auto done = static_cast<qreal>(peer.availablePieces.count(true));
        auto total = static_cast<qreal>(peer.availablePieces.count());
        return total > 0 ? qMin(qCeil(100 * done / total), 100) : 0;

    } else if (role == SegmentRole) {
        return peer.availablePieces;

    } else if (role == ConnectRole) {
        return m_connectedPeers.contains(peer.endpoint);

    } else if (role == SortRole) {
        switch (index.column()) {
        case  0: return peer.endpoint.sortableIp();
        case  1: return peer.endpoint.port();
        case  2: return peer.userAgent.toLower();
        case  3: return peer.bytesDownloaded;
        case  4: return peer.bytesUploaded;
        case  5: return peer.availablePieces.count(true); // Progress bar
        case  6: return peer.lastTimeRequested;
        case  7: return peer.lastTimeActive;
        case  8: return peer.timeDownloadQueue;
        case  9: return peer.flagString();
        case 10: return peer.sourceFlagString();
        default:
            break;
        }

    } else if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case  0: return peer.endpoint.ip().toString();
        case  1: return peer.endpoint.port();
        case  2: return peer.userAgent;
        case  3: return Format::fileSizeToString(peer.bytesDownloaded);
        case  4: return Format::fileSizeToString(peer.bytesUploaded);
        case  5: return {}; // Progress bar
        case  6: return Format::timeToString(peer.lastTimeRequested);
        case  7: return Format::timeToString(peer.lastTimeActive);
        case  8: return Format::timeToString(peer.timeDownloadQueue);
        case  9: return peer.flagString();
        case 10: return peer.sourceFlagString();
        default:
            break;
        }

    } else if (role == Qt::ToolTipRole) {
        switch (index.column()) {
        case  0:
        case  1:
        case  2:
        case  3:
        case  4: return {};
        case  5: {
            auto done = QString::number(peer.availablePieces.count(true));
            auto total = QString::number(peer.availablePieces.count());
            return tr("%0 of %1 pieces").arg(done, total); // Progress bar
        }
        case  6:
        case  7:
        case  8: return {};
        case  9: return TorrentPeerInfo::flagTooltip();
        case 10: return TorrentPeerInfo::sourceFlagTooltip();
        default:
            break;
        }
    }
    return {};
}

void TorrentPeerTableModel::removeUnconnectedPeers()
{
    beginResetModel();
    QList<TorrentPeerInfo> peers;
    for (auto peer : m_peers) {
        if (m_connectedPeers.contains(peer.endpoint)) {
            peers.append(peer);
        }
    }
    m_peers = peers;
    endResetModel();
}

void TorrentPeerTableModel::refreshData(const QList<TorrentPeerInfo> &peers)
{
    if (peers.isEmpty()) {
        return;
    }
    m_connectedPeers.clear();
    QList<TorrentPeerInfo> newItems;

    for (auto newItem : peers) {
        m_connectedPeers.insert(newItem.endpoint);
        bool replaced = false;
        for (auto i = 0; i < m_peers.count(); ++i) {
            auto item = m_peers.at(i);

            // Try update
            if (item.endpoint == newItem.endpoint) {
                m_peers.replace(i, newItem);
                auto row = static_cast<int>(i);
                emit dataChanged(index(row, 0), index(row, columnCount()), {Qt::DisplayRole});
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            newItems.append(newItem);
        }
    }
    // Append remaining items
    appendRemainingSafely(newItems);
}

void TorrentPeerTableModel::appendRemainingSafely(const QList<TorrentPeerInfo> &newItems)
{
    if (newItems.isEmpty()) {
        return;
    }
    int ptr = 0;
    if (m_peers.count() < MAX_PEER_LIST_COUNT) {
        ptr = qMin(newItems.count(), MAX_PEER_LIST_COUNT - m_peers.count());

        auto first = m_peers.count();
        auto last = qMin(first + ptr - 1, MAX_PEER_LIST_COUNT - 1);
        beginInsertRows(QModelIndex(), first, last);
        m_peers.append(newItems.mid(0, ptr));
        endInsertRows();
    }
    if (ptr < newItems.count()) {
        for (auto i = m_peers.count() - 1; i >= 0; --i) {
            auto peer = m_peers.at(i);
            if (m_connectedPeers.contains(peer.endpoint)) {
                continue;
            }
            m_peers.replace(i, newItems.at(ptr));
            auto row = static_cast<int>(i);
            emit dataChanged(index(row, 0), index(row, columnCount()), {Qt::DisplayRole});
            ptr++;
            if (ptr >=newItems.count() ) {
                break;
            }
        }
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
    return parent.isValid() ? 0 : static_cast<int>(m_trackers.count());
}

QVariant TorrentTrackerTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.row() >= rowCount() || index.row() < 0) {
        return {};
    }
    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case  0:
        case  1:
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        case  2:
        case  3:
        case  4:
            return int(Qt::AlignRight | Qt::AlignVCenter);
        case  5:
        case  6:
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        default:
            break;
        }

    } else if (role == Qt::DisplayRole) {
        TorrentTrackerInfo tracker = m_trackers.at(index.row());
        switch (index.column()) {
        case 0: return tracker.url;
        case 1: return tracker.trackerId;
        case 2: return tracker.endpoints.size();
        case 3: return tracker.tier;
        case 4: return tracker.failLimit != 0 ? QString::number(tracker.failLimit) : Format::infinity();
        case 5: return tracker.sourceString();
        case 6: return tracker.isVerified ? tr("verified") : tr("not verified");
        default:
            break;
        }
    }
    return {};
}

void TorrentTrackerTableModel::refreshData(const QList<TorrentTrackerInfo> &trackers)
{
    if (trackers.isEmpty()) {
        return;
    }

    QModelIndex parent = {}; // empty is always root

    QList<TorrentTrackerInfo> newItems;

    for (auto i = 0; i < trackers.count(); ++i) {
        auto newItem = trackers.at(i);
        bool replaced = false;
        for (auto j = 0; j < m_trackers.count(); ++j) {
            auto item = m_trackers.at(j);

            // Try update
            if (item.url == newItem.url) {
                m_trackers.removeAt(j);
                m_trackers.insert(j, newItem);
                auto row = static_cast<int>(j);
                emit dataChanged(index(row, 0), index(row, columnCount()), {Qt::DisplayRole});
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
        auto first = static_cast<int>(m_trackers.count());
        auto last = first + static_cast<int>(newItems.count()) - 1;
        beginInsertRows(parent, first, last);
        m_trackers.append(newItems);
        endInsertRows();
    }
}
