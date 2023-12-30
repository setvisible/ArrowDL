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

#include "about.h"

#include <Constants>
#include <Core/IDownloadItem>
#include <Core/DownloadManager>
#include <Core/DownloadTorrentItem>
#include <Core/FileAccessManager>
#include <Core/Format>
#include <Core/Locale>
#include <Core/Settings>
#include <Core/StreamManager>
#include <Core/Theme>
#include <Core/Torrent>
#include <Core/TorrentContext>
#include <Core/TorrentMessage>
#include <Core/UpdateChecker>
#include <Dialogs/AddBatchDialog>
#include <Dialogs/AddContentDialog>
#include <Dialogs/AddStreamDialog>
#include <Dialogs/AddTorrentDialog>
#include <Dialogs/AddUrlsDialog>
#include <Dialogs/BatchRenameDialog>
#include <Dialogs/CompilerDialog>
#include <Dialogs/EditionDialog>
#include <Dialogs/HomeDialog>
#include <Dialogs/InformationDialog>
#include <Dialogs/PreferenceDialog>
#include <Dialogs/StreamDialog>
#include <Dialogs/TutorialDialog>
#include <Dialogs/UpdateDialog>
#include <Ipc/InterProcessCommunication>
#include <Io/FileReader>
#include <Io/FileWriter>
#include <Widgets/DownloadQueueView>
#include <Widgets/SystemTray>
#include <Widgets/TorrentWidget>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QMimeData>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>
#include <QtGui/QAction>
#include <QtGui/QClipboard>
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopServices>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QScreen>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QSplitter>

#ifdef USE_QT_WINEXTRAS
#  include <QtWinExtras/QWinTaskbarButton>
#  include <QtWinExtras/QWinTaskbarProgress>
#endif



MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_downloadManager(new DownloadManager(this))
  , m_streamManager(new StreamManager(this))
  , m_fileAccessManager(new FileAccessManager(this))
  , m_settings(new Settings(this))
  , m_statusBarLabel(new QLabel(this))
  , m_updateChecker(new UpdateChecker(this))
  , m_systemTray(new SystemTray(this))
{
    ui->setupUi(this);

    m_downloadManager->setSettings(m_settings);

    m_streamManager->setSettings(m_settings);

    TorrentContext& torrentContext = TorrentContext::getInstance();
    torrentContext.setSettings(m_settings);
    torrentContext.setNetworkManager(m_downloadManager->networkManager());

    m_updateChecker->setNetworkManager(m_downloadManager->networkManager());

    Qt::WindowFlags flags = Qt::Window
            | Qt::WindowTitleHint
            | Qt::WindowSystemMenuHint
            | Qt::WindowMinimizeButtonHint
            | Qt::WindowMaximizeButtonHint
            | Qt::WindowCloseButtonHint
            // | Qt::WindowFullscreenButtonHint
            // | Qt::WindowShadeButtonHint
            // | Qt::WindowStaysOnTopHint
            // | Qt::WindowStaysOnBottomHint
            | Qt::WindowContextHelpButtonHint; // "What's this"
    this->setWindowFlags(flags);
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    this->setAcceptDrops(true);
#ifdef Q_OS_OSX
    this->setUnifiedTitleAndToolBarOnMac(true);
#endif    
#ifdef USE_QT_WINEXTRAS
    m_winTaskbarButton = new QWinTaskbarButton(this);
    m_winTaskbarButton->setWindow(this->windowHandle());
    m_winTaskbarProgress = m_winTaskbarButton->progress();
    m_winTaskbarProgress->setVisible(false);
#endif

    /* Connect the GUI to the DownloadManager. */
    ui->downloadQueueView->setEngine(m_downloadManager);

    /* Connect the GUI to the TorrentContext. */
    ui->torrentWidget->setTorrentContext(&torrentContext);

    /* Connect the SceneManager to the MainWindow. */
    /* The SceneManager centralizes the changes. */
    connect(m_downloadManager, SIGNAL(jobAppended(DownloadRange)), this, SLOT(onJobAddedOrRemoved(DownloadRange)));
    connect(m_downloadManager, SIGNAL(jobRemoved(DownloadRange)), this, SLOT(onJobAddedOrRemoved(DownloadRange)));
    connect(m_downloadManager, SIGNAL(jobStateChanged(IDownloadItem*)), this, SLOT(onJobStateChanged(IDownloadItem*)));
    connect(m_downloadManager, SIGNAL(jobFinished(IDownloadItem*)), this, SLOT(onJobFinished(IDownloadItem*)));
    connect(m_downloadManager, SIGNAL(jobRenamed(QString,QString,bool)), this, SLOT(onJobRenamed(QString,QString,bool)), Qt::QueuedConnection);
    connect(m_downloadManager, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));

    connect(ui->downloadQueueView, SIGNAL(doubleClicked(IDownloadItem*)), this, SLOT(openFile(IDownloadItem*)));

    /* Torrent Context Manager */
    connect(&torrentContext, &TorrentContext::changed, this, &MainWindow::onTorrentContextChanged);

    /* File Access Manager */
    m_fileAccessManager->setSettings(m_settings);

    /* Connect the rest of the GUI widgets together (selection, focus, etc.) */
    createActions();
    createContextMenu();
    createStatusbar();
    createSystemTray();

    readSettings();

    refreshTitleAndStatus();
    refreshMenus();

    Locale::applyLanguage(m_settings->language());
    Theme::applyTheme(m_settings->theme());

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 0);

    if (!m_settings->isDontShowTutorialEnabled()) {
        QTimer::singleShot(TIMEOUT_TUTORIAL, this, SLOT(showTutorial()));
    }

    /* Update Checker */
    connect(m_updateChecker, SIGNAL(updateAvailableForConsole()), this, SLOT(onUpdateAvailableForConsole()));
    m_updateChecker->checkForUpdates(m_settings);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    m_systemTray->close();
    event->accept();
}

