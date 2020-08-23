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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Core/Stream>

#include <QtWidgets/QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow();

private slots:
    void onContinueClicked();
    void onResetClicked();
    void onError(QString errorMessage);
    void onCollected(QList<StreamInfo> streamInfoList);

    void onEmptyButtonClicked();
    void onErrorButtonClicked();
    void onYoutubeButtonClicked();
    void onDailymotionButtonClicked();
    void onOtherSiteButtonClicked();
    void onUrlMp4ButtonClicked();
    void onPlaylistButtonClicked();

private:
    Ui::MainWindow *ui;
    StreamInfoDownloader *m_streamInfoDownloader;

    void start(const QString &url = QString());
};

#endif // MAINWINDOW_H
