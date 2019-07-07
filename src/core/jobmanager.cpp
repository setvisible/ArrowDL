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

#include "jobmanager.h"

#include <Core/Engine>
#include <Core/JobClient>
#include <Core/ResourceItem>
#include <Core/Session>
#include <Core/Settings>


#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QtCore/QSaveFile>


#include <QtNetwork/QtNetwork>

#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

JobManager::JobManager(QObject *parent) : QObject(parent)
  , m_engine(new Engine(this))
  , m_settings(Q_NULLPTR)
  , m_dirtyQueueTimer(Q_NULLPTR)
{
    /* Auto save of the queue */
    connect(this, SIGNAL(jobAppended(JobClient*)), this, SLOT(onQueueChanged(JobClient*)));
    connect(this, SIGNAL(jobRemoved(JobClient*)), this, SLOT(onQueueChanged(JobClient*)));
    connect(this, SIGNAL(jobStateChanged(JobClient*)), this, SLOT(onQueueChanged(JobClient*)));


    connect(m_engine, SIGNAL(jobAppended(JobClient*)), this, SLOT(onEngineJobAppended(JobClient*)));
    connect(m_engine, SIGNAL(jobRemoved(JobClient*)), this, SLOT(onEngineJobRemoved(JobClient*)));
    connect(m_engine, SIGNAL(jobStateChanged(JobClient*)), this, SLOT(onEngineJobStateChanged(JobClient*)));
}

JobManager::~JobManager()
{
    saveQueue();
}

/******************************************************************************
 ******************************************************************************/
/**
 * \fn void JobManager::jobStateChanged(JobClient *job)
 * This signal is emited whenever the job data or its progress or its state has changed
 */

/******************************************************************************
 ******************************************************************************/
Settings *JobManager::settings() const
{
    return m_settings;
}

void JobManager::setSettings(Settings *settings)
{
    if (m_settings) {
        disconnect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
    }
    m_settings = settings;
    if (m_settings) {
        connect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
    }
}

void JobManager::onSettingsChanged()
{
    // reload the queue here
    if (m_queueFile != m_settings->database()) {
        m_queueFile = m_settings->database();
        loadQueue();
    }
}

/******************************************************************************
 ******************************************************************************/
void JobManager::loadQueue()
{
    QList<JobClient *> jobs;
    Session::read(jobs, m_queueFile);
    clear();
    foreach (auto job, jobs) {
        append(job, false);
    }
}

void JobManager::saveQueue()
{
    Session::write(jobs(), m_queueFile);
}

void JobManager::onQueueChanged(JobClient */*job*/)
{
    if (!m_dirtyQueueTimer) {
        m_dirtyQueueTimer = new QTimer(this);
        m_dirtyQueueTimer->setSingleShot(true);
        connect(m_dirtyQueueTimer, SIGNAL(timeout()), SLOT(saveQueue()));
    }
    if (!m_dirtyQueueTimer->isActive()) {
        m_dirtyQueueTimer->start(3000);
    }
}

/******************************************************************************
 ******************************************************************************/
int JobManager::count() const
{
    return jobs().count();
}

/******************************************************************************
 ******************************************************************************/
void JobManager::clear()
{
    clearSelection();
    m_engine->clear();
}

void JobManager::append(JobClient *job, const bool started)
{
    m_engine->append(job,  started);
}

void JobManager::append(ResourceItem *item, const bool started)
{
    JobClient *job = new JobClient(this);
    job->setResource(item);
    append(job, started);
}

void JobManager::append(const QList<ResourceItem *> &items, const bool started)
{
    foreach (auto item, items) {
        append(item, started);
    }
}

void JobManager::remove(JobClient *job)
{
    setSelected(job, false);
    m_engine->remove(job);
}

void JobManager::remove(const QList<JobClient *> &jobs)
{
    foreach (auto job, jobs) {
        remove(job);
    }
}

/******************************************************************************
 ******************************************************************************/
const JobClient* JobManager::clientForRow(int row) const
{
    return m_engine->jobAt(row);
}

/******************************************************************************
 ******************************************************************************/
QList<JobClient*> JobManager::jobs() const
{
    return m_engine->jobs();
}

static inline QList<JobClient*> filter(const QList<JobClient*> &jobs,
                                       const JobClient::State state)
{
    QList<JobClient*> list;
    foreach (auto job, jobs) {
        if (job->state() == state) {
            list.append(job);
        }
    }
    return list;
}

QList<JobClient*> JobManager::waitingJobs() const
{
    return filter(jobs(), JobClient::Idle);
}

QList<JobClient*> JobManager::completedJobs() const
{
    return filter(jobs(), JobClient::Completed);
}

QList<JobClient*> JobManager::pausedJobs() const
{
    return filter(jobs(), JobClient::Paused);
}

QList<JobClient*> JobManager::failedJobs() const
{
    return filter(jobs(), JobClient::Stopped);
}

QList<JobClient*> JobManager::runningJobs() const
{
    QList<JobClient*> list;
    foreach (auto job, jobs()) {
        JobClient::State state = job->state();
        if ( state != JobClient::Idle &&
             state != JobClient::Completed &&
             state != JobClient::Paused &&
             state != JobClient::Stopped ) {
            list.append(job);
        }
    }
    return list;
}

/******************************************************************************
 ******************************************************************************/

QString JobManager::totalSpeed() const
{
    /// \todo // "750.35 MB/s"
    return "";
}

/******************************************************************************
 ******************************************************************************/
void JobManager::resume(JobClient *job)
{
    m_engine->resume(job);
}

void JobManager::pause(JobClient *job)
{
    m_engine->pause(job);
}

void JobManager::cancel(JobClient *job)
{
    m_engine->cancel(job);
}

/******************************************************************************
 ******************************************************************************/
void JobManager::onEngineJobAppended(JobClient *job)
{
    emit jobAppended(job);
}

void JobManager::onEngineJobRemoved(JobClient *job)
{
    emit jobRemoved(job);
}

void JobManager::onEngineJobStateChanged(JobClient *job)
{
    emit jobStateChanged(job);
}

/******************************************************************************
 ******************************************************************************/
void JobManager::clearSelection()
{
    m_selection.clear();
    emit selectionChanged();
}

QList<JobClient*> JobManager::selection() const
{
    return m_selection;
}

void JobManager::setSelection(const QList<JobClient*> &selection)
{
    m_selection.clear();
    m_selection.append(selection);
    emit selectionChanged();
}

bool JobManager::isSelected(JobClient *job) const
{
    return m_selection.contains(job);
}

void JobManager::setSelected(JobClient* job, bool isSelected)
{
    m_selection.removeAll(job);
    if (isSelected) {
        m_selection.append(job);
    }
    emit selectionChanged();
}

QString JobManager::selectionToString() const
{
    QString ret;
    int count = 0;
    foreach (auto job, m_selection) {
        ret += job->localFileName();
        ret += "\n";
        count++;
        if (count > 10) {
            ret += tr("... (%0 others)").arg(m_selection.count()-10);
        }
    }
    return ret;
}
