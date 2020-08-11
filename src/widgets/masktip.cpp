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

#include "masktip.h"
#include "ui_masktip.h"

#include <Core/Mask>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtCore/QDebug>


MaskTip::MaskTip(QWidget *parent) : QFrame(parent)
    , ui(new Ui::MaskTip)
{
    ui->setupUi(this);

    ui->groupBox->setLayout(new QVBoxLayout());

    foreach (auto tag, Mask::tags()) {
        add(Mask::description(tag), tag);
    }
}

MaskTip::~MaskTip()
{
    delete ui;
}

void MaskTip::add(const QString &text, const QString &link)
{
    QLabel *label = new QLabel(QString("<a href='%1'>%1</a> : %0").arg(text).arg(link));
    label->setTextFormat(Qt::RichText);
    label->setCursor(Qt::PointingHandCursor);
    label->setTextInteractionFlags(label->textInteractionFlags()
                                   | Qt::TextSelectableByMouse
                                   | Qt::LinksAccessibleByMouse
                                   | Qt::LinksAccessibleByKeyboard);
    ui->groupBox->layout()->addWidget(label);

    connect(label, SIGNAL(linkActivated(QString)),
            this, SLOT(onLinkActivated(QString)));
}

void MaskTip::onLinkActivated(const QString& link)
{
    emit linkActivated(link);
}

