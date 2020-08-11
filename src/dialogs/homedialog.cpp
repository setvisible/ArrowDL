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

#include "homedialog.h"
#include "ui_homedialog.h"

#include <Globals>

#include <QtWidgets/QCommandLinkButton>
#include <QtWidgets/QPushButton>


HomeDialog::HomeDialog(QWidget *parent) : QDialog(parent)
  , ui(new Ui::HomeDialog)
{
    ui->setupUi(this);

    setWindowTitle(QString("%0 - %1").arg(STR_APPLICATION_NAME).arg(tr("Getting Started")));

    adjustSize();
    setFixedSize(size());

    connect(ui->buttonContent, &QPushButton::released, this, &HomeDialog::acceptContent);
    connect(ui->buttonBatch, &QPushButton::released, this, &HomeDialog::acceptBatch);
    connect(ui->buttonStream, &QPushButton::released, this, &HomeDialog::acceptStream);
    connect(ui->buttonTorrent, &QPushButton::clicked, this, &HomeDialog::acceptTorrent);
    connect(ui->cancelButton, &QPushButton::released, this, &HomeDialog::reject);
}

HomeDialog::~HomeDialog()
{
    delete ui;
}

void HomeDialog::acceptContent()
{
    done(static_cast<int>(HomeDialog::Content));
}

void HomeDialog::acceptBatch()
{
    done(static_cast<int>(HomeDialog::Batch));
}

void HomeDialog::acceptStream()
{
    done(static_cast<int>(HomeDialog::Stream));
}

void HomeDialog::acceptTorrent()
{
    done(static_cast<int>(HomeDialog::Torrent));
}
