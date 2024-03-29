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

#ifndef WIDGETS_GLOBALS_H
#define WIDGETS_GLOBALS_H

#include <QtGui/QColor>

/* Constant */

// Light Mode
static const QColor s_darkGreen     = QColor(0, 143, 0);
static const QColor s_darkGrey      = QColor(160, 160, 160);
static const QColor s_darkRed       = QColor(177, 40, 1);
static const QColor s_darkPurple    = QColor(64, 32, 64);
static const QColor s_darkYellow    = QColor(191, 191, 55);
static const QColor s_green         = QColor(170, 224, 97);
static const QColor s_grey          = QColor(200, 200, 200);
static const QColor s_lightGrey     = QColor(240, 240, 240);
static const QColor s_lightYellow   = QColor(255, 255, 179);
static const QColor s_orange        = QColor(255, 204, 0);
static const QColor s_red           = QColor(205, 0, 0);
static const QColor s_purple        = QColor(200, 191, 231);


// Dark Mode
static const QColor s_dark_lightYellow = QColor(85, 85, 60);
static const QColor s_dark_darkYellow  = QColor(113, 113, 37);
static const QColor s_darkOrange    = QColor(128, 102, 0);

static const char* const s_xpm[] = {
    "16 16 2 1",
    "   c #F0F0F0",         // light grey
    "+  c #AAE061",         // light green
    "++++++++        ",
    " ++++++++       ",
    "  ++++++++      ",
    "   ++++++++     ",
    "    ++++++++    ",
    "     ++++++++   ",
    "      ++++++++  ",
    "       ++++++++ ",
    "        ++++++++",
    "+        +++++++",
    "++        ++++++",
    "+++        +++++",
    "++++        ++++",
    "+++++        +++",
    "++++++        ++",
    "+++++++        +"};


#endif // WIDGETS_GLOBALS_H
