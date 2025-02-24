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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../../utils/fakejob.h"
#include "../../utils/fakescheduler.h"

#include <Core/Format>
#include <Core/Theme>
#include <Widgets/QueueView>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtGui/QAction>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_scheduler(new FakeScheduler(this))
{
    ui->setupUi(this);

    Theme::applyTheme({ { Theme::IconTheme, "flat"} });

    /* Connect the GUI to the Scheduler. */
    ui->queueView->setModel(m_scheduler->model());

    /* Connect the SceneManager to the MainWindow. */
    /* The SceneManager centralizes the changes. */
    connect(m_scheduler, SIGNAL(jobFinished(AbstractJob*)), this, SLOT(onJobFinished(AbstractJob*)));
    // connect(m_scheduler, SIGNAL(jobRenamed(QString,QString,bool)), this, SLOT(onJobRenamed(QString,QString,bool)), Qt::QueuedConnection);

    // QObject::connect(m_scheduler, SIGNAL(jobStateChanged(AbstractJob*)), this, SLOT(onJobStateChanged(AbstractJob*)));
    connect(ui->queueView, SIGNAL(dataChanged()), this, SLOT(onDataChanged()));
    connect(ui->queueView, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));

    /* Connect the rest of the GUI widgets together (selection, focus, etc.) */
    createActions();
    createContextMenu();
    propagateIcons();

    refreshTitleAndStatus();
    refreshMenus();

    m_scheduler->createFakeJobs();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::createActions()
{
    //! [0] File
    //! [0]

    //! [1] Edit
    connect(ui->actionSelectAll, SIGNAL(triggered()), ui->queueView, SLOT(selectAll()));
    connect(ui->actionSelectNone, SIGNAL(triggered()), ui->queueView, SLOT(selectNone()));
    connect(ui->actionInvertSelection, SIGNAL(triggered()), ui->queueView, SLOT(invertSelection()));
    connect(ui->actionSelectCompleted, SIGNAL(triggered()), ui->queueView, SLOT(selectCompleted()));
    //! [1]

    //! [2] View
    connect(ui->actionRemoveCompleted, SIGNAL(triggered()), this, SLOT(removeCompleted()));
    connect(ui->actionRemoveSelected, SIGNAL(triggered()), this, SLOT(removeSelected()));
    connect(ui->actionRemoveAll, SIGNAL(triggered()), this, SLOT(removeAll()));
    //! [2]

    //! [3] Download
    connect(ui->actionAdd, SIGNAL(triggered()), this, SLOT(add()));
    //--
    connect(ui->actionResume, SIGNAL(triggered()), this, SLOT(resume()));
    connect(ui->actionPause, SIGNAL(triggered()), this, SLOT(pause()));
    connect(ui->actionCancel, SIGNAL(triggered()), this, SLOT(cancel()));
    //--
    connect(ui->actionUp, SIGNAL(triggered()), ui->queueView, SLOT(up()));
    connect(ui->actionTop, SIGNAL(triggered()), ui->queueView, SLOT(top()));
    connect(ui->actionDown, SIGNAL(triggered()), ui->queueView, SLOT(down()));
    connect(ui->actionBottom, SIGNAL(triggered()), ui->queueView, SLOT(bottom()));
    //! [3]

    //! [4]  Options
    //! [4]
}

void MainWindow::createContextMenu()
{
    QMenu *contextMenu = new QMenu(this);

    contextMenu->addAction(ui->actionResume);
    contextMenu->addAction(ui->actionPause);
    contextMenu->addAction(ui->actionCancel);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionRemoveCompleted);
    contextMenu->addAction(ui->actionRemoveSelected);
    contextMenu->addAction(ui->actionRemoveAll);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionSelectAll);
    contextMenu->addAction(ui->actionInvertSelection);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionTop);
    contextMenu->addAction(ui->actionUp);
    contextMenu->addAction(ui->actionDown);
    contextMenu->addAction(ui->actionBottom);

    ui->queueView->setContextMenu(contextMenu);
}

