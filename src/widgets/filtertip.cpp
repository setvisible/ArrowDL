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

#include "filtertip.h"
#include "ui_filtertip.h"

#include <QtWidgets/QLabel>

FilterTip::FilterTip(QWidget *parent) : QFrame(parent)
    , ui(new Ui::FilterTip)
{
    ui->setupUi(this);
    ui->groupBox->setLayout(new QVBoxLayout());

    ui->title->setText(QString("-= %0 =-").arg(tr("Fast Filtering")));

    add("JavaScripts", "^.*\\.(js)$");
    add("HTML pages", "^.*\\.((?:m|d)htm(l?))$");
    add("Java, Fortran, C and C++ source files", "^.*\\.(java|f77|c(?:pp)|h)$");
}

FilterTip::~FilterTip()
{
    delete ui;
}

void FilterTip::add(const QString &text, const QString &link)
{
    QLabel *label = new QLabel(QString("<a href='%1'>%1</a> : %0").arg(text, link));
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

void FilterTip::onLinkActivated(const QString& link)
{
    emit linkActivated(link);
}
