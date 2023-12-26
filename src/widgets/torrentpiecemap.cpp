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

#define ENABLE_MONITORING false


#if (ENABLE_MONITORING)
#include <QtCore/QElapsedTimer>
#endif

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
    auto _color = color(status);
    auto pal = widget->palette();
    pal.setColor(QPalette::Window, _color);
    widget->setAutoFillBackground(true);
    widget->setPalette(pal);
    widget->setStyleSheet(QString());
}

static QString decorate(int count, TorrentFileInfo::Priority priority)
{
    switch (priority) {
    case TorrentFileInfo::High:   return QString("%0 ³").arg(QString::number(count));
    case TorrentFileInfo::Normal: return QString("%0 ²").arg(QString::number(count));
    case TorrentFileInfo::Low:    return QString("%0 ¹").arg(QString::number(count));
    case TorrentFileInfo::Ignore:
    default:                      return QString("%0 °").arg(QString::number(count));
    }
}

TorrentPieceMap::TorrentPieceMap(QWidget *parent) : QWidget(parent)
  , ui(new Ui::TorrentPieceMap)
  , m_scene(new QGraphicsScene(this))
  , m_workerThread(new TorrentPieceMapWorker(this))
  , m_tileFont(font())
{
    ui->setupUi(this);

    colorize(ui->boxNotAvailable, TorrentPieceItem::Status::NotAvailable);
    colorize(ui->boxAvailable,    TorrentPieceItem::Status::Available);
    colorize(ui->boxDownloaded,   TorrentPieceItem::Status::Downloaded);
    colorize(ui->boxVerified,     TorrentPieceItem::Status::Verified);

    ui->priorityLabel->setText(
        tr("Priority: %0=high %1=normal %2=low %3=ignore").arg(
            QString("³"), QString("²"), QString("¹"), QString("°")));

    /* Calculate metrics */

    // const int pointSize = m_tileFont.pointSize();
    // m_tileFont.setPointSize(pointSize - 1);

    qRegisterMetaType<TorrentPieceData>("TorrentPieceData");

    QFontMetrics fm(m_tileFont);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    m_tileWidth = fm.horizontalAdvance(decorate(999, TorrentFileInfo::Low));
#else
    m_tileWidth = fm.width(decorate(999, TorrentFileInfo::Low));
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
void TorrentPieceMap::showEvent(QShowEvent * /*event*/)
{
    adjustScene();
    m_workerThread->setUseful(true);
}

void TorrentPieceMap::hideEvent(QHideEvent * /*event*/)
{
    // the worker doesn't need to work when the widget is hidden.
    m_workerThread->setUseful(false);
}

void TorrentPieceMap::resizeEvent(QResizeEvent * /*event*/)
{
    adjustScene();
}

/******************************************************************************
 ******************************************************************************/
bool TorrentPieceMapWorker::isUseful()
{
    m_lock.lockForRead();
    auto v = m_isUseful;
    m_lock.unlock();
    return v;
}

void TorrentPieceMapWorker::setUseful(bool useful)
{
    m_lock.lockForWrite();
    m_isUseful = useful;
    m_lock.unlock();
    if (!isRunning()) {
        start();
    }
}

bool TorrentPieceMapWorker::isDirty()
{
    m_lock.lockForRead();
    auto v = m_isDirty;
    m_lock.unlock();
    return v;
}

void TorrentPieceMapWorker::setDirty(bool dirty)
{
    m_lock.lockForWrite();
    m_isDirty = dirty;
    m_lock.unlock();
}

void TorrentPieceMapWorker::doWork(const TorrentPieceData &pieceData,
                                   const QList<TorrentPeerInfo> &peers)
{
    // This method is called every 10~20 milliseconds,
    // while the worker needs >100 milliseconds to complete the task.
    // The idea behind this construct is to cache the input data into a buffer
    // shared between the producer (caller) and the consumer (worker).
    // That is, when the worker is free, it just consumes the most recent data.
    m_lock.lockForWrite();
    m_pieceData = pieceData;
    m_peers = peers;
    m_lock.unlock();

    setDirty(true);

    if (!isRunning()) {
        start();
    }
}

void TorrentPieceMapWorker::run()
{
    if (!isUseful() || !isDirty()) {
        return;
    }

#if (ENABLE_MONITORING)
    QElapsedTimer timer;
    timer.start();
    static int s_id = 0;
    const int id = ++s_id;
    qDebug() << Q_FUNC_INFO << this->thread() << id << "a started";
#endif

    m_lock.lockForRead();
    auto pieceData = m_pieceData;
    // const QList<TorrentPeerInfo> peers = m_peers;
    m_lock.unlock();

    setDirty(false);

    /* Expensive operation */
    // Rem : no more expensive task
    /// \todo remove worker?

    emit resultReady(pieceData);

#if (ENABLE_MONITORING)
    qDebug() << Q_FUNC_INFO << this->thread() << id << "finished in" << timer.elapsed() << "ms";
#endif

    // Once finished, relaunch the worker.
    // Indeed, hhe buffer has maybe been updated during the current run().
    start();
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

        pieceData.pieceAvailability = m_torrent->detail().pieceAvailability;
        pieceData.piecePriority = m_torrent->detail().piecePriority;
        auto peers = m_torrent->detail().peers;

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
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
        flags.setFlag(QGraphicsItem::ItemIsMovable, false);
        flags.setFlag(QGraphicsItem::ItemIsSelectable, false);
        flags.setFlag(QGraphicsItem::ItemIsFocusable, false);
        flags.setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        flags.setFlag(QGraphicsItem::ItemSendsScenePositionChanges, false);
#else
        flags &= ~QGraphicsItem::ItemIsMovable;
        flags &= ~QGraphicsItem::ItemIsMovable;
        flags &= ~QGraphicsItem::ItemIsSelectable;
        flags &= ~QGraphicsItem::ItemIsFocusable;
        flags &= ~QGraphicsItem::ItemSendsGeometryChanges;
        flags &= ~QGraphicsItem::ItemSendsScenePositionChanges;
#endif
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
    auto viewportSize = ui->graphicsView->viewport()->size();
    qreal maxWidth = static_cast<qreal>(viewportSize.width());
    qreal width = m_tileWidth + 2 * m_tilePadding;
    qreal height = m_tileHeight + 2 * m_tilePadding;
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
    auto rect = m_scene->itemsBoundingRect().united(viewportRect);
    m_scene->setSceneRect(rect);
}

/******************************************************************************
 ******************************************************************************/
void TorrentPieceMap::updateScene(const TorrentPieceData &pieceData)
{
    Q_ASSERT(pieceData.size == m_items.count());
    auto size = pieceData.size;
    for (auto i = 0; i < size; ++i) {
        auto item = m_items.at(i);

        if (i < pieceData.pieceAvailability.size()) {
            item->setAvailability(pieceData.pieceAvailability.at(i));
        }

        if (i < pieceData.piecePriority.size()) {
            item->setPriority(pieceData.piecePriority.at(i));
        }

        TorrentPieceItem::Status status = TorrentPieceItem::Status::NotAvailable;
        if (i < pieceData.verifiedPieces.size() && pieceData.verifiedPieces.at(i)) {
            status = TorrentPieceItem::Status::Verified;

        } else if (i < pieceData.downloadedPieces.size() && pieceData.downloadedPieces.at(i)) {
            status = TorrentPieceItem::Status::Downloaded;

        } else if (i < pieceData.availablePieces.size() && pieceData.availablePieces.at(i)) {
            status = TorrentPieceItem::Status::Available;
        }
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
TorrentPieceItem::TorrentPieceItem(qreal width, qreal height, qreal padding,
                                   const QFont &font, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_font(font)
    , m_width(width)
    , m_height(height)
    , m_padding(padding)
{
}

void TorrentPieceItem::setAvailability(int availability)
{
    m_availability = availability;
}

void TorrentPieceItem::setPriority(TorrentFileInfo::Priority priority)
{
    m_priority = priority;
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
    painter->drawText(tileRect, Qt::AlignCenter, decorate(m_availability, m_priority));
}
