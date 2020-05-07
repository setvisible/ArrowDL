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

#include "torrentwidget.h"
#include "ui_torrentwidget.h"


#include <Core/DownloadTorrentItem>
#include <Core/Format>
#include <Core/Torrent>

#include <QtCore/QDebug>
#include <QtCore/QAbstractTableModel>
#include <QtWidgets/QTableView>

#define C_COLUMN_MINIMUM_WIDTH       10
#define C_COLUMN_DEFAULT_WIDTH      100
#define C_ROW_DEFAULT_HEIGHT         18


TorrentWidget::TorrentWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::TorrentWidget)
  , m_item(Q_NULLPTR)
{
    ui->setupUi(this);

    setupUiTableView(ui->fileTableView);
    setupUiTableView(ui->peerTableView);
    setupUiTableView(ui->trackerTableView);

    ui->downloadedProgressBar->setRange(0, 100);

    resetUi();
}

TorrentWidget::~TorrentWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::clear()
{
    m_item = Q_NULLPTR;
    resetUi();
}

bool TorrentWidget::isEmpty() const
{
    return m_item == Q_NULLPTR;
}

/******************************************************************************
 ******************************************************************************/
IDownloadItem *TorrentWidget::downloadItem() const
{
    return m_item;
}

void TorrentWidget::setDownloadItem(IDownloadItem *item)
{    
    DownloadTorrentItem *torrentItem = dynamic_cast<DownloadTorrentItem*>(item);
    if (m_item == torrentItem) {
        return;
    }
    if (m_item) {
        disconnect(m_item, &DownloadTorrentItem::changed, this, &TorrentWidget::onChanged);
    }
    m_item = torrentItem;
    if (m_item) {
        connect(m_item, &DownloadTorrentItem::changed, this, &TorrentWidget::onChanged);
    }
    resetUi();
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::onChanged()
{
    updateWidget();
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::resetUi()
{
    if (m_item) {
        QAbstractTableModel* fileModel = m_item->fileModel();
        QAbstractTableModel* peerModel = m_item->peerModel();
        QAbstractTableModel* trackerModel = m_item->trackerModel();

        ui->fileTableView->setModel(fileModel);
        ui->peerTableView->setModel(peerModel);
        ui->trackerTableView->setModel(trackerModel);

        setColumnWidths(ui->fileTableView, m_item->defaultFileColumnWidths());
        setColumnWidths(ui->peerTableView, m_item->defaultPeerColumnWidths());
        setColumnWidths(ui->trackerTableView, m_item->defaultTrackerColumnWidths());

        // hide column 0
        ui->fileTableView->hideColumn(0);

    } else {
        ui->fileTableView->setModel(Q_NULLPTR);
        ui->peerTableView->setModel(Q_NULLPTR);
        ui->trackerTableView->setModel(Q_NULLPTR);
    }

    updateWidget();
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::updateWidget()
{
    if (!m_item) {
        ui->stackedWidget->setCurrentWidget(ui->pageGeneral);

    } else {
        if (m_item->metaInfo().error.type == TorrentError::NoError) {
            updateProgressBar();
            updateTorrentPage();
            ui->stackedWidget->setCurrentWidget(ui->pageTorrent);

        } else {
            ui->labelErrorMessage->setText(m_item->metaInfo().error.message);
            ui->stackedWidget->setCurrentWidget(ui->pageTorrentError);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::updateProgressBar()
{
    if (m_item && m_item->progress() >= 0) {
        ui->downloadedProgressBar->setValue(m_item->progress());
    } else {
        ui->downloadedProgressBar->setValue(0);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::updateTorrentPage()
{
    if (!m_item) {
        return;
    }
    const TorrentMetaInfo &mi = m_item->metaInfo();
    const TorrentInfo &ti = m_item->info();

    auto wasted         = tr("%0 (%1 hashfails)")
            .arg(Format::fileSizeToString(ti.bytesFailed + ti.bytesRedundant))
            .arg(Format::fileSizeToString(ti.bytesFailed));

    auto downloaded     = tr("%0 (total %1)")
            .arg(Format::fileSizeToString(ti.bytesSessionDownloaded))
            .arg(Format::fileSizeToString(ti.bytesSessionDownloaded + mi.bytesTotalDownloaded));

    auto uploaded       = tr("%0 (total %1)")
            .arg(Format::fileSizeToString(ti.bytesSessionUploaded))
            .arg(Format::fileSizeToString(ti.bytesSessionUploaded + mi.bytesTotalUploaded));

    auto seeds          = tr("%0 of %1 connected (%2 in swarm)")
            .arg(text(ti.connectedSeedsCount))
            .arg(text(ti.seedsCount))
            .arg(text(mi.seedsInSwarm));

    auto peers          = tr("%0 of %1 connected (%2 in swarm)")
            .arg(text(ti.connectedPeersCount))
            .arg(text(ti.peersCount))
            .arg(text(mi.peersInSwarm));

    auto shareRatio     = text(QString("0.000"));
    auto status         = text(m_item ? m_item->status() : QString());

    auto pieces         = tr("%0 x %1")
            .arg(text(mi.initialMetaInfo.pieceCount))
            .arg(Format::fileSizeToString(mi.initialMetaInfo.pieceByteSize));


    // GroupBox Transfer
    ui->timeElapsedLineEdit->setText(   Format::timeToString(ti.elapsedTime));
    ui->timeRemainingLineEdit->setText( Format::timeToString(ti.remaingTime));
    ui->wastedLineEdit->setText(        wasted);

    ui->downloadedLineEdit->setText(    downloaded);
    ui->downSpeedLineEdit->setText(     Format::currentSpeedToString(ti.downloadSpeed));
    ui->downLimitLineEdit->setText(     Format::currentSpeedToString(mi.downloadBandwidthLimit, true));

    ui->uploadedLineEdit->setText(      uploaded);
    ui->upSpeedLineEdit->setText(       Format::currentSpeedToString(ti.uploadSpeed));
    ui->upLimitLineEdit->setText(       Format::currentSpeedToString(mi.uploadBandwidthLimit, true));

    ui->seedsLineEdit->setText(         seeds);
    ui->peersLineEdit->setText(         peers);
    ui->shareRatioLineEdit->setText(    shareRatio);
    ui->statusLineEdit->setText(        status);

    // GroupBox General
    ui->saveAsLineEdit->setText(        text(mi.initialMetaInfo.name));
    ui->piecesLineEdit->setText         (pieces);
    ui->totalSizeLineEdit->setText(     Format::fileSizeToString(mi.initialMetaInfo.bytesTotal));
    ui->createdOnLineEdit->setText(     text(mi.initialMetaInfo.creationDate));
    ui->createdByLineEdit->setText(     text(mi.initialMetaInfo.creator));
    ui->addedOnLineEdit->setText(       text(mi.addedTime));
    ui->completedOnLineEdit->setText(   text(mi.completedTime));
    ui->hashLineEdit->setText(          text(mi.initialMetaInfo.infohash));
    ui->commentLineEdit->setText(       text(mi.initialMetaInfo.comment));
    ui->magnetLinkLineEdit->setText(    text(mi.initialMetaInfo.magnetLink));
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::setupUiTableView(QTableView *view)
{
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->setAlternatingRowColors(false);
    view->setMidLineWidth(3);
    view->setShowGrid(false);
    view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->verticalHeader()->setHighlightSections(false);
    view->verticalHeader()->setDefaultSectionSize(C_ROW_DEFAULT_HEIGHT);
    view->verticalHeader()->setMinimumSectionSize(C_ROW_DEFAULT_HEIGHT);
    view->horizontalHeader()->setHighlightSections(false);
    view->horizontalHeader()->setDefaultSectionSize(C_COLUMN_DEFAULT_WIDTH);
    view->horizontalHeader()->setMinimumSectionSize(C_COLUMN_MINIMUM_WIDTH);
    view->verticalHeader()->setVisible(false);
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::setColumnWidths(QTableView *view, const QList<int> &widths)
{
    if (view && view->model()) {
        for (int column = 0, count = view->model()->columnCount(); column < count; ++column) {
            if (column < widths.count()) {
                auto width = widths.at(column);
                view->setColumnWidth(column, width);
            } else {
                view->setColumnWidth(column, C_COLUMN_DEFAULT_WIDTH);
            }
        }
    }
}

/******************************************************************************
 ******************************************************************************/
inline QString TorrentWidget::text(int value, bool showInfiniteSymbol)
{
    const QString defaultSymbol = showInfiniteSymbol ? Format::infinity() : QString("-");
    return value < 0
            ? defaultSymbol
            : QString::number(value);
}

inline QString TorrentWidget::text(const QString &text)
{
    return text.isNull() || text.isEmpty()
            ? QString("-")
            : text;
}

inline QString TorrentWidget::text(const QDateTime &datetime)
{
    return datetime.isNull() || !datetime.isValid()
            ? QString("-")
            : datetime.toString("dd/MM/yy HH:mm:ss");
}
