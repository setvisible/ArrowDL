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

#ifndef CORE_JOB_MANAGER_H
#define CORE_JOB_MANAGER_H

#include <QtCore/QObject>
#include <QtCore/QList>

class Engine;
class JobClient;
class ResourceItem;
class Settings;

class QTimer;

class JobManager : public QObject
{
    Q_OBJECT

public:
    JobManager(QObject *parent = 0);
    ~JobManager();

    /* Settings */
    Settings* settings() const;
    void setSettings(Settings *settings);

    /* Queue Management */
    int count() const;

    void clear();

    void append(JobClient *job, const bool started = false);
    void append(ResourceItem *item, const bool started = false);
    void append(const QList<ResourceItem *> &items, const bool started = false);

    void remove(JobClient *job);
    void remove(const QList<JobClient*> &jobs);

    const JobClient* clientForRow(int row) const;

    /* Statistics */
    QList<JobClient*> jobs() const;
    QList<JobClient*> waitingJobs() const;
    QList<JobClient*> completedJobs() const;
    QList<JobClient*> pausedJobs() const;
    QList<JobClient*> failedJobs() const;
    QList<JobClient*> runningJobs() const;

    QString totalSpeed() const;

    /* Actions */
    void resume(JobClient *job);
    void pause(JobClient *job);
    void cancel(JobClient *job);

    /* Selection */
    void clearSelection();
    QList<JobClient*> selection() const;
    void setSelection(const QList<JobClient *> &selection);

    bool isSelected(JobClient *job) const;
    void setSelected(JobClient *job, bool isSelected);

    QString selectionToString() const;

signals:
    void jobAppended(JobClient *job);
    void jobRemoved(JobClient *job);
    void jobStateChanged(JobClient *job);

    void selectionChanged();

public slots:

private slots:

    void onSettingsChanged();    
    void onQueueChanged(JobClient *job);
    void loadQueue();
    void saveQueue();

    void onEngineJobAppended(JobClient *job);
    void onEngineJobRemoved(JobClient *job);
    void onEngineJobStateChanged(JobClient *job);

private:
    Engine *m_engine;
    QList<JobClient*> m_selection; // QSet instead
    Settings *m_settings;

    // Crash Recovery
    QTimer* m_dirtyQueueTimer;
    QString m_queueFile;
};

#endif // CORE_JOB_MANAGER_H
