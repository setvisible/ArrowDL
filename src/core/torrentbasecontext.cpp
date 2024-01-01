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

#include "torrentbasecontext.h"

#include <Core/Torrent>

const double ONE_THIRD = 0.3334;
const double TWO_THIRDS = 0.6667;

void TorrentBaseContext::setPriority(Torrent *torrent, int fileIndex, TorrentFileInfo::Priority p)
{
    Q_ASSERT(torrent);
    torrent->setFilePriority(fileIndex, p);
}

void TorrentBaseContext::setPriorityByFileOrder(Torrent *torrent, const QList<int> &fileIndexes)
{
    Q_ASSERT(torrent);
    auto fileCount = torrent->fileCount();
    for (auto fileIndex : fileIndexes) {
        auto priority = TorrentBaseContext::computePriority(fileIndex, fileCount);
        setPriority(torrent, fileIndex, priority);
    }
}

TorrentFileInfo::Priority TorrentBaseContext::computePriority(int row, qsizetype count)
{
    if (count < 3) {
        return TorrentFileInfo::Normal;
    }
    auto pos = static_cast<qreal>(row + 1) / static_cast<qreal>(count);
    if (pos < ONE_THIRD) {
        return TorrentFileInfo::High;
    }
    if (pos < TWO_THIRDS) {
        return TorrentFileInfo::Normal;
    }
    return TorrentFileInfo::Low;
}
