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

#ifndef CORE_DOWNLOAD_STREAM_ITEM_H
#define CORE_DOWNLOAD_STREAM_ITEM_H

#include <Core/DownloadItem>

#include <QtCore/QObject>
#include <QtCore/QString>

class DownloadManager;
class Stream;

class DownloadStreamItem : public DownloadItem
{
    Q_OBJECT

public:
    DownloadStreamItem(DownloadManager *downloadManager);
    ~DownloadStreamItem() Q_DECL_OVERRIDE = default;

    void resume() Q_DECL_OVERRIDE;
    void pause() Q_DECL_OVERRIDE;
    void stop() Q_DECL_OVERRIDE;

private slots:
    void onMetaDataChanged();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onFinished();
    void onError(const QString &errorMessage);

private:
    Stream *m_stream;
};

#endif // CORE_DOWNLOAD_STREAM_ITEM_H
