/* - DownZemAll! - Copyright (C) 2019 Sebastien Vavassori
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

#include "autoclosedialog.h"

#include <QtWidgets/QFrame>
#include <QtWidgets/QVBoxLayout>

/*!
 * \class AutoCloseDialog
 *
 * An extended QDialog that closes automatically when the mouse leaves.
 *
 */
AutoCloseDialog::AutoCloseDialog(QFrame *content, QWidget *parent) : QDialog(parent)
{
    setWindowModality(Qt::NonModal);
    setWindowFlags(Qt::Popup);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    content->setFrameShape(QFrame::Panel);
    content->setFrameShadow(QFrame::Raised);

    auto layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(content);
    setLayout(layout);

    QRect rect = geometry();
    const QSize minAcceptableSize = QLayout::closestAcceptableSize(this, QSize(0,0));
    rect.setSize(minAcceptableSize);
    if (parent) {
        // aligned to parent widget
        const QPoint parentTopRight = parent->mapToGlobal(QPoint(parent->width() - 5, -5));
        rect.moveBottomRight(parentTopRight);
    }
    setGeometry(rect);
}

void AutoCloseDialog::leaveEvent(QEvent * /*event*/)
{
    this->close();
}
