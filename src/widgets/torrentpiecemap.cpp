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

#include "torrentpiecemap.h"
#include "ui_torrentpiecemap.h"

#include <Core/Torrent>
#include <Widgets/Globals>

#include <QtCore/QDebug>
#include <QtCore/QtMath>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsItem>


static QColor color(TorrentPieceItem::Status status)
{
    switch (status) {
    case TorrentPieceItem::Status::NotAvailable:   return s_lightGrey;
    case TorrentPieceItem::Status::Available:      return s_orange;
    case TorrentPieceItem::Status::Downloaded:     return s_green;
    case TorrentPieceItem::Status::Verified:       return s_purple;
    }
    Q_UNREACHABLE();
}

static void colorize(QWidget *widget, TorrentPieceItem::Status status)
{
    QColor _color = color(status);
    QPalette pal = widget->palette();
    pal.setColor(QPalette::Background, _color);
    widget->setAutoFillBackground(true);
    widget->setPalette(pal);
    widget->setStyleSheet(QString());
}

TorrentPieceMap::TorrentPieceMap(QWidget *parent) : QWidget(parent)
  , ui(new Ui::TorrentPieceMap)
  , m_scene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    colorize(ui->boxNotAvailable, TorrentPieceItem::Status::NotAvailable);
    colorize(ui->boxAvailable,    TorrentPieceItem::Status::Available);
    colorize(ui->boxDownloaded,   TorrentPieceItem::Status::Downloaded);
    colorize(ui->boxVerified,     TorrentPieceItem::Status::Verified);

    /* Calculate metrics */
    m_tileFont = font();
    // m_tileFont.setPixelSize(8);

    qRegisterMetaType<TorrentPieceData>("TorrentPieceData");

    QFontMetrics fm(m_tileFont);
#if QT_VERSION >= 0x051100
    squareWidth = fm.horizontalAdvance("999");
#else
    m_tileWidth = fm.width("999");
#endif
    m_tileHeight = fm.height() + 2 * m_tilePadding;
    m_tileWidth += 2 * m_tilePadding;

    /* Graphics Scene */
    m_rootItem = m_scene->addRect(QRectF(0, 0, 0, 0));
    m_rootItem->setFlags(QGraphicsItem::ItemHasNoContents);
    m_scene->setBackgroundBrush(QBrush(s_darkPurple));

    /* Indexing is efficient for static scenes */
    /* But here items will move around, so disable it. */
    m_scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    /* Graphics View */
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setRenderHint(QPainter::TextAntialiasing);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    /* Cache pre-rendered content in a pixmap, which is then drawn onto the viewport */
    ui->graphicsView->setCacheMode(QGraphicsView::CacheBackground);

    /* Worker thread */
    m_workerThread = new TorrentPieceMapWorker(this);
    connect(m_workerThread, &TorrentPieceMapWorker::resultReady, this, &TorrentPieceMap::handleResults);

    resetUi();
}

TorrentPieceMap::~TorrentPieceMap()
{
    delete ui;
    m_workerThread->quit();
    m_workerThread->wait();
    m_workerThread->deleteLater();
}

/******************************************************************************
 ******************************************************************************/
Torrent *TorrentPieceMap::torrent() const
{
    return m_torrent;
}

void TorrentPieceMap::setTorrent(Torrent *torrent)
{
    if (m_torrent == torrent) {
        return;
    }
    if (m_torrent) {
        disconnect(m_torrent, &Torrent::changed, this, &TorrentPieceMap::onChanged);
    }
    m_torrent = torrent;
    if (m_torrent) {
        connect(m_torrent, &Torrent::changed, this, &TorrentPieceMap::onChanged);
    }
    resetUi();
}

/******************************************************************************
 ******************************************************************************/
void TorrentPieceMap::showEvent(QShowEvent */*event*/)
{
    adjustScene();
}

void TorrentPieceMap::resizeEvent(QResizeEvent */*event*/)
{
    adjustScene();
}

/******************************************************************************
 ******************************************************************************/
void TorrentPieceMapWorker::doWork(const TorrentPieceData &pieceData,
                                   const QList<TorrentPeerInfo> &peers)
{
    // This method is called every 10~20 milliseconds,
    // while the worker needs >100 milliseconds to complete the task.
    // The idea behind this construct is to cache the input data into a buffer
    // shared between the producer (caller) and the consumer (worker).
    // That is, when the worker is free, it just consumes the most recent data.
    //
    m_lock.lockForWrite();
    m_pieceData = pieceData;
    m_peers = peers;
    m_isDirty = true;
    m_lock.unlock();

    if (!isRunning()) {
        start();
    }
}

