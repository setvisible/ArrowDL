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

#include <functional> /* std::function */

#include <Core/Settings>
#include <CAutoUpdaterGithub>

#include <QtCore/QObject>

class QNetworkReply;

class NetworkManager;
class StreamUpgrader;

class UpdateChecker : public QObject, private CAutoUpdaterGithub::UpdateStatusListener
{
    Q_OBJECT

public:
    explicit UpdateChecker(QObject *parent);
    ~UpdateChecker() Q_DECL_OVERRIDE = default;

    QString currentVersion() const;

    void setNetworkManager(NetworkManager *networkManager);

    void checkForUpdates(const Settings *settings);
    void checkForUpdates();

    QString installTempDir() const;
    void downloadAndInstallUpdate();

    QString latestUpdateUrl() const;

signals:
    void updateAvailable();
    void updateAvailable(CAutoUpdaterGithub::ChangeLog changelog);
    void updateDownloadProgress(float percentageDownloaded);
    void updateDownloadFinished();
    void updateError(QString errorMessage);

private:
    QString m_latestUpdateUrl;
    CAutoUpdaterGithub m_updater;
    StreamUpgrader *m_streamUpgrader;

    void onUpdateAvailable(CAutoUpdaterGithub::ChangeLog changelog) Q_DECL_OVERRIDE;
    void onUpdateDownloadProgress(float percentageDownloaded) Q_DECL_OVERRIDE;
    void onUpdateDownloadFinished() Q_DECL_OVERRIDE;
    void onUpdateError(QString errorMessage) Q_DECL_OVERRIDE;

    static const std::function<QNetworkReply* (const QUrl&)> createNetworkGetCallback();
    static const std::function<bool (const QString&)> createAddressMatcherCallback(bool isHost64Bit);

    inline qint64 daysSinceLastCheck();
    inline void storeDate();
};

#endif // CORE_UPDATE_CHECKER_H
