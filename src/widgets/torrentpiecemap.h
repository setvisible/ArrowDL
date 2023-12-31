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

#ifndef WIDGETS_TORRENT_PIECE_MAP_H
#define WIDGETS_TORRENT_PIECE_MAP_H

#include <QtCore/QBitArray>
#include <QtCore/QReadWriteLock>
#include <QtCore/QThread>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGraphicsItem>

#include <Core/Torrent>

class QGraphicsScene;

namespace Ui {
class TorrentPieceMap;
}

class Torrent;
class TorrentPieceMapWorker;
class TorrentPieceItem;

struct TorrentPieceData
{
    qint64 size = 0;
    QBitArray availablePieces = {};
    QBitArray downloadedPieces = {};
    QBitArray verifiedPieces = {};
    QVector<int> pieceAvailability = {};
    QVector<TorrentFileInfo::Priority> piecePriority = {};
};

/* Enable the type to be used with QVariant. */
Q_DECLARE_METATYPE(TorrentPieceData)


class TorrentPieceMap : public QWidget
{
    Q_OBJECT
public:
    explicit TorrentPieceMap(QWidget *parent = nullptr);
    ~TorrentPieceMap() override;

    Torrent *torrent() const;
    void setTorrent(Torrent *torrent);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

protected slots:
    void changeEvent(QEvent *event) override;

private slots:
    void onChanged();
    void handleResults(const TorrentPieceData &pieceData);

private:
    Ui::TorrentPieceMap *ui = nullptr;
    Torrent *m_torrent = nullptr;
    QGraphicsScene *m_scene = nullptr;
    QGraphicsRectItem *m_rootItem = nullptr;
    QList<TorrentPieceItem *> m_items = {};

    TorrentPieceMapWorker *m_workerThread = nullptr;

    QFont m_tileFont = {};
    qreal m_tileHeight = 0;
    qreal m_tileWidth = 0;
    qreal m_tilePadding = 1;

    void resetUi();
    void retranslateUi();

    void setPieceData(const TorrentPieceData &pieceData);

    void updateWidget();
    void clearScene();
    void populateScene(const TorrentPieceData &pieceData);
    void adjustScene();
    void updateScene(const TorrentPieceData &pieceData);
};

/******************************************************************************
 ******************************************************************************/
class TorrentPieceMapWorker : public QThread
{
    Q_OBJECT
public:
    TorrentPieceMapWorker(QObject *parent = nullptr): QThread(parent) {}

    bool isUseful();
    void setUseful(bool useful);

    bool isDirty();
    void setDirty(bool dirty);

    void doWork(const TorrentPieceData &pieceData, const QList<TorrentPeerInfo> &peers);

signals:
    void resultReady(const TorrentPieceData &pieceData);

protected:
    void run() override;

private:
    QReadWriteLock m_lock;
    bool m_isUseful = false;
    bool m_isDirty = false;

    TorrentPieceData m_pieceData = {};
    QList<TorrentPeerInfo> m_peers = {};
};

/******************************************************************************
 ******************************************************************************/
class TorrentPieceItem : public QGraphicsItem
{
public:
    enum class Status {
        NotAvailable,
        Available,
        Downloaded,
        Verified
    };

    explicit TorrentPieceItem(
        qreal width, qreal height, qreal padding,
        const QFont &font, QGraphicsItem *parent = nullptr);

    void setAvailability(int availability);
    void setPriority(TorrentFileInfo::Priority priority);

    void setStatus(Status status);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QFont m_font = {};
    qreal m_width = 0;
    qreal m_height = 0;
    qreal m_padding = 0;
    int m_availability = 0;
    TorrentFileInfo::Priority m_priority = TorrentFileInfo::Normal;
    Status m_status = Status::NotAvailable;
};

#endif // WIDGETS_TORRENT_PIECE_MAP_H