void TorrentPieceMapWorker::run()
{
    m_lock.lockForRead();
    TorrentPieceData pieceData = m_pieceData;
    const QList<TorrentPeerInfo> peers = m_peers;
    m_lock.unlock();
    m_lock.lockForWrite();
    m_isDirty = false;
    m_lock.unlock();

    /* Expensive operation */
    foreach (auto peer, peers) {
        QBitArray peerAvailablePieces = peer.availablePieces;
        if (peerAvailablePieces.size() != pieceData.size) {
            qWarning("Peer has not the same number of pieces as You "
                     "(peer:'%i', you:'%i').",
                     peerAvailablePieces.size(),
                     pieceData.size);
        } else {
            for (int i = 0; i < pieceData.size; ++i) {
                pieceData.availablePieces |= peerAvailablePieces;
                if (peerAvailablePieces.testBit(i)) {
                    pieceData.totalPeers[i]++;
                }
            }
        }
    }
    emit resultReady(pieceData);

    /* Once finished, relaunch the worker if the buffer was updated during the process. */
    m_lock.lockForRead();
    const bool isDirty = m_isDirty;
    m_lock.unlock();
    if (isDirty) {
        m_lock.lockForWrite();
        m_isDirty = false;
        m_lock.unlock();
        start();
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentPieceMap::handleResults(const TorrentPieceData &pieceData)
{
    setPieceData(pieceData);
}

void TorrentPieceMap::updateWidget()
{
    if (m_torrent) {
        TorrentPieceData pieceData;
        pieceData.size = m_torrent->metaInfo().initialMetaInfo.pieceCount;
        pieceData.downloadedPieces = m_torrent->info().downloadedPieces;
        pieceData.verifiedPieces = m_torrent->info().verifiedPieces;
        pieceData.availablePieces.resize(pieceData.size);
        pieceData.totalPeers.resize(pieceData.size);

        const QList<TorrentPeerInfo> peers = m_torrent->detail().peers;

        m_workerThread->doWork(pieceData, peers);

    } else {
        clearScene();
    }
}

void TorrentPieceMap::setPieceData(const TorrentPieceData &pieceData)
{
    if (pieceData.size != m_items.count()) {
        clearScene();
        populateScene(pieceData);
        adjustScene();
    }
    updateScene(pieceData);
}

/******************************************************************************
 ******************************************************************************/
void TorrentPieceMap::clearScene()
{
    foreach (auto item, m_items) {
        m_scene->removeItem(item);
    }
    m_items.clear();
}

void TorrentPieceMap::populateScene(const TorrentPieceData &pieceData)
{
    for (int i = 0; i < pieceData.size; ++i) {
        auto item = new TorrentPieceItem(
                    m_tileWidth, m_tileHeight,
                    m_tilePadding, m_tileFont,
                    m_rootItem);

        auto flags = item->flags();
        flags.setFlag(QGraphicsItem::ItemIsMovable, false);
        flags.setFlag(QGraphicsItem::ItemIsSelectable, false);
        flags.setFlag(QGraphicsItem::ItemIsFocusable, false);
        flags.setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        flags.setFlag(QGraphicsItem::ItemSendsScenePositionChanges, false);

        item->setFlags(flags);
        m_items.append(item);
    }
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Items are moved around, to fill the width of the viewport.
 */
void TorrentPieceMap::adjustScene()
{
    const QSize viewportSize = ui->graphicsView->viewport()->size();
    const int maxWidth = viewportSize.width();
    const qreal width = m_tileWidth + 2 * m_tilePadding;
    const qreal height = m_tileHeight + 2 * m_tilePadding;
    qreal x = 0;
    qreal y = 0;
    foreach (auto item, m_items) {
        if (x + width >= maxWidth) {
            x = 0;
            y += height;
        }
        item->setPos(x, y);
        x += width;
    }
    const QRectF viewportRect(QPointF(0, 0), viewportSize);
    const QRectF rect = m_scene->itemsBoundingRect().united(viewportRect);
    m_scene->setSceneRect(rect);
}

/******************************************************************************
 ******************************************************************************/
void TorrentPieceMap::updateScene(const TorrentPieceData &pieceData)
{
    Q_ASSERT(pieceData.size == m_items.count());
    const int size = pieceData.size;
    for (int i = 0; i < size; ++i) {
        TorrentPieceItem *item = m_items.at(i);
        const int totalPeers = i < pieceData.totalPeers.size()
                ? pieceData.totalPeers.at(i)
                : 0;
        const bool availablePiece = i < pieceData.availablePieces.size()
                ? pieceData.availablePieces.at(i)
                : false;
        const bool downloadedPiece = i < pieceData.downloadedPieces.size()
                ? pieceData.downloadedPieces.at(i)
                : false;
        const bool verifiedPiece = i < pieceData.verifiedPieces.size()
                ? pieceData.verifiedPieces.at(i)
                : false;

        TorrentPieceItem::Status status =
                verifiedPiece
                ? TorrentPieceItem::Status::Verified
                : downloadedPiece
                  ? TorrentPieceItem::Status::Downloaded
                  : availablePiece
                    ? TorrentPieceItem::Status::Available
                    : TorrentPieceItem::Status::NotAvailable;

        item->setValue(totalPeers);
        item->setStatus(status);
        item->update();
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentPieceMap::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

/******************************************************************************
 ******************************************************************************/
void TorrentPieceMap::onChanged()
{
    updateWidget();
}

void TorrentPieceMap::resetUi()
{
    updateWidget();
}

void TorrentPieceMap::retranslateUi()
{
    // Nothing
}

/******************************************************************************
 ******************************************************************************/
TorrentPieceItem::TorrentPieceItem(int width, int height, int padding,
                                   const QFont &font, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_font(font)
    , m_width(width)
    , m_height(height)
    , m_padding(padding)
    , m_value(0)
    , m_status(Status::NotAvailable)
{
}

void TorrentPieceItem::setValue(int value)
{
    m_value = value;
}

void TorrentPieceItem::setStatus(Status status)
{
    m_status = status;
}

QRectF TorrentPieceItem::boundingRect() const
{
    return {0, 0, m_width + 2* m_padding, m_height + 2* m_padding};
}

QPainterPath TorrentPieceItem::shape() const
{
    QPainterPath path;
    path.addRect(m_padding, m_padding, m_width, m_height);
    return path;
}

void TorrentPieceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QRectF tileRect = QRectF(m_padding, m_padding, m_width, m_height);
    QColor tileColor = color(m_status);
    painter->fillRect(tileRect, tileColor);
    painter->setFont(m_font);
    painter->drawText(tileRect, Qt::AlignCenter, QString::number(m_value));
}
