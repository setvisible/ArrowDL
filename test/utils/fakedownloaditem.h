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

#ifndef FAKE_DOWNLOAD_ITEM_H
#define FAKE_DOWNLOAD_ITEM_H

#include <Core/AbstractDownloadItem>

#include <QtCore/QObject>
#include <QtCore/QString>

#include <QtCore/QTimer>

class FakeDownloadItem : public AbstractDownloadItem
{
    Q_OBJECT

public:
    explicit FakeDownloadItem(QObject *parent = Q_NULLPTR);
    explicit FakeDownloadItem(QUrl url, QString filename,
                              qint64 bytesTotal, qint64 timeIncrement, qint64 duration,
                              QObject *parent= Q_NULLPTR);

    ~FakeDownloadItem();

    virtual QUrl sourceUrl() const Q_DECL_OVERRIDE;
    virtual QString localFileName() const Q_DECL_OVERRIDE;

    virtual QString localFullFileName() const Q_DECL_OVERRIDE;
    virtual QString localFilePath() const Q_DECL_OVERRIDE;

    virtual QUrl localFileUrl() const Q_DECL_OVERRIDE;
    virtual QUrl localDirUrl() const Q_DECL_OVERRIDE;

public slots:
    virtual void resume() Q_DECL_OVERRIDE;
    virtual void pause() Q_DECL_OVERRIDE;
    virtual void stop() Q_DECL_OVERRIDE;

private slots:

public slots:
    void simulateNetworkError();

private slots:
    void tickFakeStream();

private:
    /* Simulate a stream of data */
    QUrl m_resourceUrl;
    QString m_resourceLocalFileName;

    qint64 m_simulationBytesTotal;
    qint64 m_simulationTimeIncrement;
    qint64 m_simulationTimeDuration;

    bool m_isSimulateFileErrorEnabled;
    bool m_isSimulateFileErrorAtTheEndEnabled;
    int  m_simulateHttpErrorNumber;

    QTimer m_fakeStreamTimer;

    void init();
};

#endif // FAKE_DOWNLOAD_ITEM_H