void MainWindow::showEvent(QShowEvent *event)
{
#ifdef USE_QT_WINEXTRAS
    m_winTaskbarButton->setWindow(windowHandle());
#endif
    event->accept();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if ( m_settings->isSystemTrayIconEnabled() &&
             m_settings->isHideWhenMinimizedEnabled()
             && isVisible() && isMinimized()
             ) {
            m_systemTray->hideParentWidget();
        }
    } else if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        createContextMenu();
        propagateToolTips(); // propagate tooltips translations
        refreshTitleAndStatus();
        refreshMenus();
        refreshSplitter();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape
            && m_settings->isMinimizeEscapeEnabled()) {
        setWindowState(Qt::WindowMinimized);
    }
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    auto url = droppedUrl(event->mimeData());
    if (url.isValid()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    auto url = droppedUrl(event->mimeData());
    if (url.isValid()) {
        if (url.isLocalFile()) {
            loadFile(url.toLocalFile());
        } else {
            addBatch(url);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::createActions()
{
    //! [0] File
    connect(ui->actionHome, &QAction::triggered, this, &MainWindow::home);
    //--
    connect(ui->actionAddContent, SIGNAL(triggered()), this, SLOT(addContent()));
    connect(ui->actionAddBatch,   SIGNAL(triggered()), this, SLOT(addBatch()));
    connect(ui->actionAddStream,  SIGNAL(triggered()), this, SLOT(addStream()));
    connect(ui->actionAddTorrent, SIGNAL(triggered()), this, SLOT(addTorrent()));
    connect(ui->actionAddUrls,    SIGNAL(triggered()), this, SLOT(addUrls()));
    // --
    connect(ui->actionImportFromFile, SIGNAL(triggered()), this, SLOT(importFromFile()));
    connect(ui->actionExportSelectedToFile, SIGNAL(triggered()), this, SLOT(exportSelectedToFile()));
    // --
    ui->actionQuit->setShortcuts(QKeySequence::Quit);
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close())); //&QWidget::close
    //! [0]

    //! [1] Edit
    connect(ui->actionSelectAll, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(ui->actionSelectNone, SIGNAL(triggered()), this, SLOT(selectNone()));
    connect(ui->actionInvertSelection, SIGNAL(triggered()), this, SLOT(invertSelection()));
    connect(ui->actionSelectCompleted, SIGNAL(triggered()), this, SLOT(selectCompleted()));
    // --
    connect(ui->actionCopy, SIGNAL(triggered()), this, SLOT(copy()));
    // --
    connect(ui->actionManageMirrors, SIGNAL(triggered()), this, SLOT(manageMirrors()));
    connect(ui->actionOneMoreSegment, SIGNAL(triggered()), this, SLOT(oneMoreSegment()));
    connect(ui->actionOneFewerSegment, SIGNAL(triggered()), this, SLOT(oneFewerSegment()));
    //! [1]

    //! [2] View
    connect(ui->actionInformation, SIGNAL(triggered()), this, SLOT(showInformation()));
    // --
    connect(ui->actionOpenFile, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui->actionRenameFile, SIGNAL(triggered()), this, SLOT(renameFile()));
    connect(ui->actionDeleteFile, SIGNAL(triggered()), this, SLOT(deleteFile()));
    connect(ui->actionOpenDirectory, SIGNAL(triggered()), this, SLOT(openDirectory()));
    // --
    connect(ui->actionRemoveCompleted, SIGNAL(triggered()), this, SLOT(removeCompleted()));
    connect(ui->actionRemoveSelected, SIGNAL(triggered()), this, SLOT(removeSelected()));
    connect(ui->actionRemoveAll, SIGNAL(triggered()), this, SLOT(removeAll()));
    // --
    connect(ui->actionRemoveWaiting, SIGNAL(triggered()), this, SLOT(removeWaiting()));
    connect(ui->actionRemoveDuplicates, SIGNAL(triggered()), this, SLOT(removeDuplicates()));
    connect(ui->actionRemoveRunning, SIGNAL(triggered()), this, SLOT(removeRunning()));
    connect(ui->actionRemovePaused, SIGNAL(triggered()), this, SLOT(removePaused()));
    connect(ui->actionRemoveFailed, SIGNAL(triggered()), this, SLOT(removeFailed()));
    //! [2]

    //! [3] Download
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
    //--
    connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(showPreferences()));
    //! [4]

    //! [5] Help
    connect(ui->actionCheckForUpdates, SIGNAL(triggered()), this, SLOT(checkForUpdates()));
    connect(ui->actionTutorial, SIGNAL(triggered()), this, SLOT(showTutorial()));

    ui->actionAbout->setShortcuts(QKeySequence::HelpContents);
    ui->actionAbout->setToolTip(tr("About %0").arg(STR_APPLICATION_NAME));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));

    ui->actionAboutQt->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F1));
    ui->actionAboutQt->setToolTip(tr("About Qt"));
    connect(ui->actionAboutQt, SIGNAL(triggered()), QApplication::instance(), SLOT(aboutQt()));

    connect(ui->actionAboutCompiler, SIGNAL(triggered()), this, SLOT(aboutCompiler()));
    connect(ui->actionAboutYTDLP, SIGNAL(triggered()), this, SLOT(aboutStream()));
    //! [5]

    propagateToolTips();
    propagateIcons();
}

