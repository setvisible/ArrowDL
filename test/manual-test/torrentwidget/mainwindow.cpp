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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <Core/Torrent>
#include <Widgets/TorrentWidget>

#include "../../utils/dummytorrentfactory.h"
#include "../../utils/faketorrentcontext.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtWidgets/QPushButton>


#define TICK_MSEC 10

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_torrentContext(new FakeTorrentContext())
  , m_timer(Q_NULLPTR)
{
    ui->setupUi(this);
    ui->torrentWidget->setTorrentContext(m_torrentContext);

    qsrand(QDateTime::currentDateTime().toTime_t());

    connect(ui->startButton, SIGNAL(released()), this, SLOT(onStartClicked()));
    connect(ui->beginButton, SIGNAL(released()), this, SLOT(onZeroPercentClicked()));
    connect(ui->endButton, SIGNAL(released()), this, SLOT(onCompletedClicked()));
    connect(ui->randomButton, SIGNAL(released()), this, SLOT(onRandomClicked()));
    connect(ui->halfButton, SIGNAL(released()), this, SLOT(onHalfClicked()));

    m_torrent = DummyTorrentFactory::createDummyTorrent(this);
    ui->torrentWidget->setTorrent(m_torrent.data());
}

MainWindow::~MainWindow()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onStartClicked()
{
    m_timer ? stopAnimation() : startAnimation();
}

void MainWindow::stopAnimation()
{
    ui->startButton->setText(tr("Start Animation"));
    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = Q_NULLPTR;
    }
}

void MainWindow::startAnimation()
{
    ui->startButton->setText(tr("Stop Animation"));
    if (!m_timer) {
        randomize();
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::animate));
        m_timer->start(TICK_MSEC);
    }
}

void MainWindow::randomize()
{
    int total = m_torrent->metaInfo().initialMetaInfo.files.count();
    m_timeouts.clear();
    m_ticks.clear();
    for (int i = 0; i < total; ++i) {
        int timeout = TICK_MSEC * (((9 * qrand()) / RAND_MAX) + 1);
        m_timeouts << timeout;
    }
}

void MainWindow::animate()
{
    if (m_ticks.count() != m_timeouts.count()) {
        m_ticks = m_timeouts;
    }
    for (int i = 0, total = m_ticks.count(); i < total; ++i) {
        m_ticks[i]--;
        if (m_ticks[i] <= 0) {
            m_ticks[i] = m_timeouts[i];
            animateFile(i);
        }
    }
}

void MainWindow::animateFile(int index)
{
    // qDebug() << Q_FUNC_INFO << index;
    const qint64 receivedPieceSize = 32*1024*8;

    TorrentMetaInfo metaInfo = m_torrent->metaInfo();
    auto bytesTotal = metaInfo.initialMetaInfo.files.at(index).bytesTotal;

    TorrentHandleInfo detail = m_torrent->detail();
    detail.files[index].bytesReceived += receivedPieceSize;
    detail.files[index].bytesReceived = qMin(detail.files[index].bytesReceived, bytesTotal);

    TorrentInfo info = m_torrent->info();
    info.bytesReceived += receivedPieceSize;
    info.bytesReceived = qMin(info.bytesReceived, info.bytesTotal);
    info.bytesSessionDownloaded += receivedPieceSize;
    info.bytesSessionUploaded += receivedPieceSize >> 2;
    info.state = TorrentInfo::downloading;

    m_torrent->setInfo(info, false);
    m_torrent->setDetail(detail, false); // emit changed
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onZeroPercentClicked()
{
    stopAnimation();
    DummyTorrentFactory::setProgress(m_torrent, 0);
}

void MainWindow::onCompletedClicked()
{
    stopAnimation();
    DummyTorrentFactory::setProgress(m_torrent, 100);
}

void MainWindow::onRandomClicked()
{
    stopAnimation();
    int percent = int((100 * qrand()) / RAND_MAX);
    DummyTorrentFactory::setProgress(m_torrent, percent);
}

void MainWindow::onHalfClicked()
{
    stopAnimation();
    DummyTorrentFactory::setProgress(m_torrent, 50);
}
