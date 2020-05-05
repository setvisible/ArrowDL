/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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

#include "torrenthandler.h"

#include <Core/IDownloadItem>
#include <Core/Torrent>

#include <QtCore/QDebug>
#include <QtCore/QFile>

#include <fstream>  // std::fstream


bool TorrentHandler::canRead() const
{
    return true;
}

bool TorrentHandler::canWrite() const
{
    return false;
}

static std::vector<char> hack_load_file(std::string const& filename)
{
    std::fstream in;
    in.exceptions(std::ifstream::failbit);
    in.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
    in.seekg(0, std::ios_base::end);
    size_t const size = size_t(in.tellg());
    in.seekg(0, std::ios_base::beg);
    std::vector<char> ret(size);
    in.read(ret.data(), static_cast<std::streamsize>(size));
    return ret;
}

bool TorrentHandler::read(DownloadEngine *engine)
{
    qDebug() << Q_FUNC_INFO;

    if (!engine) {
        qWarning("TorrentHandler::read() cannot read into null pointer");
        return false;
    }
    QIODevice *d = device();
    if (!d->isReadable()) {
        return false;
    }
    QUrl url;
    QFile* f = static_cast<QFile*>(d);
    if (f) {
        auto filename = f->fileName();
        url = QUrl(filename);
    }

    std::vector<char> buf = hack_load_file(url.toString().toStdString());
    QByteArray data(&buf[0], static_cast<int>(buf.size()));

    IDownloadItem *item = engine->createTorrentItem(url, data);
    if (!item) {
        qWarning("DownloadEngine::createItem() not overridden. It still returns null pointer!");
        return false;
    }

    QList<IDownloadItem*> items;
    items.append(item);
    engine->append(items, false);
    return true;
}

bool TorrentHandler::write(const DownloadEngine &/*engine*/)
{
    return false;
}
