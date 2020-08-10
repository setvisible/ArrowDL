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

#include <Core/Stream>
#include <Widgets/StreamWidget>

#include <QtCore/QDebug>
#include <QtWidgets/QPushButton>

static const QString s_urlMp4Link = "http://camendesign.com/code/video_for_everybody/test.html";


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_streamInfoDownloader(new StreamInfoDownloader(this))
{
    ui->setupUi(this);

    ui->lineEdit->setText("https://www.youtube.com/watch?v=C0DPdy98e4c");

    connect(ui->resetButton, SIGNAL(released()), this, SLOT(onResetClicked()));
    connect(ui->continueButton, SIGNAL(released()), this, SLOT(onContinueClicked()));

    connect(ui->emptyButton,       SIGNAL(released()), this, SLOT(onEmptyButtonClicked()));
    connect(ui->youtubeButton,     SIGNAL(released()), this, SLOT(onYoutubeButtonClicked()));
    connect(ui->dailymotionButton, SIGNAL(released()), this, SLOT(onDailymotionButtonClicked()));
    connect(ui->otherSiteButton,   SIGNAL(released()), this, SLOT(onOtherSiteButtonClicked()));
    connect(ui->urlMp4Button,      SIGNAL(released()), this, SLOT(onUrlMp4ButtonClicked()));

    ui->urlMp4Button->setToolTip(s_urlMp4Link);

    connect(m_streamInfoDownloader, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(m_streamInfoDownloader, SIGNAL(collected(StreamInfoPtr)), this, SLOT(onCollected(StreamInfoPtr)));

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
        ui->streamWidget->setState(StreamWidget::Downloading);
        m_streamInfoDownloader->runAsync(url);
    } else {
        onEmptyButtonClicked();
    }
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onEmptyButtonClicked()
{
    auto info = DummyStreamFactory::createDummyStreamInfo();
    ui->streamWidget->showStreamInfo(info);
}

void MainWindow::onYoutubeButtonClicked()
{
    auto info = DummyStreamFactory::createDummyStreamInfo_Youtube();
    ui->streamWidget->showStreamInfo(info);
}

void MainWindow::onDailymotionButtonClicked()
{
    auto info = DummyStreamFactory::createDummyStreamInfo_Dailymotion();
    ui->streamWidget->showStreamInfo(info);
}

void MainWindow::onOtherSiteButtonClicked()
{
    auto info = DummyStreamFactory::createDummyStreamInfo_Other();
    ui->streamWidget->showStreamInfo(info);
}

void MainWindow::onUrlMp4ButtonClicked()
{
    start(s_urlMp4Link);
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onError(QString errorMessage)
{
    ui->continueButton->setEnabled(true);
    ui->streamWidget->showErrorMessage(errorMessage);
}

void MainWindow::onCollected(StreamInfoPtr info)
{
    ui->continueButton->setEnabled(true);
    ui->streamWidget->showStreamInfo(info);
}
