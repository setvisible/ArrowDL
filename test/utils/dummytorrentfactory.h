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

#ifndef DUMMY_TORRENT_FACTORY_H
#define DUMMY_TORRENT_FACTORY_H

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>

#include <Core/Torrent>

using TorrentPtr = QSharedPointer<Torrent>;

class DummyTorrentFactory
{
public:
    static TorrentPtr createDummyTorrent(QObject *parent);

    static TorrentPeerInfo createDummyPeer(
            const EndPoint &endpoint, const QString &pieceSketch, const QString &userAgent,
            qint64 size);
    static TorrentPeerInfo createDummyPeer2(
            const EndPoint &endpoint, const QString &pieceSketch, const QString &userAgent,
            qint64 size, qint64 bytesDownloaded, qint64 bytesUploaded);
};

#endif // DUMMY_TORRENT_FACTORY_H
