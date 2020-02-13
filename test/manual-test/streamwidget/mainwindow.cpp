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

#include "../../utils/dummystreamfactory.h"

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
        StreamInfosPtr infos = DummyStreamFactory::createDummyStreamInfos();
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

void MainWindow::onCollected(StreamInfosPtr infos)
{
    ui->continueButton->setEnabled(true);
    ui->streamWidget->showStreamInfos(infos);
}
