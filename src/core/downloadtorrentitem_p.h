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

#ifndef CORE_DOWNLOAD_TORRENT_ITEM_P_H
#define CORE_DOWNLOAD_TORRENT_ITEM_P_H

#include "downloadtorrentitem.h"

#include <Core/Torrent>

#include <QtCore/QAbstractTableModel>

class TorrentFileTableModel;
class TorrentPeerTableModel;
class TorrentTrackerTableModel;


class DownloadTorrentItemPrivate
{
public:
    DownloadTorrentItemPrivate(DownloadTorrentItem *qq);
    DownloadTorrentItem *q;

    QByteArray m_torrentData;

    TorrentMetaInfo metaInfo;
    TorrentInfo info;
    TorrentHandleInfo detail;

    TorrentFileTableModel* m_fileModel;
    TorrentPeerTableModel* m_peerModel;
    TorrentTrackerTableModel* m_trackerModel;

    void startTorrentInfoAsync();
};

/******************************************************************************
 ******************************************************************************/
class Headers // Holds column header's widths and titles
{
public:
    Headers() = default;
    Headers(const QList<QPair<int, QString> > &l) {d = l; }
    Headers &operator=(const QList<QPair<int, QString> > &l) { d = l; return *this; }

    int count() const { return d.count(); }

    QString title(int index) const {
        return (index >= 0 && index < d.count()) ? d.at(index).second : QString();
    }

    int width(int index) const {
        return (index >= 0 && index < d.count()) ? d.at(index).first : 100;
    }

    QList<int> widths() const
    {
        QList<int> widths;
        foreach (auto header, d) { widths << header.first; }
        return widths;
    }

private:
    QList<QPair<int, QString> > d;
};

/******************************************************************************
 ******************************************************************************/
class AbstractTorrentTableModel: public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit AbstractTorrentTableModel(DownloadTorrentItem *parent);

    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    QList<int> defaultColumnWidths() const;
protected:
    Headers m_headers;
};

/******************************************************************************
 ******************************************************************************/
class TorrentFileTableModel: public AbstractTorrentTableModel
{
    Q_OBJECT
public:
    explicit TorrentFileTableModel(DownloadTorrentItem *parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;


    void refreshMetaData(QList<TorrentFileMetaInfo> files);
    void refreshData(QList<TorrentFileInfo> files);
private:
    QList<TorrentFileMetaInfo> m_filesMeta;
    QList<TorrentFileInfo> m_files;
};

/******************************************************************************
 ******************************************************************************/
/*!
 * \class TorrentPeerTableModel
 * \brief List of peers and various information about them
 */
class TorrentPeerTableModel: public AbstractTorrentTableModel
{
    Q_OBJECT
public:
    explicit TorrentPeerTableModel(DownloadTorrentItem *parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    void refreshData(QList<TorrentPeerInfo> peers);
private:
    QList<TorrentPeerInfo> m_peers;
};

/******************************************************************************
 ******************************************************************************/
class TorrentTrackerTableModel: public AbstractTorrentTableModel
{
    Q_OBJECT
public:
    explicit TorrentTrackerTableModel(DownloadTorrentItem *parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    void refreshData(QList<TorrentTrackerInfo> trackers);
private:
    QList<TorrentTrackerInfo> m_trackers;
};


#endif // CORE_DOWNLOAD_TORRENT_ITEM_P_H
