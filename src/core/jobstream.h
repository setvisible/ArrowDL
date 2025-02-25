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

#ifndef CORE_JOB_STREAM_H
#define CORE_JOB_STREAM_H

#include <Core/AbstractDownloadItem>

#include <QtCore/QObject>
#include <QtCore/QString>

class DownloadManager;
// class ResourceItem;
class Stream;

class JobStream : public AbstractDownloadItem
{
    Q_OBJECT

public:
    JobStream(QObject *parent, ResourceItem *resource);
    ~JobStream();

    void resume() override;
    void pause() override;
    void stop() override;

private slots:
    void onMetaDataChanged();
    void onDownloadProgress(qsizetype bytesReceived, qsizetype bytesTotal);
    void onFinished();
    void onError(const QString &errorMessage);

private:
    Stream *m_stream = nullptr;
};

#endif // CORE_JOB_STREAM_H
