/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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

#include <QtCore/QDebug>
#include <QtCore/QDate>
#include <QtCore/QSettings>


UpdateChecker::UpdateChecker(QObject *parent) : QObject(parent)
  , m_updater(STR_GITHUB_REPO_ADDRESS,
              STR_APPLICATION_VERSION,
              UpdateCheckerNS::addressMatcher(IS_HOST_64BIT))
{
    m_updater.setUpdateStatusListener(this);
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
    m_updater.checkForUpdates();
}

/******************************************************************************
 ******************************************************************************/
QString UpdateChecker::installTempDir() const
{
    return m_updater.installTempDir();
}

void UpdateChecker::downloadAndInstallUpdate()
{
    m_updater.downloadAndInstallUpdate(m_latestUpdateUrl);
}

/******************************************************************************
 ******************************************************************************/
QString UpdateChecker::latestUpdateUrl() const
{
    return m_latestUpdateUrl;
}

/******************************************************************************
 ******************************************************************************/
qint64 UpdateChecker::daysSinceLastCheck()
{
    QSettings settings;
    auto lastCheckDate = settings.value("LastUpdateCheckDate", QDate(1,1,1)).toDate();
    return lastCheckDate.daysTo(QDate::currentDate());
}

void UpdateChecker::storeDate()
{
    QSettings settings;
    settings.setValue("LastUpdateCheckDate", QDate::currentDate());
}

/******************************************************************************
 ******************************************************************************/
void UpdateChecker::onUpdateAvailable(CAutoUpdaterGithub::ChangeLog changelog)
{
    if (!changelog.empty()) {
        m_latestUpdateUrl = changelog.front().versionUpdateUrl;
        emit updateAvailable();
    }
    storeDate();
    emit updateAvailable(changelog);
}

void UpdateChecker::onUpdateDownloadProgress(float percentageDownloaded)
{
    emit updateDownloadProgress(percentageDownloaded);
}

void UpdateChecker::onUpdateDownloadFinished()
{
    emit updateDownloadFinished();
}

void UpdateChecker::onUpdateError(QString errorMessage)
{
    emit updateError(errorMessage);
}
