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

#ifndef DUMMY_STREAM_FACTORY_H
#define DUMMY_STREAM_FACTORY_H

#include <Core/Stream>

class DummyStreamFactory
{
public:
    static StreamObject createDummyStreamObject();
    static StreamObject createDummyErrorStreamObject();

    static StreamObject createDummyStreamObject_Youtube();
    static StreamObject createDummyStreamObject_Dailymotion();
    static StreamObject createDummyStreamObject_Other();

    static StreamObject createDummyStreamObject_unavailable();

    static QByteArray dumpSingleVideo();
    static QByteArray dumpSingleVideo(const QString &id);
    static QByteArray dumpPlaylist();
    static QByteArray dumpPlaylistStandardError();

    static QByteArray flatSingleVideo();
    static QByteArray flatSingleVideo(const QString &id);
    static QByteArray flatPlaylist();

};

#endif // DUMMY_STREAM_FACTORY_H
