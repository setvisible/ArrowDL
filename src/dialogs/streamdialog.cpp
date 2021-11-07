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

#include "streamdialog.h"
#include "ui_streamdialog.h"

#include <Globals>
#include <Core/Stream>

#include <QtCore/QDebug>
#include <QtCore/QString>

StreamDialog::StreamDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StreamDialog)
{
    ui->setupUi(this);

    setWindowTitle(QString("%0 - %1").arg(STR_APPLICATION_NAME, tr("Stream Download Info")));

    adjustSize();

    connect(ui->okButton, &QPushButton::released, this, &StreamDialog::onOkButtonReleased);

    ui->title->setText(QLatin1String("YT-DLP"));
    ui->version->setText(tr("Reading..."));
    askStreamVersionAsync();

    ui->link->setText(QString("<a href=\"%0\">%0</a>").arg(Stream::website()));
    ui->link->setTextFormat(Qt::RichText);
    ui->link->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->link->setOpenExternalLinks(true);

    ui->plainTextEdit->setPlainText(tr("Collecting..."));
    ui->plainTextEdit->setEnabled(false);

    askStreamExtractorsAsync();
}

StreamDialog::~StreamDialog()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void StreamDialog::onOkButtonReleased()
{
    QDialog::accept();
}

/******************************************************************************
 ******************************************************************************/
void StreamDialog::askStreamVersionAsync()
{
    auto w = new AskStreamVersionThread();
    connect(this, &QObject::destroyed, w, &AskStreamVersionThread::stop);
    connect(w, &AskStreamVersionThread::resultReady, ui->version, &QLabel::setText);
    connect(w, &AskStreamVersionThread::finished, w, &QObject::deleteLater);
    w->start();
}

/******************************************************************************
 ******************************************************************************/
void StreamDialog::askStreamExtractorsAsync()
{
    auto w = new StreamExtractorListCollector(this);

    connect(w, &StreamExtractorListCollector::error, this, &StreamDialog::onError);
    connect(w, &StreamExtractorListCollector::collected, this, &StreamDialog::onCollected);
    connect(w, &StreamExtractorListCollector::finished, w, &QObject::deleteLater);

    w->runAsync();
}

void StreamDialog::onError(const QString &errorMessage)
{
    ui->plainTextEdit->setPlainText(QString("%0\n\n%1").arg(tr("Error:"), errorMessage));
    ui->plainTextEdit->setEnabled(true);
}

void StreamDialog::onCollected(const QStringList &extractors, const QStringList &descriptions)
{
    auto count = qMin(extractors.count(), descriptions.count());
    ui->plainTextEdit->clear();
    ui->plainTextEdit->appendPlainText(tr("YT-DLP supports %0 sites:").arg(extractors.count()));
    ui->plainTextEdit->appendPlainText(QLatin1String("===================="));
    ui->plainTextEdit->appendPlainText(QString());
    ui->plainTextEdit->appendPlainText(QString("%0\t%1").arg(tr("Site"), tr("Description")));
    ui->plainTextEdit->appendPlainText(QString());
    for (int i = 0; i < count; ++i) {
        ui->plainTextEdit->appendPlainText(QString("%0\t%1").arg(
                                               extractors.at(i),
                                               descriptions.at(i)));
    }
    for (int i = count, total = extractors.count(); i < total; ++i) {
        ui->plainTextEdit->appendPlainText(QString("%0").arg(
                                               extractors.at(i)));
    }

    ui->plainTextEdit->setEnabled(true);

    // Scroll to top
    ui->plainTextEdit->moveCursor(QTextCursor::Start);
    ui->plainTextEdit->ensureCursorVisible();
}
