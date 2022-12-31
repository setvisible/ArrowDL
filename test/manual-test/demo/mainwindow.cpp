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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../../utils/fakedownloaditem.h"
#include "../../utils/fakedownloadmanager.h"

#include <Core/Format>
#include <Core/Theme>
#include <Widgets/DownloadQueueView>

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
  , m_downloadManager(new FakeDownloadManager(this))

{
    ui->setupUi(this);

    Theme::applyTheme({ { Theme::IconTheme, "flat"} });

    /* Connect the GUI to the DownloadManager. */
    ui->downloadQueueView->setEngine(m_downloadManager);

    /* Connect the SceneManager to the MainWindow. */
    /* The SceneManager centralizes the changes. */
    QObject::connect(m_downloadManager, SIGNAL(jobAppended(DownloadRange)),
                     this, SLOT(onJobAddedOrRemoved(DownloadRange)));
    QObject::connect(m_downloadManager, SIGNAL(jobRemoved(DownloadRange)),
                     this, SLOT(onJobAddedOrRemoved(DownloadRange)));
    QObject::connect(m_downloadManager, SIGNAL(jobStateChanged(IDownloadItem*)),
                     this, SLOT(onJobStateChanged(IDownloadItem*)));
    QObject::connect(m_downloadManager, SIGNAL(selectionChanged()),
                     this, SLOT(onSelectionChanged()));


    /* Connect the rest of the GUI widgets together (selection, focus, etc.) */
    createActions();
    createContextMenu();
    propagateIcons();

    refreshTitleAndStatus();
    refreshMenus();

    m_downloadManager->createFakeJobs();
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
    connect(ui->actionSelectAll, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(ui->actionSelectNone, SIGNAL(triggered()), this, SLOT(selectNone()));
    connect(ui->actionInvertSelection, SIGNAL(triggered()), this, SLOT(invertSelection()));
    connect(ui->actionSelectCompleted, SIGNAL(triggered()), this, SLOT(selectCompleted()));
    // --
    connect(ui->actionManageMirrors, SIGNAL(triggered()), this, SLOT(manageMirrors()));
    connect(ui->actionOneMoreSegment, SIGNAL(triggered()), this, SLOT(oneMoreSegment()));
    connect(ui->actionOneFewerSegment, SIGNAL(triggered()), this, SLOT(oneFewerSegment()));
    //! [1]

    //! [2] View
    connect(ui->actionRemoveCompleted, SIGNAL(triggered()), this, SLOT(removeCompleted()));
    connect(ui->actionRemoveAll, SIGNAL(triggered()), this, SLOT(removeAll()));
    connect(ui->actionRemoveWaiting, SIGNAL(triggered()), this, SLOT(removeWaiting()));
    // --
    connect(ui->actionRemoveSelected, SIGNAL(triggered()), this, SLOT(removeSelected()));
    connect(ui->actionRemoveDuplicates, SIGNAL(triggered()), this, SLOT(removeDuplicates()));
    connect(ui->actionRemoveFailed, SIGNAL(triggered()), this, SLOT(removeFailed()));
    connect(ui->actionRemovePaused, SIGNAL(triggered()), this, SLOT(removePaused()));
    //! [2]

    //! [3] Download
    connect(ui->actionAdd, SIGNAL(triggered()), this, SLOT(add()));
    //--
    connect(ui->actionResume, SIGNAL(triggered()), this, SLOT(resume()));
    connect(ui->actionPause, SIGNAL(triggered()), this, SLOT(pause()));
    connect(ui->actionCancel, SIGNAL(triggered()), this, SLOT(cancel()));
    //--
    connect(ui->actionUp, SIGNAL(triggered()), this, SLOT(up()));
    connect(ui->actionTop, SIGNAL(triggered()), this, SLOT(top()));
    connect(ui->actionDown, SIGNAL(triggered()), this, SLOT(down()));
    connect(ui->actionBottom, SIGNAL(triggered()), this, SLOT(bottom()));
    //! [3]

    //! [4]  Options
    connect(ui->actionSpeedLimit, SIGNAL(triggered()), this, SLOT(speedLimit()));
    connect(ui->actionAddDomainSpecificLimit, SIGNAL(triggered()), this, SLOT(addDomainSpecificLimit()));
    //--
    connect(ui->actionForceStart, SIGNAL(triggered()), this, SLOT(forceStart()));
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

    QMenu *remove = contextMenu->addMenu(tr("Other"));
    remove->addAction(ui->actionRemoveWaiting);
    remove->addAction(ui->actionRemoveDuplicates);
    remove->addSeparator();
    remove->addAction(ui->actionRemovePaused);
    remove->addAction(ui->actionRemoveFailed);

    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionSelectAll);
    contextMenu->addAction(ui->actionInvertSelection);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionTop);
    contextMenu->addAction(ui->actionUp);
    contextMenu->addAction(ui->actionDown);
    contextMenu->addAction(ui->actionBottom);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionSpeedLimit);

    QMenu *advanced = contextMenu->addMenu(tr("Advanced"));
    advanced->addAction(ui->actionOneMoreSegment);
    advanced->addAction(ui->actionOneFewerSegment);
    advanced->addSeparator();
    advanced->addAction(ui->actionManageMirrors);
    advanced->addSeparator();
    advanced->addAction(ui->actionForceStart);
    advanced->addSeparator();
    advanced->addAction(ui->actionAddDomainSpecificLimit);

    ui->downloadQueueView->setContextMenu(contextMenu);
}

