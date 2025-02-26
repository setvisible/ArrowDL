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

#ifndef CORE_JOB_FILE_H
#define CORE_JOB_FILE_H

#include <Core/AbstractJob>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>

class File;
class Scheduler;
class ResourceItem;

// class QNetworkReply;

class JobFile : public AbstractJob
{
    Q_OBJECT

public:
    JobFile(QObject *parent, ResourceItem *resource);
    ~JobFile() override;

    void resume() override;
    void pause() override;
    void stop() override;

private slots:
    void onMetaDataChanged();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onRedirected(const QUrl &url);
    void onFinished();
    void onErrorOccurred(QNetworkReply::NetworkError error);
    void onReadyRead();
    void onAboutToClose();

private:
    Scheduler *m_scheduler = nullptr;
    QNetworkReply *m_reply = nullptr;

    QString statusToHttp(QNetworkReply::NetworkError error);
};

#endif // CORE_JOB_FILE_H
