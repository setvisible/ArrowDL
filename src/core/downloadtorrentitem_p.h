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

#include <Core/TorrentMessage>

#include <QtCore/QAbstractTableModel>

class TorrentFileTableModel;
class TorrentPeerTableModel;
class TorrentTrackerTableModel;


class DownloadTorrentItemPrivate
{
public:
    DownloadTorrentItemPrivate(DownloadTorrentItem *qq);
    DownloadTorrentItem *q;

    TorrentMetaInfo metaInfo;
    TorrentInfo info;
    TorrentHandleInfo detail;

    TorrentFileTableModel* m_fileModel;
    TorrentPeerTableModel* m_peerModel;
    TorrentTrackerTableModel* m_trackerModel;

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

protected:
    QStringList m_headers;
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
    void retranslateUi();

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
    void retranslateUi();

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
    void retranslateUi();

private:
    QList<TorrentTrackerInfo> m_trackers;
};


#endif // CORE_DOWNLOAD_TORRENT_ITEM_P_H
