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

#include "torrentprogressbar.h"

#include <Constants>
#include <Widgets/CustomStyleOptionProgressBar>
#include <Widgets/Globals>

#include <QtCore/QDebug>
#include <QtGui/QColor>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtWidgets/QApplication>


/*!
 * \class TorrentProgressBar
 *
 * A 'segmented' QProgressBar.
 */
TorrentProgressBar::TorrentProgressBar(QWidget *parent) : QProgressBar(parent)
  , m_downloadedPieces(QBitArray())
{    
    setRange(MIN_PROGRESS, MAX_PROGRESS);
    setValue(MIN_PROGRESS);
}

/******************************************************************************
 ******************************************************************************/
void TorrentProgressBar::clearPieces()
{
    m_downloadedPieces.clear();
    repaint();
}

void TorrentProgressBar::setPieces(const QBitArray &downloadedPieces)
{
    m_downloadedPieces = downloadedPieces;
    repaint();
}

/******************************************************************************
 ******************************************************************************/
void TorrentProgressBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    CustomStyleOptionProgressBar myOption;
    initStyleOption(&myOption);

    myOption.palette.setColor(QPalette::All, QPalette::Window, s_white);
    myOption.palette.setColor(QPalette::All, QPalette::WindowText, s_black);
    myOption.palette.setColor(QPalette::All, QPalette::Highlight, s_lightBlue);
    myOption.palette.setColor(QPalette::All, QPalette::HighlightedText, s_black);

    auto progress = value();

    CustomStyleOptionProgressBar progressBarOption;
    progressBarOption.state = myOption.state;
    progressBarOption.direction = QApplication::layoutDirection();
    progressBarOption.rect = myOption.rect;
    progressBarOption.fontMetrics = QApplication::fontMetrics();
    progressBarOption.minimum = MIN_PROGRESS;
    progressBarOption.maximum = MAX_PROGRESS;
    progressBarOption.textAlignment = Qt::AlignCenter;
    progressBarOption.textVisible = false;
    progressBarOption.palette = myOption.palette;
    progressBarOption.progress = progress;
    progressBarOption.color = progress < MAX_PROGRESS ? s_green : s_darkGreen;
    progressBarOption.icon = {};

    progressBarOption.hasSegments = true;
    progressBarOption.segments = m_downloadedPieces;

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, &painter);
}
