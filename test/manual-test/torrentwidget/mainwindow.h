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

#include <QtWidgets/QMainWindow>

class ITorrentContext;
class Torrent;
typedef QSharedPointer<Torrent> TorrentPtr;

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
    void onStartClicked();
    void onZeroPercentClicked();
    void onCompletedClicked();
    void onRandomClicked();
    void onHalfClicked();

    void animate();

private:
    Ui::MainWindow *ui;
    ITorrentContext *m_torrentContext;

    TorrentPtr m_torrent;
    QList<int> m_timeouts;
    QList<int> m_ticks;
    QTimer *m_timer;

    void setupTorrent();
    void randomize();
    void startAnimation();
    void stopAnimation();

    void animateFile(int index);
};

#endif // MAINWINDOW_H