void MainWindow::createContextMenu()
{
    // delete previous menu if any
    QMenu *contextMenu = ui->downloadQueueView->contextMenu();
    ui->downloadQueueView->setContextMenu(nullptr);
    if (contextMenu) {
        delete contextMenu;
        contextMenu = nullptr;
    }

    contextMenu = new QMenu(this);

    contextMenu->addAction(ui->actionInformation);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionOpenFile);
    contextMenu->addAction(ui->actionRenameFile);
    contextMenu->addAction(ui->actionDeleteFile);
    contextMenu->addAction(ui->actionOpenDirectory);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionCopy);
    contextMenu->addSeparator();
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
    remove->addAction(ui->actionRemoveRunning);
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
    advanced->addAction(ui->actionImportFromFile);
    advanced->addAction(ui->actionExportSelectedToFile);
    advanced->addAction(ui->actionAddDomainSpecificLimit);

    ui->downloadQueueView->setContextMenu(contextMenu);
}

void MainWindow::createStatusbar()
{
    this->statusBar()->addPermanentWidget(m_statusBarLabel);
    this->statusBar()->addAction(ui->actionPreferences);
}

void MainWindow::createSystemTray()
{    
    m_systemTray->setSettings(m_settings);
    m_systemTray->setupContextMenu(
                ui->actionPreferences,
                ui->actionQuit);
}

void MainWindow::propagateToolTips()
{
    // Propagate tooltip to whatsThis and statusTip
    QList<QAction*> actions = this->findChildren<QAction*>();
    for (auto *action : actions) {
        if (!action->isSeparator()) {
            auto str = action->toolTip();
            action->setWhatsThis(str);
            action->setStatusTip(str);
        }
    }
}

