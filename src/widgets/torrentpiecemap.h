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

#ifndef WIDGETS_TORRENT_PIECE_MAP_H
#define WIDGETS_TORRENT_PIECE_MAP_H

#include <QtWidgets/QWidget>

class Torrent;

namespace Ui {
class TorrentPieceMap;
}

class TorrentPieceMap : public QWidget
{
    Q_OBJECT
public:
    explicit TorrentPieceMap(QWidget *parent);
    ~TorrentPieceMap() Q_DECL_OVERRIDE;

    void clear();
    bool isEmpty() const;

    Torrent* torrent() const;
    void setTorrent(Torrent *torrent);

protected slots:
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onChanged();

private:
    Ui::TorrentPieceMap *ui;
    Torrent *m_torrent;

    void resetUi();
    void retranslateUi();
};

#endif // WIDGETS_TORRENT_PIECE_MAP_H
