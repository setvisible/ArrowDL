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

#include "scheduler.h"

#include <Constants>
#include <Core/AbstractJob>
#include <Core/JobFile>
#include <Core/JobTorrent>
#include <Core/NetworkManager>
#include <Core/QueueModel>
#include <Core/ResourceItem>
#include <Core/Settings>
#include <Core/Snapshot>

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QtMath>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

using namespace Qt::Literals::StringLiterals;


/*!
 * \class Scheduler
 *
 * The Scheduler class manages:
 * \li settings persistence
 * \li queue persistence
 * \li network requests (GET, POST, PUT, HEAD...)
 */
Scheduler::Scheduler(QObject *parent) : QObject(parent)
    , m_queueModel(new QueueModel(this))
    , m_networkManager(new NetworkManager(this))
    , m_snapshot(new Snapshot(this))
    , m_speedTimer(new QTimer(this))
{
    connect(this, SIGNAL(jobFinished(AbstractJob*)),
            this, SLOT(startNext(AbstractJob*)));

    connect(m_speedTimer, SIGNAL(timeout()), this, SLOT(onSpeedTimerTimeout()));
}

Scheduler::~Scheduler()
{
    clear();
}

/******************************************************************************
 ******************************************************************************/
QAbstractItemModel *Scheduler::model() const
{
    return m_queueModel;
}

/******************************************************************************
 ******************************************************************************/
Settings *Scheduler::settings() const
{
    return m_settings;
}

void Scheduler::setSettings(Settings *settings)
{
    if (m_settings) {
        disconnect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
    }
    m_settings = settings;
    if (m_settings) {
        connect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
    }
    m_networkManager->setSettings(m_settings);
    m_snapshot->setSettings(m_settings);
}

void Scheduler::onSettingsChanged()
{
    setMaxSimultaneousDownloads(m_settings->maxSimultaneousDownloads());
}

void Scheduler::activateSnapshot()
{
    m_snapshot->shot();
}

/******************************************************************************
 ******************************************************************************/
