/* - DownZemAll! - Copyright (C) 2019 Sebastien Vavassori
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

#include <Core/Stream>
#include <Widgets/StreamWidget>

#include <QtCore/QDebug>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_streamInfoDownloader(new StreamInfoDownloader(this))
{
    ui->setupUi(this);

    ui->lineEdit->setText("https://www.youtube.com/watch?v=C0DPdy98e4c");

    connect(ui->resetButton, SIGNAL(released()), this, SLOT(onResetClicked()));
    connect(ui->continueButton, SIGNAL(released()), this, SLOT(onContinueClicked()));

    connect(m_streamInfoDownloader, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(m_streamInfoDownloader, SIGNAL(collected(StreamInfos*)), this, SLOT(onCollected(StreamInfos*)));

    onResetClicked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onResetClicked()
{
    start();
}

void MainWindow::onContinueClicked()
{
    start(ui->lineEdit->text());
}

void MainWindow::start(const QString &url)
{
    qDebug() << Q_FUNC_INFO << url;
    if (url.isEmpty()) {
        StreamInfos* infos = createDummyStreamInfos();
        ui->streamWidget->showStreamInfos(infos);

    } else {
        ui->continueButton->setEnabled(false);
        ui->streamWidget->setState(StreamWidget::Downloading);
        m_streamInfoDownloader->runAsync(url);
    }
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onError(QString errorMessage)
{
    ui->continueButton->setEnabled(true);
    ui->streamWidget->showErrorMessage(errorMessage);
}

void MainWindow::onCollected(StreamInfos* infos)
{
    ui->continueButton->setEnabled(true);
    ui->streamWidget->showStreamInfos(infos);
}

/******************************************************************************
 ******************************************************************************/
StreamInfos* MainWindow::createDummyStreamInfos()
{
    StreamInfos *infos = new StreamInfos(this);
    infos->_filename        = "TEST VIDEO-C0DPdy98e4c.webm";
    infos->fulltitle        = "TEST VIDEO";
    infos->title            = "TEST VIDEO";
    infos->ext              = "webm";
    infos->description      = "COUNTING LEADER AND TONE";
    infos->thumbnail        = "https://i.ytimg.com/vi/C0DPdy98e4c/hqdefault.jpg";
    infos->extractor        = "youtube";
    infos->extractor_key    = "Youtube";
    infos->format_id        = "244+140";
    infos->formats.clear();
    infos->formats << new StreamFormat("18", "mp4", "360p", 552999, "mp4a.40.2", 96, 44100, "avc1.42001E", 480, 360, 0, 0);
    infos->formats << new StreamFormat("43", "webm", "360p", 287596, "vorbis", 128, 0, "vp8.0", 640, 360, 0, 0);
    infos->formats << new StreamFormat("133", "mp4", "240p", 87155, "none", 0, 0, "avc1.4d400d", 320, 240, 25, 0);
    infos->formats << new StreamFormat("134", "mp4", "360p", 142316, "none", 0, 0, "avc1.4d4015", 480, 360, 25, 0);
    infos->formats << new StreamFormat("135", "mp4", "480p", 202392, "none", 0, 0, "avc1.4d401e", 640, 480, 25, 0);
    infos->formats << new StreamFormat("140", "m4a", "tiny", 280597, "mp4a.40.2", 128, 44100, "none", 0, 0, 0, 0);
    infos->formats << new StreamFormat("160", "mp4", "144p", 63901, "none", 0, 0, "avc1.4d400b", 192, 144, 25, 0);
    infos->formats << new StreamFormat("242", "webm", "240p", 134629, "none", 0, 0, "vp9", 320, 240, 25, 0);
    infos->formats << new StreamFormat("243", "webm", "360p", 205692, "none", 0, 0, "vp9", 480, 360, 25, 0);
    infos->formats << new StreamFormat("244", "webm", "480p", 294311, "none", 0, 0, "vp9", 640, 480, 25, 0);
    infos->formats << new StreamFormat("249", "webm", "tiny", 44319, "opus", 50, 48000, "none", 0, 0, 0, 0);
    infos->formats << new StreamFormat("250", "webm", "tiny", 60843, "opus", 70, 48000, "none", 0, 0, 0, 0);
    infos->formats << new StreamFormat("251", "webm", "tiny", 87201, "opus", 160, 48000, "none", 0, 0, 0, 0);
    infos->formats << new StreamFormat("278", "webm", "144p", 53464, "none", 0, 0, "vp9", 192, 144, 13, 0);
    infos->playlist         = "";
    infos->playlist_index   = "";
    return infos;
}
