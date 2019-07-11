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

#ifndef CORE_ENGINE_H
#define CORE_ENGINE_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtNetwork/QNetworkReply>

class JobClient;
class ResourceItem;

class QSaveFile;
class QNetworkAccessManager;

class Engine : public QObject
{
    Q_OBJECT

public:
    explicit Engine(QObject *parent = 0);
    ~Engine();

    void clear();

    QList<JobClient*> jobs() const;
    JobClient* jobAt(int index) const;

    void append(JobClient *job, const bool started);
    void remove(JobClient *job);

    /* Actions */
    void resume(JobClient *job);
    void pause(JobClient *job);
    void cancel(JobClient *job);


public slots:

signals:
    void jobAppended(JobClient *job);
    void jobRemoved(JobClient *job);
    void jobStateChanged(JobClient *job);

    void downloadFinished(bool success);

private slots:
    void onMetaDataChanged();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onRedirected(const QUrl &url);
    void onFinished();
    void onError(QNetworkReply::NetworkError error);
    void onReadyRead();
    void onAboutToClose();

    void onQueueChanged(JobClient *job);

private:
    QNetworkAccessManager *m_networkManager;
    QList<JobClient*> m_jobs;

    // Pool
    int m_currentSimultaneousDownloads;
    int m_maxSimultaneousDownloads;
    inline void start(JobClient *job);

    void addReply(QNetworkReply* item, JobClient *job);
    void removeReply(QNetworkReply* item);
    QNetworkReply* getReply(JobClient *job);
    JobClient* getJob(QNetworkReply* item);
    QMap<QNetworkReply*, JobClient* > m_map; // == running jobs !

    void addFile(QSaveFile* file, JobClient *job);
    void removeFile(QSaveFile* file);
    QSaveFile* getFile(JobClient *job);
    JobClient* getJob(QSaveFile* file);
    QMap<QSaveFile*, JobClient* > m_fileMap;

};

#endif // CORE_ENGINE_H
