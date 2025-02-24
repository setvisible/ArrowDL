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

#ifndef FAKE_FAKE_JOB_H
#define FAKE_FAKE_JOB_H

#include <Core/AbstractJob>

#include <QtCore/QObject>
#include <QtCore/QString>

#include <QtCore/QTimer>

class ResourceItem;

class FakeJob : public AbstractJob
{
    Q_OBJECT

public:
    explicit FakeJob(QObject *parent, ResourceItem *resource);
    explicit FakeJob(QString localFileName, QObject *parent = nullptr);
    explicit FakeJob(QUrl url,
                     QString filename,
                     qsizetype bytesTotal,
                     qint64 timeIncrement,
                     qint64 duration,
                     QObject *parent = nullptr);

    virtual QUrl sourceUrl() const override;
    void setSourceUrl(const QUrl &resourceUrl);

    virtual QString localFileName() const override;

    virtual QString localFullFileName() const override;
    virtual QString localFilePath() const override;

    virtual QUrl localFileUrl() const override;
    virtual QUrl localDirUrl() const override;

public slots:
    virtual void resume() override;
    virtual void pause() override;
    virtual void stop() override;

private slots:

public slots:
    void simulateNetworkError();

private slots:
    void tickFakeStream();

private:
    /* Simulate a stream of data */
    QUrl m_resourceUrl;
    QString m_resourceLocalFileName;

    qsizetype m_simulationBytesTotal;
    qint64 m_simulationTimeIncrement;
    qint64 m_simulationTimeDuration;

    bool m_isSimulateFileErrorEnabled;
    bool m_isSimulateFileErrorAtTheEndEnabled;
    int  m_simulateHttpErrorNumber;

    QTimer m_fakeStreamTimer;

    void init();
};

#endif // FAKE_FAKE_JOB_H
