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

#include "downloadtorrentitem_p.h"

#include <Core/Format>
#include <Core/Torrent>

#include <QtCore/QDebug>


DownloadTorrentItemPrivate::DownloadTorrentItemPrivate(DownloadTorrentItem *qq)
    : q(qq)
{
    m_fileModel = new TorrentFileTableModel(q);
    m_peerModel = new TorrentPeerTableModel(q);
    m_trackerModel = new TorrentTrackerTableModel(q);
}

/******************************************************************************
 ******************************************************************************/
AbstractTorrentTableModel::AbstractTorrentTableModel(DownloadTorrentItem *parent)
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
TorrentFileTableModel::TorrentFileTableModel(DownloadTorrentItem *parent)
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
            << tr("%")
            << tr("First Piece")
            << tr("# Pieces")
            << tr("Pieces")   /// \todo graph
            << tr("Priority")
            << tr("Modification date")
            << tr("SHA-1")
            << tr("CRC-32");
}

int TorrentFileTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_filesMeta.count();
}

static QString shortPath(const QString &path)
{
    // Remove the name of the torrent file from the path to make the path shorter
    int i = path.indexOf(QChar('\\'));
    if (i < 0) i = path.lastIndexOf(QChar('/'));
    if (i >= 0) i++;
    return path.mid(i);
}

QVariant TorrentFileTableModel::data(const QModelIndex &item, int role) const
{
    if (!item.isValid()) {
        return QVariant();
    }
    if (item.row() >= rowCount() || item.row() < 0) {
        return QVariant();
    }
    if (role == Qt::DisplayRole) {

        const TorrentFileMetaInfo mi = m_filesMeta.at(item.row());

        TorrentFileInfo ti;
        if (item.row() < m_files.count()) {
            ti = m_files.at(item.row());
        }

        const qint64 percentageDownloaded
                = mi.bytesTotal != 0 ? 100 * ti.bytesReceived / mi.bytesTotal : 0;
        const int percent = static_cast<int>(percentageDownloaded);

        switch (item.column()) {
        case  0: return mi.fileName;
        case  1: return shortPath(mi.filePath);
        case  2: return Format::fileSizeToString(mi.bytesTotal);
        case  3: return Format::fileSizeToString(ti.bytesReceived);
        case  4: return QString("%0%").arg(QString::number(percent));

        case  5: return Format::fileSizeToString(mi.bytesOffset);
        case  6: return Format::fileSizeToString(mi.bytesTotal);

        case  7: return QString(); /// \todo graph

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
    endResetModel();
}

void TorrentFileTableModel::refreshData(QList<TorrentFileInfo> files)
{
    m_files = files;
    emit dataChanged(index(0,0), index(rowCount(), columnCount()), {Qt::DisplayRole});
}

/******************************************************************************
 ******************************************************************************/
TorrentPeerTableModel::TorrentPeerTableModel(DownloadTorrentItem *parent)
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
    if (role == Qt::DisplayRole) {
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
TorrentTrackerTableModel::TorrentTrackerTableModel(DownloadTorrentItem *parent)
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
    if (role == Qt::DisplayRole) {
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
