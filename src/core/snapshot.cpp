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

#include "snapshot.h"

#include <Constants>
#include <Core/AbstractJob>
#include <Core/DownloadManager>
#include <Core/Session>
#include <Core/Settings>

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QtMath>

using namespace Qt::Literals::StringLiterals;


/*!
 * \class Snapshot
 * \brief Ensure Crash Recovery by saving the queue in a physical file periodically.
 *
 * Rem: onSettingsChanged() loads the queue.
 * Once loaded, it is saved every X seconds.
 */
Snapshot::Snapshot(QObject *parent) : QObject(parent)
{
    m_downloadManager = static_cast<DownloadManager*>(parent);
}

Snapshot::~Snapshot()
{
    saveQueue();
}

/******************************************************************************
 ******************************************************************************/
Settings *Snapshot::settings() const
{
    return m_settings;
}

void Snapshot::setSettings(Settings *settings)
{
    if (m_settings) {
        disconnect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
    }
    m_settings = settings;
    if (m_settings) {
        connect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
    }
}

void Snapshot::onSettingsChanged()
{
    // reload the queue here
    if (m_queueFile != m_settings->database()) {
        m_queueFile = m_settings->database();
        loadQueue();
    }
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Save the queue periodically, in case of crash.
 */
void Snapshot::shot()
{
    if (!m_dirtyQueueTimer) {
        m_dirtyQueueTimer = new QTimer(this);
        m_dirtyQueueTimer->setSingleShot(true);
        connect(m_dirtyQueueTimer, SIGNAL(timeout()), SLOT(saveQueue()));
    }
    if (!m_dirtyQueueTimer->isActive()) {
        m_dirtyQueueTimer->start(MSEC_AUTO_SAVE);
    }
}

/******************************************************************************
 ******************************************************************************/
void Snapshot::loadQueue()
{
    if (m_queueFile.isEmpty()) {
        return;
    }
    QList<AbstractJob*> jobs;
    Session::read(jobs, m_queueFile, m_downloadManager);
    m_downloadManager->clear();
    m_downloadManager->append(jobs, false);
}

void Snapshot::saveQueue()
{
    if (m_queueFile.isEmpty()) {
        return;
    }
    QList<AbstractJob *> jobs;

    auto skipCompleted = m_settings->isRemoveCompletedEnabled();
    auto skipCanceled = m_settings->isRemoveCanceledEnabled();
    auto skipPaused = m_settings->isRemovePausedEnabled();

    for (auto job : m_downloadManager->jobs()) {
        if (job) {
            switch (job->state()) {
            case AbstractJob::Idle:
            case AbstractJob::Paused:
            case AbstractJob::Preparing:
            case AbstractJob::Connecting:
            case AbstractJob::DownloadingMetadata:
            case AbstractJob::Downloading:
            case AbstractJob::Endgame:
                if (skipPaused) continue;
                break;

            case AbstractJob::Completed:
            case AbstractJob::Seeding:
                if (skipCompleted) continue;
                break;

            case AbstractJob::Stopped:
            case AbstractJob::Skipped:
            case AbstractJob::NetworkError:
            case AbstractJob::FileError:
                if (skipCanceled) continue;
                break;
            }
            jobs.append(job);
        }
    }
    Session::write(jobs, m_queueFile);
}