void MainWindow::propagateIcons()
{
    const QHash<QAction*, QString> hash = {

        //! [0] File
        {ui->actionAdd                    , "add-batch"},
        //! [0]

        //! [1] Edit
        {ui->actionSelectAll              , "select-all"},
        {ui->actionSelectNone             , "select-none"},
        {ui->actionInvertSelection        , "select-invert"},
        {ui->actionSelectCompleted        , "select-completed"},
        //! [1]

        //! [2] View
        {ui->actionRemoveCompleted        , "remove-completed"},
        {ui->actionRemoveSelected         , "remove-downloaded"},
        {ui->actionRemoveAll              , "remove-all"},
        //! [2]

        //! [3] Download
        {ui->actionResume                 , "play-resume"},
        {ui->actionPause                  , "play-pause"},
        {ui->actionCancel                 , "play-stop"},
        //--
        {ui->actionUp                     , "move-up"},
        {ui->actionTop                    , "move-top"},
        {ui->actionDown                   , "move-down"},
        {ui->actionBottom                 , "move-bottom"},
        //! [3]

        //! [4]  Options
        //! [4]

        //! [5] Help
        //! [5]
    };
    Theme::setIcons(this, hash);
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::removeCompleted()
{
    ui->queueView->removeCompleted();
}

void MainWindow::removeSelected()
{
    ui->queueView->removeSelected();
}

void MainWindow::removeAll()
{    
    ui->queueView->removeAll();
}

void MainWindow::add()
{
    m_scheduler->createFakeJobs(100);
}

void MainWindow::resume()
{
    for (auto job : ui->queueView->selectedJobs()) {
        m_scheduler->resume(job);
    }
}

void MainWindow::cancel()
{
    for (auto job : ui->queueView->selectedJobs()) {
        m_scheduler->cancel(job);
    }
}

void MainWindow::pause()
{
    for (auto job : ui->queueView->selectedJobs()) {
        m_scheduler->pause(job);
    }
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onJobFinished(AbstractJob *job)
{
    qDebug() << "Finished" << job;
    onDataChanged();
}

void MainWindow::onDataChanged()
{
    refreshMenus();
    refreshTitleAndStatus();
}

void MainWindow::onSelectionChanged()
{
    refreshMenus();
}

void MainWindow::refreshTitleAndStatus()
{
    auto speed = m_scheduler->totalSpeed();
    auto totalSpeed = speed > 0
            ? QString("~%0").arg(Format::currentSpeedToString(speed))
            : QString();

    auto completedCount = m_scheduler->completedJobs().count();
    auto failedCount = m_scheduler->failedJobs().count();
    auto count = m_scheduler->count();
    auto doneCount = completedCount + failedCount;

    auto windowTitle = QString("%0 %1/%2 - %3")
            .arg(totalSpeed).arg(doneCount).arg(count)
            .arg(QLatin1String("Demo - Download Queue View")).trimmed();

    this->setWindowTitle(windowTitle);
}

void MainWindow::refreshMenus()
{
    auto jobs = m_scheduler->jobs();
    auto selectedJobs = ui->queueView->selectedJobs();

    const bool hasJobs = !jobs.isEmpty();
    const bool hasSelection = !selectedJobs.isEmpty();
    const bool hasOnlyOneSelected = selectedJobs.count() == 1;

    bool hasOnlyCompletedSelected = hasSelection;
    for (auto job : selectedJobs) {
        if (job->state() != AbstractJob::Completed) {
            hasOnlyCompletedSelected = false;
            break;
        }
    }

    bool hasResumableSelection = false;
    bool hasPausableSelection = false;
    bool hasCancelableSelection = false;
    for (auto job : selectedJobs) {
        if (job->isResumable()) {
            hasResumableSelection = true;
        }
        if (job->isPausable()) {
            hasPausableSelection = true;
        }
        if (job->isCancelable()) {
            hasCancelableSelection = true;
        }
    }

    //! [0] File
    //! [0]

    //! [1] Edit
    ui->actionSelectNone->setEnabled(hasSelection);
    //! [1]

    //! [2] View
    ui->actionRemoveCompleted->setEnabled(hasJobs);
    ui->actionRemoveSelected->setEnabled(hasJobs);
    // ui->actionRemoveAll->setEnabled(hasJobs); // always enabled
    //! [2]

    //! [3] Download
    ui->actionResume->setEnabled(hasResumableSelection);
    ui->actionPause->setEnabled(hasPausableSelection);
    ui->actionCancel->setEnabled(hasCancelableSelection);
    //--
    ui->actionUp->setEnabled(hasSelection);
    ui->actionTop->setEnabled(hasSelection);
    ui->actionDown->setEnabled(hasSelection);
    ui->actionBottom->setEnabled(hasSelection);
    //! [3]

    //! [4]  Options
    //! [4]

    //! [5] Help
    //! [5]
}
