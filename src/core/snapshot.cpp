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
#include <Core/Scheduler>
#include <Core/Session>
#include <Core/Settings>

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QtMath>

using namespace Qt::Literals::StringLiterals;


/*!
 * \class Snapshot
 * \brief Ensure Crash Recovery by saving the queue in a physical file periodically.
 */
Snapshot::Snapshot(QObject *parent) : QObject(parent)
    , m_delayedIOTimer(new QTimer(this))
    , m_scheduler(static_cast<Scheduler*>(parent))
    , m_settings(nullptr)
{
    // Save the queue periodically
    // Note: the timer optimizes the I/O (file write),
    // indeed it queues the demand of writes.
    // It plans one and only one I/O operation a few seconds later.
    m_delayedIOTimer->setSingleShot(true);
    connect(m_delayedIOTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));

    // Save the queue on before quitting
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(onTimeout()));
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
    if (m_queueFile == m_settings->database()) {
        return;
    }
    m_queueFile = m_settings->database();
    loadQueue();
}

/******************************************************************************
 ******************************************************************************/
void Snapshot::shot()
{
    if (m_delayedIOTimer->isActive()) {
        return;
    }
    m_delayedIOTimer->start(MSEC_AUTO_SAVE);
}

void Snapshot::onTimeout()
{
    saveQueue();
}

/******************************************************************************
 ******************************************************************************/
void Snapshot::loadQueue()
{
    if (m_queueFile.isEmpty()) {
        return;
    }
    QList<AbstractJob*> jobs;
    Session::read(jobs, m_queueFile, m_scheduler);
    m_scheduler->clear();
    m_scheduler->append(jobs, false);
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

    for (auto job : m_scheduler->jobs()) {
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
