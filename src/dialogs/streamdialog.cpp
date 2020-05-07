/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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

    adjustSize();

    ui->title->setText(QLatin1String("Youtube-DL"));
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
void StreamDialog::on_okButton_released()
{
    QDialog::reject();
}

/******************************************************************************
 ******************************************************************************/
void StreamDialog::askStreamVersionAsync()
{
    AskStreamVersionThread *w = new AskStreamVersionThread();
    connect(this, &QObject::destroyed, w, &AskStreamVersionThread::stop);
    connect(w, &AskStreamVersionThread::resultReady, ui->version, &QLabel::setText);
    connect(w, &AskStreamVersionThread::finished, w, &QObject::deleteLater);
    w->start();
}

/******************************************************************************
 ******************************************************************************/
void StreamDialog::askStreamExtractorsAsync()
{
    StreamExtractorListCollector *w = new StreamExtractorListCollector(this);

    connect(w, &StreamExtractorListCollector::error, this, &StreamDialog::onError);
    connect(w, &StreamExtractorListCollector::collected, this, &StreamDialog::onCollected);
    connect(w, &StreamExtractorListCollector::finished, w, &QObject::deleteLater);

    w->runAsync();
}

void StreamDialog::onError(QString errorMessage)
{
    ui->plainTextEdit->setPlainText(tr("Error:\n\n%0").arg(errorMessage));
    ui->plainTextEdit->setEnabled(true);
}

void StreamDialog::onCollected(QStringList extractors, QStringList descriptions)
{
    auto count = qMin(extractors.count(), descriptions.count());
    qDebug() << Q_FUNC_INFO << extractors.count() << descriptions.count();

    ui->plainTextEdit->clear();
    ui->plainTextEdit->appendPlainText(tr("Youtube-DL supports %0 sites:")
                                       .arg(extractors.count()));
    ui->plainTextEdit->appendPlainText(QLatin1String("===================="));
    ui->plainTextEdit->appendPlainText(QString());
    ui->plainTextEdit->appendPlainText(tr("Site\tDescription"));
    ui->plainTextEdit->appendPlainText(QString());
    for (int i = 0; i < count; ++i) {
        ui->plainTextEdit->appendPlainText(
                    QString("%0\t%1")
                    .arg(extractors.at(i))
                    .arg(descriptions.at(i)));
    }
    for (int i = count, total = extractors.count(); i < total; ++i) {
        ui->plainTextEdit->appendPlainText(
                    QString("%0")
                    .arg(extractors.at(i)));
    }

    ui->plainTextEdit->setEnabled(true);

    // Scroll to top
    ui->plainTextEdit->moveCursor(QTextCursor::Start);
    ui->plainTextEdit->ensureCursorVisible();
}