void MainWindow::propagateIcons()
{
    const QHash<QAction*, QString> hash = {

        //! [0] File
        {ui->actionHome                   , "home"},
        //--
        {ui->actionAddContent             , "add-content"},
        {ui->actionAddBatch               , "add-batch"},
        {ui->actionAddStream              , "add-stream"},
        {ui->actionAddTorrent             , "add-torrent"},
        {ui->actionAddUrls                , "add-urls"},
        // --
        {ui->actionImportFromFile         , "file-import"},
        {ui->actionExportSelectedToFile   , "file-export"},
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
        {ui->actionInformation            , "info"},
        // --
        {ui->actionOpenFile               , "file-open"},
        {ui->actionRenameFile             , "rename"},
        {ui->actionDeleteFile             , "file-delete"},
        {ui->actionOpenDirectory          , "directory-open"},
        // --
        {ui->actionRemoveCompleted        , "remove-completed"},
        {ui->actionRemoveSelected         , "remove-downloaded"},
        {ui->actionRemoveAll              , "remove-all"},
        // --
        {ui->actionRemoveWaiting          , "remove-waiting"},
        {ui->actionRemoveDuplicates       , "remove-duplicates"},
        {ui->actionRemoveRunning          , "remove-resumed"},
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
        {ui->actionPreferences            , "preference"},
        //! [4]

        //! [5] Help
        // {ui->actionCheckForUpdates        , ""},
        // {ui->actionTutorial               , ""},
        {ui->actionAbout                  , "about"}
        // {ui->actionAboutQt                , ""},
        // {ui->actionAboutCompiler          , ""},
        // {ui->actionAboutYTDLP             , ""}
        //! [5]
    };
    Theme::setIcons(this, hash);
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::importFromFile()
{
    const QString filePath = askOpenFileName(FileReader::supportedFormats());
    if (!filePath.isEmpty()) {
        setWorkingDirectory(filePath);
        loadFile(filePath);
    }
}

void MainWindow::exportSelectedToFile()
{
    const QString filePath = askSaveFileName(FileWriter::supportedFormats());
    if (!filePath.isEmpty()) {
        setWorkingDirectory(filePath);
        saveFile(filePath);
    }
}

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
    QList<IDownloadItem *> inverted;
    for (auto item : m_downloadManager->downloadItems()) {
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

void MainWindow::copy()
{
    const QString text = m_downloadManager->selectionToClipboard();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void MainWindow::manageMirrors()
{
    qWarning("todo: manageMirrors() not implemented yet.");
}

void MainWindow::oneMoreSegment()
{
    m_downloadManager->oneMoreSegment();
}

void MainWindow::oneFewerSegment()
{
    m_downloadManager->oneFewerSegment();
}

void MainWindow::showInformation()
{
    if (m_downloadManager->selection().count() == 1) {
        InformationDialog dialog(m_downloadManager->selection(), this);
        int answer = dialog.exec();
        if (answer == QDialog::Accepted) {
            m_downloadManager->updateItems(m_downloadManager->selection());
        }
    } else if (m_downloadManager->selection().count() > 1) {
        EditionDialog dialog(m_downloadManager->selection(), this);
        int answer = dialog.exec();
        if (answer == QDialog::Accepted) {
            m_downloadManager->updateItems(m_downloadManager->selection());
        }
    }
}

void MainWindow::openFile()
{
    if (!m_downloadManager->selection().isEmpty()) {
        auto item = m_downloadManager->selection().first();
        if (item->state() == IDownloadItem::Completed) {
            openFile(item);
            return;
        }
    }
}

void MainWindow::openFile(IDownloadItem *downloadItem)
{
    auto url = downloadItem->localFileUrl();
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::information(
                    this, tr("Error"),
                    QString("%0:\n\n%1").arg(
                        tr("File not found"),
                        url.toLocalFile()));
    }
}

void MainWindow::renameFile()
{
    if (m_downloadManager->selection().count() == 1) {
        ui->downloadQueueView->rename();
    } else if (m_downloadManager->selection().count() > 1) {
        BatchRenameDialog dialog(m_downloadManager->selection(), this);
        int answer = dialog.exec();
        if (answer == QDialog::Accepted) {
            m_downloadManager->updateItems(m_downloadManager->selection());
        }
    }
}

void MainWindow::deleteFile()
{
    if (!m_downloadManager->selection().isEmpty()) {
        QString text = m_downloadManager->selectionToString();

        QMessageBox msgbox(this);
        msgbox.setWindowTitle(tr("Remove Downloads"));
        msgbox.setText(tr("Are you sure to remove %0 downloads?").arg(text));
        msgbox.setIcon(QMessageBox::Icon::Question);
        QAbstractButton *deleteButton = msgbox.addButton(tr("Delete"), QMessageBox::ActionRole);
        msgbox.addButton(QMessageBox::Cancel);
        msgbox.setDefaultButton(QMessageBox::Cancel);

        msgbox.exec();
        if (msgbox.clickedButton() == deleteButton) {
            qWarning("todo: MoveToTrash() not implemented yet.");
        }
    }
}

void MainWindow::openDirectory()
{
    if (!m_downloadManager->selection().isEmpty()) {
        auto item = m_downloadManager->selection().first();
        auto url = item->localDirUrl();
        if (!QDesktopServices::openUrl(url)) {
            QMessageBox::information(
                        this, tr("Error"),
                        QString("%0\n\n%1").arg(
                            tr("Destination directory not found:"),
                            url.toLocalFile()));
        }
    }
}

/******************************************************************************
 ******************************************************************************/
bool MainWindow::askConfirmation(const QString &text)
{
    if (m_settings->isConfirmRemovalEnabled()) {
        QMessageBox msgbox(this);
        msgbox.setWindowTitle(tr("Remove Downloads"));
        msgbox.setText(tr("Are you sure to remove %0 downloads?").arg(text));
        msgbox.setIcon(QMessageBox::Icon::Question);
        msgbox.addButton(QMessageBox::Yes);
        msgbox.addButton(QMessageBox::No);
        msgbox.setDefaultButton(QMessageBox::No);

        QCheckBox *cb = new QCheckBox(tr("Don't ask again"));
        msgbox.setCheckBox(cb);
        QObject::connect(cb, &QCheckBox::stateChanged, [this](int state){
            if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
                m_settings->setConfirmRemovalEnabled(false);
            }
        });

        int response = msgbox.exec();
        return response == QMessageBox::Yes;
    }
    return true;
}

void MainWindow::removeAll()
{    
    if (askConfirmation(tr("ALL"))) {
        m_downloadManager->remove(m_downloadManager->downloadItems());
    }
}

void MainWindow::removeSelected()
{
    if (askConfirmation(tr("selected"))) {
        m_downloadManager->remove(m_downloadManager->selection());
    }
}

void MainWindow::removeDuplicates()
{
    qWarning("todo: removeDuplicates() not implemented yet.");
}

void MainWindow::removeCompleted()
{
    if (askConfirmation(tr("completed"))) {
        m_downloadManager->remove(m_downloadManager->completedJobs());
    }
}

void MainWindow::removeWaiting()
{
    if (askConfirmation(tr("waiting"))) {
        m_downloadManager->remove(m_downloadManager->waitingJobs());
    }
}

void MainWindow::removePaused()
{
    if (askConfirmation(tr("paused"))) {
        m_downloadManager->remove(m_downloadManager->pausedJobs());
    }
}

void MainWindow::removeFailed()
{
    if (askConfirmation(tr("failed"))) {
        m_downloadManager->remove(m_downloadManager->failedJobs());
    }
}


void MainWindow::removeRunning()
{
    if (askConfirmation(tr("running"))) {
        m_downloadManager->remove(m_downloadManager->runningJobs());
    }
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::handleMessage(const QString &message)
{
    // qDebug() << Q_FUNC_INFO << message;
    const QString cleaned = InterProcessCommunication::clean(message);
    if (!cleaned.isEmpty()) {

        if (InterProcessCommunication::isSingleUrl(cleaned)) {
            const QUrl url = QUrl::fromUserInput(cleaned);
            if (AddTorrentDialog::isTorrentUrl(url)) {
                addTorrent(url);

            } else if (AddStreamDialog::isStreamUrl(url, m_settings)) {
                addStream(url);

            } else {
                // Assume the URL is a HTML page address,
                // so that the program downloads the content
                addContent(url);
            }

        } else if(InterProcessCommunication::isCommandOpenUrl(cleaned)) {
            const QString str = InterProcessCommunication::getCurrentUrl(cleaned);
            const QUrl url = QUrl::fromUserInput(str);
            addContent(url);

        } else if(InterProcessCommunication::isCommandDownloadLink(cleaned)) {
            const QString str = InterProcessCommunication::getDownloadLink(cleaned);
            const QUrl url = QUrl::fromUserInput(str);

            if (AddTorrentDialog::isTorrentUrl(url)) {
                addTorrent(url);

            } else if (AddStreamDialog::isStreamUrl(url, m_settings)) {
                addStream(url);

            } else {
                AddBatchDialog::quickDownload(url, m_downloadManager);
            }

        } else if(InterProcessCommunication::isCommandOpenManager(cleaned)) {
            // Try to popup the window.
            // Rem: on Windows, it's not possible for an application
            // to raise its main window itself, by design.
            // See QWidget::activateWindow() doc, section "behavior under windows"
            activateWindow();

        } else if(InterProcessCommunication::isCommandShowPreferences(cleaned)) {
            showPreferences();

        } else {
            // Otherwise, assume it's a list of resources
            addContent(cleaned);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::home()
{
    HomeDialog dialog(this);
    int reply = dialog.exec();
    if (reply == static_cast<int>(HomeDialog::Content)) {
        addContent();
    } else if (reply == static_cast<int>(HomeDialog::Batch)) {
        addBatch();
    } else if (reply == static_cast<int>(HomeDialog::Stream)) {
        addStream();
    } else if (reply == static_cast<int>(HomeDialog::Torrent)) {
        addTorrent();
    } else if (reply == static_cast<int>(HomeDialog::Urls)) {
        addUrls();
    }
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::addContent()
{
    // ask for the Url
    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("Website URL"));
    dialog.setLabelText(QString("%0  %1").arg(
                            tr("URL of the HTML page:"),
                            tr("(ex: %0)").arg(
                                QLatin1String("\"https://www.site.com/folder/page\""))));
    dialog.setTextValue(urlFromClipboard().toString());
    dialog.setOkButtonText(tr("Start!"));
    dialog.setCancelButtonText(tr("Cancel"));
    dialog.setTextEchoMode(QLineEdit::Normal);
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setInputMethodHints(Qt::ImhUrlCharactersOnly);
    dialog.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    dialog.adjustSize();
    dialog.resize(DIALOG_WIDTH, dialog.height());

    const int ret = dialog.exec();
    const QUrl url = QUrl(dialog.textValue());
    if (ret && !url.isEmpty()) {
        addContent(url);
    }
}

void MainWindow::addContent(const QUrl &url)
{
    AddContentDialog dialog(m_downloadManager, m_settings, this);
    dialog.loadUrl(url);
    dialog.exec();
}

void MainWindow::addContent(const QString &message)
{
    // Note: if the message asks for the dialog (i.e. not a silent download),
    // we must show the mainwindow first, if it was minimized.
    // In this case, once the dialog closed, we hide the mainwindow.
    bool wasHidden = !this->isVisible();

    /// \todo decouple the dialog and the dialog's model,
    /// in order to not call "dialog.exec()" when it's a silent download

    AddContentDialog dialog(m_downloadManager, m_settings, this);
    bool willShowDialog = dialog.loadResources(message);

    if (willShowDialog && wasHidden) {
        m_systemTray->showParentWidget();
    }

    dialog.exec();

    if (willShowDialog && wasHidden) {
        m_systemTray->hideParentWidget();
    }
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::addBatch()
{
    addBatch(urlFromClipboard());
}

void MainWindow::addBatch(const QUrl &url)
{
    AddBatchDialog dialog(url, m_downloadManager, m_settings, this);
    dialog.exec();
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::addStream()
{
    addStream(urlFromClipboard());
}

void MainWindow::addStream(const QUrl &url)
{
    AddStreamDialog dialog(url, m_downloadManager, m_settings, this);
    dialog.exec();
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::addTorrent()
{
    addTorrent(urlFromClipboard());
}

void MainWindow::addTorrent(const QUrl &url)
{
    AddTorrentDialog dialog(url, m_downloadManager, m_settings, this);
    dialog.exec();
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::addUrls()
{
    addUrls(fromClipboard());
}

void MainWindow::addUrls(const QString &text)
{
    AddUrlsDialog dialog(text, m_downloadManager, m_settings, this);
    dialog.exec();
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::resume()
{
    for (auto item : m_downloadManager->selection()) {
        m_downloadManager->resume(item);
    }
}

void MainWindow::cancel()
{
    for (auto item : m_downloadManager->selection()) {
        m_downloadManager->cancel(item);
    }
}

void MainWindow::pause()
{
    for (auto item : m_downloadManager->selection()) {
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
    qWarning("todo: speedLimit() not implemented yet.");
}

void MainWindow::addDomainSpecificLimit()
{
    qWarning("todo: addDomainSpecificLimit() not implemented yet.");
}

void MainWindow::forceStart()
{
    for (auto item : m_downloadManager->selection()) {
        /// todo Maybe run the item instantly (in a higher priority queue?)
        m_downloadManager->cancel(item);
        m_downloadManager->resume(item);
    }
}

void MainWindow::showPreferences()
{
    if (!this->isVisible()) {
        m_systemTray->showParentWidget();
    }
    PreferenceDialog dialog(m_settings, this);
    connect(&dialog, SIGNAL(checkUpdate()), this, SLOT(checkForUpdates()));
    dialog.exec();
}

void MainWindow::showTutorial()
{
    TutorialDialog dialog(m_settings, this);
    dialog.exec();
}

void MainWindow::onUpdateAvailableForConsole()
{
    checkForUpdates();
}

void MainWindow::checkForUpdates()
{
    disconnect(m_updateChecker, SIGNAL(updateAvailableForConsole()), this, SLOT(onUpdateAvailableForConsole()));
    UpdateDialog dialog(m_updateChecker, this);
    dialog.exec();
}

void MainWindow::about()
{
    QMessageBox msgBox(QMessageBox::NoIcon, tr("About %0").arg(STR_APPLICATION_NAME), aboutHtml());
    msgBox.exec();
}

void MainWindow::aboutCompiler()
{
    CompilerDialog dialog(this);
    dialog.exec();
}

void MainWindow::aboutStream()
{
    StreamDialog dialog(this);
    dialog.exec();
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onJobAddedOrRemoved(const DownloadRange &/*range*/)
{
    refreshTitleAndStatus();
}

void MainWindow::onJobStateChanged(IDownloadItem * /*downloadItem*/)
{
    // if (m_downloadManager->isSelected(downloadItem)) {
    refreshMenus();
    // }
    refreshTitleAndStatus();
}

void MainWindow::onJobFinished(IDownloadItem * downloadItem)
{
    refreshMenus();
    refreshTitleAndStatus();
    m_systemTray->showBalloon(downloadItem->localFileName(), downloadItem->localFullFileName());
}

void MainWindow::onSelectionChanged()
{
    refreshMenus();
    refreshSplitter();
}

void MainWindow::onJobRenamed(const QString &oldName, const QString &newName, bool success)
{
    if (!success) {
        const QString comment = tr("The new name is already used or invalid.");
        const QString message = newName.isEmpty()
                ? QString("%0 \n\n%1").arg(
                      tr("Can't rename \"%0\" as its initial name.").arg(oldName),
                      comment)
                : QString("%0\n"
                          "   \"%1\"\n"
                          "%2\n"
                          "   \"%3\"\n\n"
                          "%4").arg(
                      tr("Can't rename"), oldName, tr("as"), newName, comment);
        QMessageBox::information(this, tr("File Error"), message);
    }
}

void MainWindow::onTorrentContextChanged()
{
    refreshTitleAndStatus();
}

void MainWindow::refreshTitleAndStatus()
{
    auto speed = m_downloadManager->totalSpeed();
    QString totalSpeed;
    if (speed > 0) {
        totalSpeed = QString("~%0").arg(Format::currentSpeedToString(speed));
    }
    auto completedCount = m_downloadManager->completedJobs().count();
    auto runningCount = m_downloadManager->runningJobs().count();
    auto failedCount = m_downloadManager->failedJobs().count();
    auto count = m_downloadManager->count();
    auto doneCount = completedCount + failedCount;

    auto torrent = TorrentContext::getInstance().isEnabled();

    auto windowTitle = QString("%0 %1/%2 - %3 v%4").arg(
                totalSpeed,
                QString::number(doneCount),
                QString::number(count),
                STR_APPLICATION_NAME,
                STR_APPLICATION_VERSION).trimmed();

    this->setWindowTitle(windowTitle);

    auto title = tr("Done: %0 Running: %1 Total: %2").arg(
                QString::number(doneCount),
                QString::number(runningCount),
                QString::number(count));

    m_systemTray->setTitle(title);
    m_systemTray->setToolTip(windowTitle);

    auto state = tr("%0 of %1 (%2), %3 running  %4 | Torrent: %5").arg(
                QString::number(doneCount),
                QString::number(count),
                QString::number(count),
                QString::number(runningCount),
                totalSpeed,
                torrent ? tr("active") : tr("inactive"));

    m_statusBarLabel->setText(state);

#ifdef USE_QT_WINEXTRAS
    if (m_winTaskbarProgress) {
        if (runningCount > 0) {
            m_winTaskbarProgress->setVisible(true);
            m_winTaskbarProgress->setRange(0, count);
            m_winTaskbarProgress->setValue(doneCount);
            m_winTaskbarProgress->resume();
        } else if (failedCount > 0) {
            m_winTaskbarProgress->setVisible(true);
            m_winTaskbarProgress->setRange(0, 100);
            m_winTaskbarProgress->setValue(100);
            m_winTaskbarProgress->stop();
        } else {
            m_winTaskbarProgress->setVisible(false);
        }
    }
#endif
}

void MainWindow::refreshMenus()
{
    const bool hasJobs = !m_downloadManager->downloadItems().isEmpty();
    const bool hasSelection = !m_downloadManager->selection().isEmpty();
    const bool hasOnlyOneSelected = m_downloadManager->selection().count() == 1;
    bool hasOnlyCompletedSelected = hasSelection;
    for (auto item : m_downloadManager->selection()) {
        if (item->state() != IDownloadItem::Completed) {
            hasOnlyCompletedSelected = false;
            continue;
        }
    }
    bool hasAtLeastOneUncompletedSelected = false;
    for (auto item : m_downloadManager->selection()) {
        if (item->state() != IDownloadItem::Completed) {
            hasAtLeastOneUncompletedSelected = true;
            continue;
        }
    }
    bool hasResumableSelection = false;
    bool hasPausableSelection = false;
    bool hasCancelableSelection = false;
    for (auto item : m_downloadManager->selection()) {
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
    //ui->actionImportFromFile->setEnabled(hasSelection);
    ui->actionExportSelectedToFile->setEnabled(hasSelection);
    // --
    //ui->actionExit->setEnabled(hasSelection);
    //! [0]

    //! [1] Edit
    //ui->actionSelectAll->setEnabled(hasSelection);
    ui->actionSelectNone->setEnabled(hasSelection);
    //ui->actionInvertSelection->setEnabled(hasSelection);
    //ui->actionSelectCompleted->setEnabled(hasSelection);
    // --
    ui->actionCopy->setEnabled(hasSelection);
    // --
    ui->actionManageMirrors->setEnabled(hasAtLeastOneUncompletedSelected);
    ui->actionOneMoreSegment->setEnabled(hasAtLeastOneUncompletedSelected);
    ui->actionOneFewerSegment->setEnabled(hasAtLeastOneUncompletedSelected);
    //! [1]

    //! [2] View
    ui->actionInformation->setEnabled(hasSelection);
    // --
    ui->actionOpenFile->setEnabled(hasOnlyCompletedSelected);
    ui->actionRenameFile->setEnabled(hasSelection);
    ui->actionDeleteFile->setEnabled(hasOnlyCompletedSelected);
    ui->actionOpenDirectory->setEnabled(hasOnlyOneSelected);
    // --
    ui->actionRemoveCompleted->setEnabled(hasJobs);
    ui->actionRemoveSelected->setEnabled(hasSelection);
    ui->actionRemoveAll->setEnabled(hasJobs);
    // --
    ui->actionRemoveWaiting->setEnabled(hasJobs);
    ui->actionRemoveDuplicates->setEnabled(hasJobs);
    ui->actionRemoveRunning->setEnabled(hasJobs);
    ui->actionRemovePaused->setEnabled(hasJobs);
    ui->actionRemoveFailed->setEnabled(hasJobs);
    //! [2]

    //! [3] Download
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
    //ui->actionAboutCompiler->setEnabled(hasSelection);
    //! [5]
}

void MainWindow::refreshSplitter()
{
    if (m_downloadManager->selection().count() == 1) {
        auto item = m_downloadManager->selection().first();
        DownloadTorrentItem *torrentItem = dynamic_cast<DownloadTorrentItem*>(item);
        ui->torrentWidget->setTorrent(torrentItem ? torrentItem->torrent() : nullptr);
    } else {
        ui->torrentWidget->setTorrent(nullptr);
    }
    if (!ui->torrentWidget->isEmpty() /*&& option.showable */) {
        ui->torrentWidget->show();
    } else {
        ui->torrentWidget->hide();
    }
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::setWorkingDirectory(const QString &path)
{
    const QFileInfo fi(path); // in case it's a file
    const QString directory =  fi.isFile() ? fi.absolutePath() : fi.absoluteFilePath();
    QDir dir(directory);
    if (!dir.exists()) {
        dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    }
    if (!dir.exists()) {
        dir.setPath(QDir::homePath());
    }
    QDir::setCurrent(dir.absolutePath());
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::readSettings()
{
    QSettings settings;
    if ( !isMaximized() ) {
        const QPoint defaultPosition(DEFAULT_X, DEFAULT_Y);
        const QSize defaultSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
        QPoint position = settings.value("Position", defaultPosition).toPoint();
        QSize size = settings.value("Size", defaultSize).toSize();

        QRect availableGeometry(0, 0, 0, 0);
        for (auto screen : QApplication::screens()) {
            availableGeometry = availableGeometry.united(screen->availableGeometry());
        }

        if (!availableGeometry.intersects(QRect(position, size))) {
            position = defaultPosition;
            size = defaultSize;
        }
        this->move(position);
        this->resize(size);
    }
    setWindowState(settings.value("WindowState", 0).value<Qt::WindowStates>());
    setWorkingDirectory(settings.value("WorkingDirectory").toString());

    restoreState(settings.value("WindowToolbarState").toByteArray());
    ui->splitter->restoreState(settings.value("SplitterSizes").toByteArray());
    ui->downloadQueueView->restoreState(settings.value("DownloadQueueState").toByteArray());
    ui->torrentWidget->restoreState(settings.value("TorrentState").toByteArray());

    m_settings->readSettings();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    if( !(isMaximized() || isFullScreen()) ) {
        settings.setValue("Position", this->pos());
        settings.setValue("Size", this->size());
    }
    settings.setValue("WindowState", static_cast<int>(this->windowState())); // minimized, maximized, active, fullscreen...
    settings.setValue("WorkingDirectory", QDir::currentPath());

    settings.setValue("WindowToolbarState", saveState());
    settings.setValue("SplitterSizes", ui->splitter->saveState());
    settings.setValue("DownloadQueueState", ui->downloadQueueView->saveState());
    settings.setValue("TorrentState", ui->torrentWidget->saveState());

    // --------------------------------------------------------------
    // Write also the current version of application in the settings,
    // because it might be used by 3rd-party update manager softwares like
    // FileHippo or Google Updater (aka gup).
    // --------------------------------------------------------------
    settings.setValue("Version", STR_APPLICATION_VERSION );

    m_settings->writeSettings();
}

/******************************************************************************
 ******************************************************************************/
inline QUrl MainWindow::droppedUrl(const QMimeData* mimeData) const
{
    if ( mimeData->hasUrls() &&
         mimeData->urls().count() == 1) { // Accept only one file at a time
        return mimeData->urls().first();

    } if (!mimeData->hasUrls() && mimeData->hasText()) {
        auto supposedFilePath = mimeData->text();
        return QUrl(supposedFilePath);
    }
    return {};
}

inline QString MainWindow::fromClipboard() const
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    return mimeData->hasText() ? mimeData->text() : ""_L1;
}

inline QUrl MainWindow::urlFromClipboard() const
{
    return QUrl(fromClipboard());
}

/******************************************************************************
 ******************************************************************************/
QString MainWindow::askOpenFileName(const QString &fileFilter, const QString &title)
{
    return QFileDialog::getOpenFileName(this, title, QDir::currentPath(), fileFilter);
}

QString MainWindow::askSaveFileName(const QString &fileFilter, const QString &title)
{
    return QFileDialog::getSaveFileName(this, title, QDir::currentPath(), fileFilter);
}

/******************************************************************************
 ******************************************************************************/
bool MainWindow::saveFile(const QString &path)
{
    FileWriter writer(path);
    if (!writer.write(m_downloadManager)) {
        qWarning() << tr("Can't save file.");
        QMessageBox::warning(this, tr("Error"),
                             QString("%0\n%1").arg(
                                 tr("Can't save file %0:").arg(path),
                                 writer.errorString()));
        return false;
    }
    this->refreshTitleAndStatus();
    this->refreshMenus();
    this->statusBar()->showMessage(tr("File saved"), TIMEOUT_STATUSBAR.count());
    return true;
}

/******************************************************************************
 ******************************************************************************/
bool MainWindow::loadFile(const QString &path)
{
    FileReader reader(path);
    if (!reader.read(m_downloadManager)) {
        qWarning() << tr("Can't load file.");
        QMessageBox::warning(this, tr("Error"),
                             QString("%0\n%1").arg(
                                 tr("Can't load file %0:").arg(path),
                                 reader.errorString()));
        return false;
    }
    this->refreshTitleAndStatus();
    this->refreshMenus();
    this->statusBar()->showMessage(tr("File loaded"), TIMEOUT_STATUSBAR_LONG.count());
    return true;
}
