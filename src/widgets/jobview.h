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

#ifndef WIDGETS_JOB_VIEW_H
#define WIDGETS_JOB_VIEW_H

#include <QtCore/QMap>
#include <QtWidgets/QWidget>
#include <QtCore/QModelIndex>

class JobClient;
class JobManager;

class QueueView;
class QTreeWidgetItem;
class QMenu;

class JobView : public QWidget
{
    Q_OBJECT
public:
    explicit JobView(QWidget *parent = 0);
    ~JobView();

    const JobManager* jobManager() const;
    void setManager(JobManager *jobManager);
    void setContextMenu(QMenu *contextMenu);

    QSize sizeHint() const override;

signals:
    void doubleClicked(JobClient *job);
    void selectionChanged();

private slots:
    void onJobAdded(JobClient *job);
    void onJobRemoved(JobClient *job);    
    void onJobStateChanged(JobClient *job);
    void onSelectionChanged();

    void onQueueViewDoubleClicked(const QModelIndex &index);
    void onQueueViewItemSelectionChanged();

    void showContextMenu(const QPoint &pos) ;

private:
    JobManager *m_jobManager;
    QueueView *m_queueView;
    QMenu *m_contextMenu;

    void addItem(QTreeWidgetItem* item, JobClient *job);
    void removeItem(QTreeWidgetItem* item);
    QTreeWidgetItem* getItem(JobClient *job);
    JobClient* getJob(QTreeWidgetItem* item);
    QMap<QTreeWidgetItem*, JobClient* > m_map;

    void updateItem(QTreeWidgetItem* item, JobClient *job);
};

#endif // WIDGETS_JOB_VIEW_H
