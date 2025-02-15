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

#include "texthandler.h"

#include <Core/AbstractDownloadItem>

#include <QtCore/QDebug>
#include <QtCore/QIODevice>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>


bool TextHandler::canRead() const
{
    return true;
}

bool TextHandler::canWrite() const
{
    return true;
}

static bool readLineInto(QTextStream &in, QString *line) // for Qt 5.4.1
{
    *line = in.readLine();
    return !line->isNull();
}

bool TextHandler::read(IDownloadManager *downloadManager)
{
    if (!downloadManager) {
        qWarning("TextHandler::read() cannot read into null pointer");
        return false;
    }
    QIODevice *d = device();
    QTextStream in(d);
    in.setEncoding(QStringConverter::Utf8);
    if (!d->isReadable()) {
        return false;
    }
    QList<AbstractDownloadItem*> items;
    QString line;
    while (readLineInto(in, &line)) {
        line = line.simplified();
        if (line.isEmpty()) {
            continue;
        }
        const QUrl url(line);
        AbstractDownloadItem *item = downloadManager->createFileItem(url);
        if (!item) {
            qWarning("DownloadManager::createFileItem() not overridden. It still returns null pointer!");
            return false;
        }
        items.append(item);
    }
    downloadManager->append(items, false);
    return true;
}

bool TextHandler::write(const IDownloadManager &downloadManager)
{
    QIODevice *d = device();
    QTextStream out(d);
    out.setEncoding(QStringConverter::Utf8);
    if (!d->isWritable()) {
        return false;
    }
    for (auto item : downloadManager.downloadItems()) {
        QUrl url = item->sourceUrl();
        QByteArray data = url.toString().toUtf8();
        out << data << '\n';
    }
    return true;
}
