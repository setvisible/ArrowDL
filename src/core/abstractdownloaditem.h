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

#ifndef CORE_ABSTRACT_DOWNLOAD_ITEM_H
#define CORE_ABSTRACT_DOWNLOAD_ITEM_H

#include <Core/IDownloadItem>

#include <QtCore/QElapsedTimer>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QTime>
#include <QtCore/QTimer>

class AbstractDownloadItem : public QObject, public IDownloadItem
{
    Q_OBJECT

public:
    explicit AbstractDownloadItem(QObject *parent = nullptr);
    ~AbstractDownloadItem() noexcept override = default; // IMPORTANT: virtual destructor

    State state() const override;
    void setState(State state);
    QString stateToString() const;
    const char* state_c_str() const;

    qsizetype bytesReceived() const override;
    void setBytesReceived(qsizetype bytesReceived);

    qsizetype bytesTotal() const override;
    void setBytesTotal(qsizetype bytesTotal);

    qreal speed() const override;
    int progress() const override;

    QString errorMessage() const;
    void setErrorMessage(const QString &message);

    int maxConnectionSegments() const override;
    void setMaxConnectionSegments(int connectionSegments);

    int maxConnections() const override;
    void setMaxConnections(int connections);

    QString log() const override;
    void setLog(const QString &log);
    void logInfo(const QString &message);

    bool isResumable() const override;
    bool isPausable() const override;
    bool isCancelable() const override;
    bool isDownloading() const override;

    QTime remainingTime();

    void setReadyToResume() override;

    void pause() override;
    void stop() override;

    void beginResume();
    bool checkResume(bool connected);
    void tearDownResume();
    void preFinish(bool commited);

    void finish();

    virtual void rename(const QString &newName);

signals:
    void changed();
    void finished();
    void renamed(QString oldName, QString newName, bool success);

public slots:
    void updateInfo(qsizetype bytesReceived, qsizetype bytesTotal);

private slots:
    void updateInfo();

private:
    State m_state = State::Idle;

    qreal m_speed = -1;
    qsizetype m_bytesReceived = 0;
    qsizetype m_bytesTotal = 0;

    QString m_errorMessage = {};

    int m_maxConnectionSegments = 4;
    int m_maxConnections = 1;

    QString m_log = {};

    QElapsedTimer m_downloadElapsedTimer;
    QTime m_remainingTime;
    QTimer m_updateInfoTimer;
    QTimer m_updateCountDownTimer;
};

#endif // CORE_ABSTRACT_DOWNLOAD_ITEM_H
