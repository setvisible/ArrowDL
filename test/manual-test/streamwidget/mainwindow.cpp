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

#include "../../utils/dummystreamfactory.h"

#include <Widgets/StreamListWidget>

#include <QtCore/QDebug>
#include <QtWidgets/QPushButton>

static const QString s_urlMp4Link = "http://camendesign.com/code/video_for_everybody/test.html";

/// \todo Video URL containing an ampersand '&' character
/// \todo static const QString s_youtubeDlTestVideoLink = "https://www.youtube.com/watch?t=4&v=BaW_jenozKc";


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_streamObjectDownloader(new StreamObjectDownloader(this))
{
    ui->setupUi(this);

    ui->lineEdit->setText("https://www.youtube.com/watch?v=C0DPdy98e4c");

    connect(ui->resetButton, SIGNAL(released()), this, SLOT(onResetClicked()));
    connect(ui->continueButton, SIGNAL(released()), this, SLOT(onContinueClicked()));

    connect(ui->emptyButton,       SIGNAL(released()), this, SLOT(onEmptyButtonClicked()));
    connect(ui->errorButton,       SIGNAL(released()), this, SLOT(onErrorButtonClicked()));
    connect(ui->youtubeButton,     SIGNAL(released()), this, SLOT(onYoutubeButtonClicked()));
    connect(ui->dailymotionButton, SIGNAL(released()), this, SLOT(onDailymotionButtonClicked()));
    connect(ui->otherSiteButton,   SIGNAL(released()), this, SLOT(onOtherSiteButtonClicked()));
    connect(ui->urlMp4Button,      SIGNAL(released()), this, SLOT(onUrlMp4ButtonClicked()));
    connect(ui->playlistButton,    SIGNAL(released()), this, SLOT(onPlaylistButtonClicked()));

    ui->urlMp4Button->setToolTip(s_urlMp4Link);

    connect(m_streamObjectDownloader, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(m_streamObjectDownloader, SIGNAL(collected(QList<StreamObject>)), this, SLOT(onCollected(QList<StreamObject>)));

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
    if (!url.isEmpty()) {
        ui->streamListWidget->setMessageWait();
        m_streamObjectDownloader->runAsync(url);
    } else {
        ui->streamListWidget->setMessageEmpty();
    }
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onEmptyButtonClicked()
{
    auto obj = DummyStreamFactory::createDummyStreamObject();
    ui->streamListWidget->setStreamObjects(obj);
}

void MainWindow::onErrorButtonClicked()
{
    auto obj = DummyStreamFactory::createDummyErrorStreamObject();
    ui->streamListWidget->setStreamObjects(obj);
}

void MainWindow::onYoutubeButtonClicked()
{
    auto obj = DummyStreamFactory::createDummyStreamObject_Youtube();
    ui->streamListWidget->setStreamObjects(obj);
}

void MainWindow::onDailymotionButtonClicked()
{
    auto obj = DummyStreamFactory::createDummyStreamObject_Dailymotion();
    ui->streamListWidget->setStreamObjects(obj);
}

void MainWindow::onOtherSiteButtonClicked()
{
    auto obj = DummyStreamFactory::createDummyStreamObject_Other();
    ui->streamListWidget->setStreamObjects(obj);
}

void MainWindow::onUrlMp4ButtonClicked()
{
    start(s_urlMp4Link);
}

void MainWindow::onPlaylistButtonClicked()
{
    QList<StreamObject> list;
    list << DummyStreamFactory::createDummyStreamObject_Youtube();
    list << DummyStreamFactory::createDummyStreamObject_Dailymotion();
    list << DummyStreamFactory::createDummyStreamObject_unavailable();
    list << DummyStreamFactory::createDummyStreamObject_Other();
    for (int i = 0; i < list.count(); ++i) {
        auto item = list.at(i);
        item.playlist = QLatin1String("Playlist of favorite streams");
        item.playlist_index = QString::number(i + 1);
        list.replace(i, item);
    }
    ui->streamListWidget->setStreamObjects(list);
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onError(QString errorMessage)
{
    ui->continueButton->setEnabled(true);
    ui->streamListWidget->setMessageError(errorMessage);
}

void MainWindow::onCollected(QList<StreamObject> streamObjects)
{
    ui->continueButton->setEnabled(true);
    ui->streamListWidget->setStreamObjects(streamObjects);
}
