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

#include "torrentpiecemap.h"
#include "ui_torrentpiecemap.h"

#include <Core/Torrent>

#include <QtCore/QDebug>

TorrentPieceMap::TorrentPieceMap(QWidget *parent) : QWidget(parent)
  , ui(new Ui::TorrentPieceMap)
  , m_torrent(Q_NULLPTR)
{
    ui->setupUi(this);
    resetUi();
}

TorrentPieceMap::~TorrentPieceMap()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void TorrentPieceMap::clear()
{
    m_torrent = Q_NULLPTR;
    resetUi();
}

bool TorrentPieceMap::isEmpty() const
{
    return m_torrent == Q_NULLPTR;
}

/******************************************************************************
 ******************************************************************************/
Torrent *TorrentPieceMap::torrent() const
{
    return m_torrent;
}

void TorrentPieceMap::setTorrent(Torrent *torrent)
{
    if (m_torrent == torrent) {
        return;
    }
    if (m_torrent) {
        disconnect(m_torrent, &Torrent::changed, this, &TorrentPieceMap::onChanged);
    }
    m_torrent = torrent;
    if (m_torrent) {
        connect(m_torrent, &Torrent::changed, this, &TorrentPieceMap::onChanged);
    }
    resetUi();
}

/******************************************************************************
 ******************************************************************************/
void TorrentPieceMap::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

/******************************************************************************
 ******************************************************************************/
void TorrentPieceMap::onChanged()
{
}

void TorrentPieceMap::resetUi()
{
}

void TorrentPieceMap::retranslateUi()
{
}
