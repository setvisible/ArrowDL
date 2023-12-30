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

#ifndef CORE_UPDATE_CHECKER_H
#define CORE_UPDATE_CHECKER_H

#include <QtCore/QFile>
#include <QtCore/QObject>

class QNetworkReply;

class NetworkManager;
class StreamUpgrader;
class Settings;

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    struct ReleaseInfo {
        QString tagName;
        QString title;
        bool draft{false};
        bool prerelease{false};
        QString createdAt;
        QString publishedAt;
        QString body;
        QString assetName;
        QString assetUrl;
        QString assetCreatedAt;
        int assetSize{0};
    };
    using ChangeLog = QVector<ReleaseInfo>;

    explicit UpdateChecker(QObject *parent);
    virtual ~UpdateChecker() = default;


    void setNetworkManager(NetworkManager *networkManager);

    void checkForUpdates(const Settings *settings);
    void checkForUpdates();

    void downloadAndInstallUpdate();

    QString currentVersion() const;
    QString tempPath() const;

    QString latestUpdateUrl() const; // For Linux

signals:
    void updateAvailableForConsole(); // for non-GUI
    void updateAvailableForGui(UpdateChecker::ChangeLog changelog);
    void downloadProgress(qsizetype bytesReceived, qsizetype bytesTotal);
    void updateDownloadFinished();
    void updateError(QString errorMessage);

private slots:
    void onMetadataFinished();
    void onBinaryProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onBinaryFinished();

private:
    StreamUpgrader *m_streamUpgrader;
    QString m_latestUpdateUrl;
    QFile m_temporaryBinaryFile;

    void downloadMetadata();

    inline qint64 daysSinceLastCheck();
    inline void storeDateTime();
};

#endif // CORE_UPDATE_CHECKER_H
