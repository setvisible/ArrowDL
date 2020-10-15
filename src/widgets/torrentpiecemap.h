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
    int size = 0;
    QBitArray availablePieces;
    QBitArray downloadedPieces;
    QBitArray verifiedPieces;
    QVector<int> pieceAvailability;
    QVector<TorrentFileInfo::Priority> piecePriority;
};

/* Enable the type to be used with QVariant. */
Q_DECLARE_METATYPE(TorrentPieceData)


class TorrentPieceMap : public QWidget
{
    Q_OBJECT
public:
    explicit TorrentPieceMap(QWidget *parent = Q_NULLPTR);
    ~TorrentPieceMap() Q_DECL_OVERRIDE;

    Torrent *torrent() const;
    void setTorrent(Torrent *torrent);

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

protected slots:
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onChanged();
    void handleResults(const TorrentPieceData &pieceData);

private:
    Ui::TorrentPieceMap *ui;
    Torrent *m_torrent{Q_NULLPTR};
    QGraphicsScene *m_scene;
    QGraphicsRectItem *m_rootItem{Q_NULLPTR};
    QList<TorrentPieceItem *> m_items;

    TorrentPieceMapWorker *m_workerThread;

    QFont m_tileFont;
    int m_tileHeight;
    int m_tileWidth;
    int m_tilePadding = 1;

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
    TorrentPieceMapWorker(QObject *parent = Q_NULLPTR): QThread(parent) {}

    bool isUseful();
    void setUseful(bool useful);

    bool isDirty();
    void setDirty(bool dirty);

    void doWork(const TorrentPieceData &pieceData, const QList<TorrentPeerInfo> &peers);

signals:
    void resultReady(const TorrentPieceData &pieceData);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QReadWriteLock m_lock;
    bool m_isUseful{false};
    bool m_isDirty{false};

    TorrentPieceData m_pieceData;
    QList<TorrentPeerInfo> m_peers;
};

/******************************************************************************
 ******************************************************************************/
class TorrentPieceItem : public QGraphicsItem
{
public:
    explicit TorrentPieceItem(int width, int height, int padding,
                              const QFont &font, QGraphicsItem *parent = Q_NULLPTR);

    void setAvailability(int availability);
    void setPriority(TorrentFileInfo::Priority priority);

    enum class Status {
        NotAvailable,
        Available,
        Downloaded,
        Verified
    };
    void setStatus(Status status);

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

private:
    QFont m_font;
    qreal m_width;
    qreal m_height;
    qreal m_padding;
    int m_availability{0};
    TorrentFileInfo::Priority m_priority{TorrentFileInfo::Normal};
    Status m_status{Status::NotAvailable};
};

#endif // WIDGETS_TORRENT_PIECE_MAP_H
