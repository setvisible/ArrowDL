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

#include "updatechecker.h"
#include "updatechecker_p.h"

#include <Globals>
#include <Core/NetworkManager>
#include <Core/Settings>
#include <Core/Stream>
#include <Core/UpdateInstaller>

#include <QtCore/QDebug>
#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>

static NetworkManager *s_networkManager;


UpdateChecker::UpdateChecker(QObject *parent) : QObject(parent)
  , m_streamUpgrader(new StreamUpgrader(this))
{
}

/******************************************************************************
 ******************************************************************************/
QString UpdateChecker::currentVersion() const
{
    return STR_APPLICATION_VERSION;
}

/******************************************************************************
 ******************************************************************************/
void UpdateChecker::setNetworkManager(NetworkManager *networkManager)
{
    s_networkManager = networkManager;
}

/******************************************************************************
 ******************************************************************************/
void UpdateChecker::checkForUpdates(const Settings *settings)
{
    if (settings) {
        auto mode = settings->checkUpdateBeatMode();
        auto days = daysSinceLastCheck();
        if ( (mode == CheckUpdateBeatMode::OnceADay && days > 1) ||
             (mode == CheckUpdateBeatMode::OnceAWeek && days > 7)) {
            checkForUpdates();
        }
    }
}

void UpdateChecker::checkForUpdates()
{
    downloadMetadata();

    /* In parallel, also update 3rd-party software */
    m_streamUpgrader->runAsync();
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Get the metadata from the Github server.
 */
void UpdateChecker::downloadMetadata()
{
    QUrl url(STR_GITHUB_RELEASES_API);

    NetworkManager *networkManager = s_networkManager;
    QNetworkReply *reply = networkManager->get(url);
    if (!reply) {
        emit updateError(tr("Network request rejected."));
        return;
    }

    connect(reply, SIGNAL(finished()),
            this, SLOT(onMetadataFinished()));
}

void UpdateChecker::onMetadataFinished()
{
    auto reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        emit updateError(tr("Network request rejected."));
        return;
    }
    reply->deleteLater();

    ChangeLog changelog;

    QByteArray result = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(result);
    QJsonArray releases = jsonResponse.array();
    foreach (auto release, releases) {

        QJsonObject jsonRelease = release.toObject();

        auto tagName = jsonRelease["tag_name"].toString();

        if (!UpdateCheckerNS::isVersionGreaterThan(tagName, currentVersion())) {
            continue;
        }

        ReleaseInfo version;
        version.tagName = tagName;
        version.title = jsonRelease["name"].toString();
        version.draft = jsonRelease["draft"].toBool();
        version.prerelease = jsonRelease["prerelease"].toBool();
        version.createdAt = jsonRelease["created_at"].toString();
        version.publishedAt = jsonRelease["published_at"].toString();
        version.body = jsonRelease["body"].toString();

        auto assets = jsonRelease["assets"].toArray();
        foreach (auto asset, assets) {
            QJsonObject jsonAsset = asset.toObject();
            auto assetName = jsonAsset["name"].toString();

            QString targetAssetName;
#if defined _WIN32
            if (IS_HOST_64BIT) {
                targetAssetName = QLatin1String("DownZemAll_x64_Setup.exe");
            } else {
                targetAssetName = QLatin1String("DownZemAll_x86_Setup.exe");
            }
#elif defined __APPLE__
            targetAssetName = QLatin1String("<UNDEFINED>");
#else
            targetAssetName = QLatin1String("<UNDEFINED>");
#endif
            Q_ASSERT(!targetAssetName.isEmpty());

            if (assetName.contains(targetAssetName, Qt::CaseInsensitive)) {
                version.assetName = assetName;
                version.assetUrl = jsonAsset["browser_download_url"].toString();
                version.assetCreatedAt = jsonAsset["created_at"].toString();
                version.assetSize = jsonAsset["size"].toInt();
                break;
            }
        }
        changelog.push_back(version);
    }

    if (!changelog.empty()) {
        // for no-gui check
        m_latestUpdateUrl = changelog.front().assetUrl;
        emit updateAvailable();
    }

    storeDateTime();
    emit updateAvailable(changelog);
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Get the binary from the Github server.
 */
void UpdateChecker::downloadAndInstallUpdate()
{
    if (m_temporaryBinaryFile.isOpen()) {
        emit updateError(tr("File '%0' currently opened. Close the file and retry.")
                         .arg(m_temporaryBinaryFile.fileName()));
        return;
    }

    QUrl url(m_latestUpdateUrl);
    m_temporaryBinaryFile.setFileName(tempPath() + '/' + url.fileName());
    if (!m_temporaryBinaryFile.open(QFile::WriteOnly)) {
        emit updateError(tr("Failed to open temporary file '%0'.")
                         .arg(m_temporaryBinaryFile.fileName()));
        return;
    }

    NetworkManager *networkManager = s_networkManager;
    QNetworkReply *reply = networkManager->get(url);
    if (!reply) {
        emit updateError(tr("Network request rejected."));
        return;
    }

    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onBinaryProgress(qint64,qint64)));
    connect(reply, SIGNAL(finished()), this, SLOT(onBinaryFinished()));
}

void UpdateChecker::onBinaryProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(
        static_cast<qsizetype>(bytesReceived),
        static_cast<qsizetype>(bytesTotal));
}

void UpdateChecker::onBinaryFinished()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        emit updateError(tr("Network request rejected."));
        return;
    }
    reply->deleteLater();

    m_temporaryBinaryFile.write(reply->readAll());
    m_temporaryBinaryFile.close();

    emit updateDownloadFinished();

    if (!UpdateInstaller::install(m_temporaryBinaryFile.fileName())) {
        emit updateError("Failed to launch the downloaded update.");
    }
}

/******************************************************************************
 ******************************************************************************/
QString UpdateChecker::tempPath() const
{
    return QDir::tempPath();
}

/******************************************************************************
 ******************************************************************************/
QString UpdateChecker::latestUpdateUrl() const
{
    return m_latestUpdateUrl; // For Linux
}

/******************************************************************************
 ******************************************************************************/
qint64 UpdateChecker::daysSinceLastCheck()
{
    QSettings settings;
    auto lastCheckDate = settings.value("LastUpdateCheckDate", QDate(1,1,1)).toDate();
    return lastCheckDate.daysTo(QDate::currentDate());
}

void UpdateChecker::storeDateTime()
{
    QSettings settings;
    settings.setValue("LastUpdateCheckDate", QDate::currentDate());
}