void MainWindow::propagateIcons()
{
    const QMap<QAction*, QString> map = {

        //! [0] File
        //        {ui->actionHome                   , "home"},
        //        //--
        {ui->actionAdd              , "add-batch"},
        //        {ui->actionAddContent             , "add-content"},
        //        {ui->actionAddBatch               , "add-batch"},
        //        {ui->actionAddStream              , "add-stream"},
        //        {ui->actionAddTorrent             , "add-torrent"},
        //        {ui->actionAddUrls                , "add-urls"},
        //        // --
        //        {ui->actionImportFromFile         , "file-import"},
        //        {ui->actionExportSelectedToFile   , "file-export"},
        // --
        // {ui->actionQuit   , ""},
        //! [0]

        //! [1] Edit
        {ui->actionSelectAll              , "select-all"},
        {ui->actionSelectNone             , "select-none"},
        {ui->actionInvertSelection        , "select-invert"},
        {ui->actionSelectCompleted        , "select-completed"},
        // --
        // {ui->actionCopy   , ""},
        // --
        {ui->actionManageMirrors          , "mirror-server"},
        {ui->actionOneMoreSegment         , "segment-add"},
        {ui->actionOneFewerSegment        , "segment-remove"},
        //! [1]

        //! [2] View
        //        {ui->actionInformation            , "info"},
        //        // --
        //        {ui->actionOpenFile               , "file-open"},
        //        {ui->actionRenameFile             , "rename"},
        //        {ui->actionDeleteFile             , "file-delete"},
        //        {ui->actionOpenDirectory          , "directory-open"},
        //        // --
        {ui->actionRemoveCompleted        , "remove-completed"},
        {ui->actionRemoveSelected         , "remove-downloaded"},
        {ui->actionRemoveAll              , "remove-all"},
        // --
        {ui->actionRemoveWaiting          , "remove-waiting"},
        {ui->actionRemoveDuplicates       , "remove-duplicates"},
        //        {ui->actionRemoveRunning          , "remove-resumed"},
        {ui->actionRemovePaused           , "remove-paused"},
        {ui->actionRemoveFailed           , "remove-stopped"},
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
        {ui->actionSpeedLimit             , "limit-speed"},
        {ui->actionAddDomainSpecificLimit , "limit-domain"},
        //--
        {ui->actionForceStart             , "play-resume-force"},
        //--
        //        {ui->actionPreferences            , "preference"},
        //! [4]

        //! [5] Help
        // {ui->actionCheckForUpdates        , ""},
        // {ui->actionTutorial               , ""},
        //        {ui->actionAbout                  , "about"}
        // {ui->actionAboutQt                , ""},
        // {ui->actionAboutCompiler          , ""},
        // {ui->actionAboutYoutubeDL         , ""}
        //! [5]
    };
    Theme::setIcons(this, map);
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::selectAll()
{
    m_downloadManager->setSelection(m_downloadManager->downloadItems());
}

void MainWindow::selectNone()
{
    m_downloadManager->clearSelection();
}

void MainWindow::invertSelection()
{
    QList<IDownloadItem*> inverted;
    foreach (auto item, m_downloadManager->downloadItems()) {
        if (!m_downloadManager->isSelected(item)) {
            inverted.append(item);
        }
    }
    m_downloadManager->setSelection(inverted);
}

void MainWindow::selectCompleted()
{
    m_downloadManager->setSelection(m_downloadManager->completedJobs());
}

void MainWindow::manageMirrors()
{    
    qDebug() << Q_FUNC_INFO << "TODO";
}

void MainWindow::oneMoreSegment()
{
    m_downloadManager->oneMoreSegment();
}

void MainWindow::oneFewerSegment()
{
    m_downloadManager->oneFewerSegment();
}

void MainWindow::removeWaiting()
{
    m_downloadManager->remove(m_downloadManager->waitingJobs());
}

void MainWindow::removeAll()
{
    m_downloadManager->remove(m_downloadManager->downloadItems());
}

void MainWindow::removeCompleted()
{
    m_downloadManager->remove(m_downloadManager->completedJobs());
}

void MainWindow::removeSelected()
{
    m_downloadManager->remove(m_downloadManager->selection());
}

void MainWindow::removeDuplicates()
{
    qDebug() << Q_FUNC_INFO << "TODO remove Duplicates";
}

void MainWindow::removeFailed()
{
    m_downloadManager->remove(m_downloadManager->failedJobs());
}

void MainWindow::removePaused()
{
    m_downloadManager->remove(m_downloadManager->pausedJobs());
}

void MainWindow::add()
{
    m_downloadManager->createFakeJobs(100);
}

void MainWindow::resume()
{
    foreach (auto item, m_downloadManager->selection()) {
        m_downloadManager->resume(item);
    }
}

void MainWindow::cancel()
{
    foreach (auto item, m_downloadManager->selection()) {
        m_downloadManager->cancel(item);
    }
}

void MainWindow::pause()
{
    foreach (auto item, m_downloadManager->selection()) {
        m_downloadManager->pause(item);
    }
}

void MainWindow::up()
{
    m_downloadManager->moveCurrentUp();
}

void MainWindow::top()
{
    m_downloadManager->moveCurrentTop();
}

void MainWindow::down()
{
    m_downloadManager->moveCurrentDown();
}

void MainWindow::bottom()
{
    m_downloadManager->moveCurrentBottom();
}

void MainWindow::speedLimit()
{    
    qDebug() << Q_FUNC_INFO << "TODO";
}

void MainWindow::addDomainSpecificLimit()
{
    qDebug() << Q_FUNC_INFO << "TODO";
}

void MainWindow::forceStart()
{
    foreach (auto item, m_downloadManager->selection()) {
        /// todo Maybe run the item instantly (in a higher priority queue?)
        m_downloadManager->cancel(item);
        m_downloadManager->resume(item);
    }
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onJobAddedOrRemoved(DownloadRange /*range*/)
{
    refreshTitleAndStatus();
}

void MainWindow::onJobStateChanged(IDownloadItem * /*downloadItem*/)
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
    auto speed = m_downloadManager->totalSpeed();
    auto totalSpeed = speed > 0
            ? QString("~%0").arg(Format::currentSpeedToString(speed))
            : QString();

    const int completedCount = m_downloadManager->completedJobs().count();
    // const int runningCount = m_downloadManager->runningJobs().count();
    const int failedCount = m_downloadManager->failedJobs().count();
    const int count = m_downloadManager->count();
    const int doneCount = completedCount + failedCount;

    auto windowTitle = QString("%0 %1/%2 - %3")
            .arg(totalSpeed).arg(doneCount).arg(count)
            .arg(QLatin1String("Demo - Download Queue View")).trimmed();

    this->setWindowTitle(windowTitle);
}

void MainWindow::refreshMenus()
{
    const bool hasJobs = !m_downloadManager->downloadItems().isEmpty();
    const bool hasSelection = !m_downloadManager->selection().isEmpty();
    // const bool hasOnlyOneSelected = m_downloadManager->selection().count() == 1;
    bool hasOnlyCompletedSelected = hasSelection;
    foreach (auto item, m_downloadManager->selection()) {
        if (item->state() != IDownloadItem::Completed) {
            hasOnlyCompletedSelected = false;
            continue;
        }
    }
    bool hasAtLeastOneUncompletedSelected = false;
    foreach (auto item, m_downloadManager->selection()) {
        if (item->state() != IDownloadItem::Completed) {
            hasAtLeastOneUncompletedSelected = true;
            continue;
        }
    }
    bool hasResumableSelection = false;
    bool hasPausableSelection = false;
    bool hasCancelableSelection = false;
    foreach (auto item, m_downloadManager->selection()) {
        if (item->isResumable()) {
            hasResumableSelection = true;
        }
        if (item->isPausable()) {
            hasPausableSelection = true;
        }
        if (item->isCancelable()) {
            hasCancelableSelection = true;
        }
    }

    //! [0] File
    //ui->actionImportWizard->setEnabled(hasSelection);
    // --
    //ui->actionImportFromFile->setEnabled(hasSelection);
    //ui->actionExportSelectedToFile->setEnabled(hasSelection);
    // --
    //ui->actionExit->setEnabled(hasSelection);
    //! [0]

    //! [1] Edit
    //ui->actionSelectAll->setEnabled(hasSelection);
    ui->actionSelectNone->setEnabled(hasSelection);
    //ui->actionInvertSelection->setEnabled(hasSelection);
    //ui->actionSelectCompleted->setEnabled(hasSelection);
    // --
    ui->actionManageMirrors->setEnabled(hasAtLeastOneUncompletedSelected);
    ui->actionOneMoreSegment->setEnabled(hasAtLeastOneUncompletedSelected);
    ui->actionOneFewerSegment->setEnabled(hasAtLeastOneUncompletedSelected);
    //! [1]

    //! [2] View
    //ui->actionInformation->setEnabled(hasOnlyOneSelected);
    // --
    //ui->actionOpenFile->setEnabled(hasOnlyCompletedSelected);
    //ui->actionRenameFile->setEnabled(hasOnlyOneSelected);
    //ui->actionDeleteFile->setEnabled(hasOnlyCompletedSelected);
    //ui->actionOpenDirectory->setEnabled(hasOnlyOneSelected);
    // --
    ui->actionRemoveCompleted->setEnabled(hasJobs);
    ui->actionRemoveAll->setEnabled(hasJobs);
    ui->actionRemoveWaiting->setEnabled(hasJobs);
    // --
    ui->actionRemoveSelected->setEnabled(hasJobs);
    ui->actionRemoveDuplicates->setEnabled(hasJobs);
    ui->actionRemoveFailed->setEnabled(hasJobs);
    ui->actionRemovePaused->setEnabled(hasJobs);
    //! [2]

    //! [3] Download
    //ui->actionAdd->setEnabled(hasSelection);
    //--
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
    ui->actionSpeedLimit->setEnabled(hasSelection);
    ui->actionAddDomainSpecificLimit->setEnabled(hasSelection);
    //--
    ui->actionForceStart->setEnabled(hasSelection);
    //--
    //ui->actionPreferences->setEnabled(hasSelection);
    //! [4]

    //! [5] Help
    //ui->actionAbout->setEnabled(hasSelection);
    //ui->actionAboutQt->setEnabled(hasSelection);
    //! [5]
}
