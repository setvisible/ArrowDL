/* - ArrowDL - Copyright (C) 2019-present Sebastien Vavassori
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

#ifndef UTILS_TORRENT_ANIMATOR_H
#define UTILS_TORRENT_ANIMATOR_H

#include <QtCore/QObject>
#include <QtCore/QTimer>

namespace Utils {
int randomBetween(int a, int b);
int randomBetweenLog(int a, int b);
}

class Torrent;

class DummyTorrentAnimator : public QObject
{
    Q_OBJECT
public:
    DummyTorrentAnimator(QObject *parent = nullptr);

    Torrent* torrent() const;
    void setTorrent(Torrent *torrent);

    void setProgress(int percent);

    bool isStarted() const;
    void start();
    void stop();

signals:
    void started(bool started);

private slots:
    void animate();

private:
    Torrent *m_torrent{nullptr};
    QTimer m_fileTimer;
    QTimer m_peerTimer;
    QList<int> m_timeouts;
    QList<int> m_ticks;

    void randomize();
    void animateFile(int index);
    void animatePieces();
    void animatePeers();

    void setPiecesRandomly(QBitArray &pieces);
    QBitArray createRandomBitArray(int size, int percent);
};

#endif // UTILS_TORRENT_ANIMATOR_H