NetworkManager* Scheduler::networkManager() const
{
    return m_networkManager;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Reimplement this method allows it to make jobs like a factory.
 * That makes the unit tests of this class easier, allowing dummy items.
 * \remark Optional
 */
AbstractJob* Scheduler::createJobFile(const QUrl &url)
{
    ResourceItem* resource = createResourceItem(url);
    auto job = new JobFile(this, resource);
    return job;
}

AbstractJob* Scheduler::createJobTorrent(const QUrl &url)
{
    ResourceItem* resource = createResourceItem(url);
    resource->setType(ResourceItem::Type::Torrent);
    auto job = new JobTorrent(this, resource);
    return job;
}

/******************************************************************************
 ******************************************************************************/
inline ResourceItem* Scheduler::createResourceItem(const QUrl &url)
{
    QSettings settings;
    settings.beginGroup("Wizard");
    auto path = settings.value("Path"_L1, {}).toString();
    auto mask = settings.value("Mask"_L1, {}).toString();
    settings.endGroup();

    auto resource = new ResourceItem();
    resource->setUrl(url.toString().toUtf8());
    resource->setCustomFileName({});
    resource->setReferringPage({});
    resource->setDescription({});
    resource->setDestination(path);
    resource->setMask(mask);
    resource->setCheckSum({});
    return resource;
}

/******************************************************************************
 ******************************************************************************/
qsizetype Scheduler::downloadingCount() const
{
    auto count = 0;
    for (auto job : m_queueModel->jobs()) {
       if (job->isDownloading()) {
           count++;
       }
    }
    return count;
}

void Scheduler::startNext(AbstractJob */*job*/)
{
    if (downloadingCount() < m_maxSimultaneousDownloads) {
       auto rows = m_queueModel->rowCount();
       for (int i = 0; i < rows; ++i) {
           auto index = m_queueModel->index(i, 0);
           AbstractJob* job = model()->data(index, QueueModel::JobRole).value<AbstractJob*>();
           if (job->state() == AbstractJob::Idle) {
               job->resume();
               startNext(nullptr);
               break;
           }
       }
    }
}

/******************************************************************************
 ******************************************************************************/
qsizetype Scheduler::count() const
{
    return m_queueModel->rowCount();
}

/******************************************************************************
 ******************************************************************************/
void Scheduler::clear()
{
    m_queueModel->removeRows(0, m_queueModel->rowCount());
}

/******************************************************************************
 ******************************************************************************/
void Scheduler::append(const QList<AbstractJob *> &jobs, bool started)
{
    if (jobs.isEmpty()) {
        return;
    }
    for (auto job : jobs) {
        if (!job) {
            return;
        }

        connect(job, SIGNAL(changed()), this, SLOT(onJobChanged()));
        connect(job, SIGNAL(finished()), this, SLOT(onJobFinished()));
        connect(job, SIGNAL(renamed(QString,QString,bool)), this, SLOT(onJobRenamed(QString,QString,bool)));

        if (started) {
            if (job->isResumable()) {
                job->setState(AbstractJob::Idle);
            }
        } else {
            if (job->isPausable()) {
                job->setState(AbstractJob::Paused);
            }
        }
    }

    /// \todo Replace append(...) with insertRow()
    m_queueModel->append(jobs);
    activateSnapshot();

    if (started) {
        startNext(nullptr);
    }
}

/******************************************************************************
 ******************************************************************************/
int Scheduler::maxSimultaneousDownloads() const
{
    return m_maxSimultaneousDownloads;
}

void Scheduler::setMaxSimultaneousDownloads(int number)
{
    m_maxSimultaneousDownloads = number;
}

/******************************************************************************
 ******************************************************************************/
QList<AbstractJob *> Scheduler::jobs() const
{
    return  m_queueModel->jobs();
}

static inline QList<AbstractJob*> filter(
    const QList<AbstractJob*> &jobs,
    const QList<AbstractJob::State> &states)
{
    QList<AbstractJob*> list;
    for (auto job : jobs) {
        for (auto state : states) {
            if (job->state() == state) {
                list.append(job);
            }
        }
    }
    return list;
}

QList<AbstractJob*> Scheduler::completedJobs() const
{
    return filter(
        jobs(),
        {AbstractJob::Completed,
         AbstractJob::Seeding});
}

QList<AbstractJob *> Scheduler::failedJobs() const
{
    return filter(
        jobs(),
        {AbstractJob::Stopped,
         AbstractJob::Skipped,
         AbstractJob::NetworkError,
         AbstractJob::FileError});
}

QList<AbstractJob*> Scheduler::runningJobs() const
{
    return filter(
        jobs(),
        {AbstractJob::Preparing,
         AbstractJob::Connecting,
         AbstractJob::DownloadingMetadata,
         AbstractJob::Downloading,
         AbstractJob::Endgame});
}

/******************************************************************************
 ******************************************************************************/
void Scheduler::onSpeedTimerTimeout()
{
    m_speedTimer->stop();
    m_previouSpeed = 0;
    emit onJobChanged();
}

qreal Scheduler::totalSpeed()
{
    qreal speed = 0;
    for (auto job : jobs()) {
        speed += qMax(job->speed(), qreal(0));
    }
    if (speed > 0) {
        m_previouSpeed = speed;
        m_speedTimer->start(MSEC_SPEED_DISPLAY_TIME);
    }
    return m_previouSpeed;
}

/******************************************************************************
 ******************************************************************************/
void Scheduler::resume(AbstractJob *job)
{
    if (job->isResumable()) {
        job->setReadyToResume();
        startNext(job);
    }
}

void Scheduler::pause(AbstractJob *job)
{
    if (job->isPausable()) {
        job->pause();
    }
}

void Scheduler::cancel(AbstractJob *job)
{
    if (job->isCancelable()) {
        job->stop();
    }
}

/******************************************************************************
 ******************************************************************************/
void Scheduler::onJobChanged()
{
    activateSnapshot();
}

void Scheduler::onJobFinished()
{
    auto job = qobject_cast<AbstractJob *>(sender());
    emit jobFinished(job);
}

void Scheduler::onJobRenamed(const QString &oldName, const QString &newName, bool success)
{
    emit jobRenamed(oldName, newName, success);
}
