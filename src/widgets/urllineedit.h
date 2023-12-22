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

#ifndef WIDGETS_URL_LINE_EDIT_H
#define WIDGETS_URL_LINE_EDIT_H

#include <QtWidgets/QLineEdit>

class UrlLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit UrlLineEdit(QWidget *parent = Q_NULLPTR);
    ~UrlLineEdit() override = default;
};

#endif // WIDGETS_URL_LINE_EDIT_H
