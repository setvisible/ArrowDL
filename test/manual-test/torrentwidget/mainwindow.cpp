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

#include <Core/Theme>
#include <Core/Torrent>
#include <Core/TorrentBaseContext>
#include <Widgets/TorrentWidget>

#include "../../utils/dummytorrentanimator.h"
#include "../../utils/dummytorrentfactory.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtWidgets/QPushButton>


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_torrentContext(new TorrentBaseContext())
  , m_animator(new DummyTorrentAnimator(this))
{
    ui->setupUi(this);

    Theme::applyTheme({});

    m_torrent = DummyTorrentFactory::createDummyTorrent(this);

    connect(m_animator, SIGNAL(started(bool)), this, SLOT(onAnimatorStarted(bool)));
    connect(ui->startButton, SIGNAL(released()), this, SLOT(onStartClicked()));
    connect(ui->beginButton, SIGNAL(released()), this, SLOT(onZeroPercentClicked()));
    connect(ui->endButton, SIGNAL(released()), this, SLOT(onCompletedClicked()));
    connect(ui->randomButton, SIGNAL(released()), this, SLOT(onRandomClicked()));
    connect(ui->halfButton, SIGNAL(released()), this, SLOT(onHalfClicked()));

    ui->torrentWidget->setTorrent(m_torrent.data());
    ui->torrentWidget->setTorrentContext(m_torrentContext);

    m_animator->setTorrent(m_torrent.data());
}

MainWindow::~MainWindow()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onAnimatorStarted(bool started)
{
    ui->startButton->setText(started ? QLatin1String("Stop Animation")
                                     : QLatin1String("Start Animation"));
}

void MainWindow::onStartClicked()
{
    m_animator->isStarted() ? m_animator->stop() : m_animator->start();
}

void MainWindow::onZeroPercentClicked()
{
    m_animator->stop();
    m_animator->setProgress(0);
}

void MainWindow::onCompletedClicked()
{
    m_animator->stop();
    m_animator->setProgress(100);
}

void MainWindow::onRandomClicked()
{
    m_animator->stop();
    auto percent = Utils::randomBetween(1, 100);
    m_animator->setProgress(percent);
}

void MainWindow::onHalfClicked()
{
    m_animator->stop();
    m_animator->setProgress(50);
}
