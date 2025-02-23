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

#ifndef CORE_ABSTRACT_DOWNLOAD_ITEM_H
#define CORE_ABSTRACT_DOWNLOAD_ITEM_H

#include <QtCore/QElapsedTimer>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QTime>

class File;
class ResourceItem;

class QTimer;

/*!
 * \class AbstractDownloadItem
 *
 * The class AbstractDownloadItem implements the most common methods of
 * AbstractDownloadItem and the Signal/Slot mechanism.
 *
 */
class AbstractDownloadItem : public QObject
{
    Q_OBJECT

public:
    enum State {
        Idle,
        Paused,
        Stopped,
        Preparing,
        Connecting,
        DownloadingMetadata,
        Downloading,
        Endgame,
        Completed,
        Seeding,
        Skipped,
        NetworkError,
        FileError
    };

    explicit AbstractDownloadItem(QObject *parent, ResourceItem *resource);
    ~AbstractDownloadItem();

    virtual State state() const;
    void setState(State state);
    QString stateToString() const;
    const char* state_c_str() const;

    virtual qsizetype bytesReceived() const; /*!< in bytes */
    void setBytesReceived(qsizetype bytesReceived);

    virtual qsizetype bytesTotal() const;  /*!< in bytes */
    void setBytesTotal(qsizetype bytesTotal);

    virtual qreal speed() const; /*!< Returns the speed in byte per second */
    virtual int progress() const; /*!< Return a value between 0 and 100, or -1 if undefined */

    QString errorMessage() const;
    void setErrorMessage(const QString &message);

    virtual int maxConnections() const;
    void setMaxConnections(int connections);

    virtual QString log() const;
    void setLog(const QString &log);
    void logInfo(const QString &message);       

    /* Resource to download */
    ResourceItem* resource() const;
   // virtual void setResource(ResourceItem *resource);

    virtual QUrl sourceUrl() const;
    virtual void setSourceUrl(const QUrl &url);

    virtual QString localFullFileName() const;
    virtual QString localFileName() const;
    virtual QString localFilePath() const;
    virtual QUrl localFileUrl() const;
    virtual QUrl localDirUrl() const;

    virtual bool isResumable() const;
    virtual bool isPausable() const;
    virtual bool isCancelable() const;
    virtual bool isDownloading() const;

    QTime remainingTime() const;

    virtual void setReadyToResume();
    virtual void resume();
    virtual void pause();
    virtual void stop();

    void beginResume();
    bool checkResume(bool connected);
    void tearDownResume();
    void preFinish(bool commited);

    void finish();

    virtual void rename(const QString &newName);
    virtual void moveToTrash();

signals:
    void changed();
    void finished();
    void renamed(QString oldName, QString newName, bool success);

public slots:
    void updateInfo(qsizetype bytesReceived, qsizetype bytesTotal);

private slots:
    void updateInfo();

protected:
    ResourceItem *m_resource = nullptr;
    File *m_file = nullptr;

private:
    State m_state = State::Idle;

    qreal m_speed = -1;
    qsizetype m_bytesReceived = 0;
    qsizetype m_bytesTotal = 0;

    QString m_errorMessage = {};

    int m_maxConnections = 1;

    QString m_log = {};

    QElapsedTimer m_downloadElapsedTimer = {};
    QTime m_remainingTime = {};
    QTimer* m_updateInfoTimer = nullptr;
    QTimer* m_updateCountDownTimer = nullptr;
};

/* Enable the type to be used with QVariant. */
Q_DECLARE_METATYPE(AbstractDownloadItem*)

#endif // CORE_ABSTRACT_DOWNLOAD_ITEM_H
