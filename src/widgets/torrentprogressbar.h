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

#ifndef WIDGETS_TORRENT_PROGRESS_BAR_H
#define WIDGETS_TORRENT_PROGRESS_BAR_H

#include <QtWidgets/QProgressBar>
#include <QtCore/QBitArray>

class TorrentProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    explicit TorrentProgressBar(QWidget *parent = Q_NULLPTR);
    ~TorrentProgressBar() Q_DECL_OVERRIDE = default;

    void clearPieces();
    void setPieces(const QBitArray &downloadedPieces);

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

private:
    QBitArray m_downloadedPieces;
};

#endif // WIDGETS_TORRENT_PROGRESS_BAR_H
