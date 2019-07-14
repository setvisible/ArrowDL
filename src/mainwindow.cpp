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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "about.h"
#include "version.h"
#include "globals.h"

#include <Core/DownloadItem>
#include <Core/DownloadManager>
#include <Core/Settings>
#include <Dialogs/AddDownloadDialog>
#include <Dialogs/InformationDialog>
#include <Dialogs/PreferenceDialog>
#include <Dialogs/WizardDialog>
#include <Widgets/DownloadQueueView>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QMimeData>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>
#include <QtGui/QCloseEvent>
#include <QtGui/QClipboard>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QAction>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>

#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , m_downloadManager(new DownloadManager(this))
  , m_settings(new Settings(this))
  , m_statusBarLabel(new QLabel(this))
  , m_showMessageBox(true)
{
    ui->setupUi(this);

    m_downloadManager->setSettings(m_settings);

    this->setWindowIcon(QIcon(":/icons/logo/maps-pin-place.ico"));
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setAcceptDrops(true);
#ifdef Q_OS_OSX
    this->setUnifiedTitleAndToolBarOnMac(true);
#endif

    /* Connect the GUI to the DownloadManager. */
    ui->downloadQueueView->setDownloadManager(m_downloadManager);

    /* Connect the SceneManager to the MainWindow. */
    /* The SceneManager centralizes the changes. */
    QObject::connect(m_downloadManager, SIGNAL(jobAppended(DownloadItem*)), this, SLOT(onJobAddedOrRemoved(DownloadItem*)));
    QObject::connect(m_downloadManager, SIGNAL(jobRemoved(DownloadItem*)), this, SLOT(onJobAddedOrRemoved(DownloadItem*)));
    QObject::connect(m_downloadManager, SIGNAL(jobStateChanged(DownloadItem*)), this, SLOT(onJobStateChanged(DownloadItem*)));
    QObject::connect(m_downloadManager, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));


    connect(ui->downloadQueueView, SIGNAL(doubleClicked(DownloadItem*)), this, SLOT(openFile(DownloadItem*)));

    /* Connect the rest of the GUI widgets together (selection, focus, etc.) */
    createActions();
    createContextMenu();
    createStatusbar();

    readSettings();

    refreshTitleAndStatus();
    refreshMenus();
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
    event->accept();
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::createActions()
{
    //! [0] File
    connect(ui->actionImportWizard, SIGNAL(triggered()), this, SLOT(openWizard()));
    // --
    connect(ui->actionImportFromFile, SIGNAL(triggered()), this, SLOT(importFromFile()));
    connect(ui->actionExportSelectedToFile, SIGNAL(triggered()), this, SLOT(exportSelectedToFile()));
    // --
    ui->actionExit->setShortcuts(QKeySequence::Quit);
    ui->actionExit->setStatusTip(tr("Quit %0").arg(STR_APPLICATION_NAME));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close())); //&QWidget::close
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
    connect(ui->actionInformation, SIGNAL(triggered()), this, SLOT(showInformation()));
    // --
    connect(ui->actionOpenFile, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui->actionRenameFile, SIGNAL(triggered()), this, SLOT(renameFile()));
    connect(ui->actionDeleteFile, SIGNAL(triggered()), this, SLOT(deleteFile()));
    connect(ui->actionOpenDirectory, SIGNAL(triggered()), this, SLOT(openDirectory()));
    // --
    connect(ui->actionRemoveCompletedDownloads, SIGNAL(triggered()), this, SLOT(removeCompletedDownloads()));
    connect(ui->actionRemoveAll1, SIGNAL(triggered()), this, SLOT(removeAll()));
    connect(ui->actionCleanGoneFiles, SIGNAL(triggered()), this, SLOT(cleanGoneFiles()));
    // --
    connect(ui->actionRemoveDownloads, SIGNAL(triggered()), this, SLOT(removeDownloads()));
    connect(ui->actionRemoveSelected, SIGNAL(triggered()), this, SLOT(removeSelected()));
    connect(ui->actionRemoveDuplicates, SIGNAL(triggered()), this, SLOT(removeDuplicates()));
    connect(ui->actionRemoveAll2, SIGNAL(triggered()), this, SLOT(removeAll()));
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
    //--
    connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(showPreferences()));
    //! [4]

    //! [5] Help
    ui->actionAbout->setShortcuts(QKeySequence::HelpContents);
    ui->actionAbout->setStatusTip(tr("About %0").arg(STR_APPLICATION_NAME));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));

    ui->actionAboutQt->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F1));
    ui->actionAboutQt->setStatusTip(tr("About Qt"));
    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    //! [5]
}

void MainWindow::createContextMenu()
{
    QMenu *contextMenu = new QMenu(this);

    contextMenu->addAction(ui->actionInformation);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionOpenFile);
    contextMenu->addAction(ui->actionRenameFile);
    contextMenu->addAction(ui->actionDeleteFile);
    contextMenu->addAction(ui->actionOpenDirectory);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionResume);
    contextMenu->addAction(ui->actionPause);
    contextMenu->addAction(ui->actionCancel);
    contextMenu->addSeparator();
    contextMenu->addAction(ui->actionRemoveCompletedDownloads);
    contextMenu->addAction(ui->actionRemoveDownloads);

    QMenu *remove = contextMenu->addMenu(tr("Other"));
    remove->addAction(ui->actionRemoveAll1);
    remove->addSeparator();
    remove->addAction(ui->actionCleanGoneFiles);
    remove->addSeparator();
    remove->addAction(ui->actionRemoveSelected);
    remove->addSeparator();
    remove->addAction(ui->actionRemoveDuplicates);
    remove->addSeparator();
    remove->addAction(ui->actionRemoveFailed);
    remove->addAction(ui->actionRemovePaused);

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

/******************************************************************************
 ******************************************************************************/
void MainWindow::openWizard()
{
    // ask for the Url
    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("Website URL"));
    dialog.setLabelText(tr("Url:"));
    dialog.setTextValue(urlFromClipboard().toString());
    dialog.setOkButtonText(tr("Start!"));
    dialog.setTextEchoMode(QLineEdit::Normal);
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setInputMethodHints(Qt::ImhUrlCharactersOnly);
    dialog.resize(600,400);

    const int ret = dialog.exec();
    const QUrl url = QUrl(dialog.textValue());
    if (ret && !url.isEmpty()) {
        openWizard(url);
    }
}

void MainWindow::openWizard(const QUrl &url)
{
    WizardDialog dialog(url, m_downloadManager, this);
    dialog.exec();
}

void MainWindow::importFromFile()
{
    QString filePath = askOpenFileName(tr("Data File (*.json);;All files (*.*)"));
    if (!filePath.isEmpty()) {
        if (loadFile(filePath)) {
            this->refreshTitleAndStatus();
            this->refreshMenus();
        }
    }
}

void MainWindow::exportSelectedToFile()
{
    QString filePath = askSaveFileName(QStringLiteral("Data File (*.json)"),
                                       tr("Data File"));
    if (filePath.isEmpty()) {
        return;
    }
    saveFile(filePath);
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
    QList<DownloadItem*> inverted;
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
    foreach (auto item, m_downloadManager->selection()) {
        int segments = item->maxConnectionSegments();
        segments++;
        item->setMaxConnectionSegments(segments);
    }
}

void MainWindow::oneFewerSegment()
{
    foreach (auto item, m_downloadManager->selection()) {
        int segments = item->maxConnectionSegments();
        segments--;
        item->setMaxConnectionSegments(segments);
    }
}

void MainWindow::showInformation()
{
    if (!m_downloadManager->selection().isEmpty()) {
        InformationDialog dialog(m_downloadManager->selection(), this);
        dialog.exec();
    }
}

void MainWindow::openFile()
{
    if (!m_downloadManager->selection().isEmpty()) {
        auto item = m_downloadManager->selection().first();
        if (item->state() == DownloadItem::Completed) {
            openFile(item);
            return;
        }
    }
}

void MainWindow::openFile(DownloadItem *downloadItem)
{
    auto url = downloadItem->localFileUrl();
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::information(
                    this, tr("Error"),
                    tr("File not found:\n\n%0")
                    .arg(url.toLocalFile()));
    }
}

void MainWindow::renameFile()
{
    qDebug() << Q_FUNC_INFO << "TODO rename File";
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

            qDebug() << Q_FUNC_INFO << "TODO Move to trash";
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
                        tr("Destination directory not found:\n\n%0")
                        .arg(url.toLocalFile()));
        }
    }
}

bool MainWindow::askConfirmation(const QString &text)
{
    if (m_showMessageBox) {
        QCheckBox *cb = new QCheckBox("Don't ask again");
        QMessageBox msgbox(this);
        msgbox.setWindowTitle(tr("Remove Downloads"));
        msgbox.setText(tr("Are you sure to remove %0 downloads?").arg(text));
        msgbox.setIcon(QMessageBox::Icon::Question);
        msgbox.addButton(QMessageBox::Yes);
        msgbox.addButton(QMessageBox::No);
        msgbox.setDefaultButton(QMessageBox::No);
        msgbox.setCheckBox(cb);

        QObject::connect(cb, &QCheckBox::stateChanged, [this](int state){
            if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
                m_showMessageBox = false;
            }
        });
        int response = msgbox.exec();
        return response == QMessageBox::Yes;
    }
    return true;
}

void MainWindow::cleanGoneFiles()
{
    if (askConfirmation(tr("waiting"))) {
        m_downloadManager->remove(m_downloadManager->waitingJobs());
    }
}

void MainWindow::removeAll()
{    
    if (askConfirmation(tr("ALL"))) {
        m_downloadManager->clear();
    }
}

void MainWindow::removeCompletedDownloads()
{
    if (askConfirmation(tr("completed"))) {
        m_downloadManager->remove(m_downloadManager->completedJobs());
    }
}

void MainWindow::removeDownloads()
{
    removeSelected();
}

void MainWindow::removeSelected()
{
    if (askConfirmation(tr("selected"))) {
        m_downloadManager->remove(m_downloadManager->selection());
    }
}

void MainWindow::removeDuplicates()
{
    qDebug() << Q_FUNC_INFO << "TODO remove Duplicates";
}

void MainWindow::removeFailed()
{
    if (askConfirmation(tr("failed"))) {
        m_downloadManager->remove(m_downloadManager->failedJobs());
    }
}

void MainWindow::removePaused()
{
    if (askConfirmation(tr("paused"))) {
        m_downloadManager->remove(m_downloadManager->pausedJobs());
    }
}

void MainWindow::add()
{
    AddDownloadDialog dialog(urlFromClipboard(), m_downloadManager, this);
    dialog.exec();
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
    qDebug() << Q_FUNC_INFO << "TODO";
}

void MainWindow::top()
{
    qDebug() << Q_FUNC_INFO << "TODO";
}

void MainWindow::down()
{
    qDebug() << Q_FUNC_INFO << "TODO";
}

void MainWindow::bottom()
{
    qDebug() << Q_FUNC_INFO << "TODO";
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
    qDebug() << Q_FUNC_INFO << "TODO";

    foreach (auto item, m_downloadManager->selection()) {
        /// todo Maybe run the item instantly (in a higher priority queue?)
        m_downloadManager->cancel(item);
        m_downloadManager->resume(item);
    }
}

void MainWindow::showPreferences()
{
    PreferenceDialog dialog(m_settings, this);
    dialog.exec();
}

void MainWindow::about()
{
    QMessageBox msgBox(QMessageBox::NoIcon, tr("About %0").arg(STR_APPLICATION_NAME), aboutHtml());
    msgBox.exec();
}

/******************************************************************************
 ******************************************************************************/
void MainWindow::onJobAddedOrRemoved(DownloadItem */*downloadItem*/)
{
    refreshTitleAndStatus();
}

void MainWindow::onJobStateChanged(DownloadItem *downloadItem)
{
    if (m_downloadManager->isSelected(downloadItem)) {
        refreshMenus();
    }
    refreshTitleAndStatus();
}

void MainWindow::onSelectionChanged()
{
    refreshMenus();
}

void MainWindow::refreshTitleAndStatus()
{
    const QString totalSpeed = m_downloadManager->totalSpeed();
    const int completedCount = m_downloadManager->completedJobs().count();
    const int runningCount = m_downloadManager->runningJobs().count();
    const int count = m_downloadManager->count();

    this->setWindowTitle(QString("%0 %1/%2 - %3 v%4")
                         .arg(totalSpeed)
                         .arg(completedCount)
                         .arg(count)
                         .arg(STR_APPLICATION_NAME)
                         .arg(STR_APPLICATION_VERSION).trimmed());

    m_statusBarLabel->setText(
                QString("%0 of %1 (%2), %3 running  %4")
                .arg(completedCount)
                .arg(count)
                .arg(count)
                .arg(runningCount)
                .arg(totalSpeed).trimmed());
}

void MainWindow::refreshMenus()
{
    const bool hasJobs = !m_downloadManager->downloadItems().isEmpty();
    const bool hasSelection = !m_downloadManager->selection().isEmpty();
    const bool hasOnlyOneSelected = m_downloadManager->selection().count() == 1;
    bool hasOnlyCompletedSelected = hasSelection;
    foreach (auto item, m_downloadManager->selection()) {
        if (item->state() != DownloadItem::Completed) {
            hasOnlyCompletedSelected = false;
            continue;
        }
    }
    bool hasAtLeastOneUncompletedSelected = false;
    foreach (auto item, m_downloadManager->selection()) {
        if (item->state() != DownloadItem::Completed) {
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
    ui->actionManageMirrors->setEnabled(hasAtLeastOneUncompletedSelected);
    ui->actionOneMoreSegment->setEnabled(hasAtLeastOneUncompletedSelected);
    ui->actionOneFewerSegment->setEnabled(hasAtLeastOneUncompletedSelected);
    //! [1]

    //! [2] View
    ui->actionInformation->setEnabled(hasOnlyOneSelected);
    // --
    ui->actionOpenFile->setEnabled(hasOnlyCompletedSelected);
    ui->actionRenameFile->setEnabled(hasOnlyOneSelected);
    ui->actionDeleteFile->setEnabled(hasOnlyCompletedSelected);
    ui->actionOpenDirectory->setEnabled(hasOnlyOneSelected);
    // --
    ui->actionRemoveCompletedDownloads->setEnabled(hasJobs);
    ui->actionRemoveAll1->setEnabled(hasJobs);
    ui->actionCleanGoneFiles->setEnabled(hasJobs);
    // --
    ui->actionRemoveDownloads->setEnabled(hasJobs);
    ui->actionRemoveSelected->setEnabled(hasJobs);
    ui->actionRemoveDuplicates->setEnabled(hasJobs);
    ui->actionRemoveAll2->setEnabled(hasJobs);
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

/******************************************************************************
 ******************************************************************************/
QString MainWindow::askSaveFileName(const QString &fileFilter, const QString &title)
{
    QString suggestedPath;
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    suggestedPath = dir + QDir::separator() + m_currentFile.fileName();
    suggestedPath = QDir::toNativeSeparators(suggestedPath);

    return QFileDialog::getSaveFileName(this, title, suggestedPath, fileFilter);
}

QString MainWindow::askOpenFileName(const QString &fileFilter, const QString &title)
{
    QString currentDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    currentDir = m_currentFile.absolutePath();

    return QFileDialog::getOpenFileName(this, title, currentDir, fileFilter);
}


/******************************************************************************
 ******************************************************************************/
void MainWindow::readSettings()
{        
    QSettings settings;
    if ( !isMaximized() ) {
        this->move(settings.value("Position", QPoint(100, 100)).toPoint());
        this->resize(settings.value("Size", QSize(350, 350)).toSize());
    }
    this->setWindowState( (Qt::WindowStates)settings.value("WindowState", 0).toInt() );

    m_settings->readSettings();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    if( !(isMaximized() | isFullScreen()) ) {
        settings.setValue("Position", this->pos());
        settings.setValue("Size", this->size());
    }
    settings.setValue("WindowState", (int)this->windowState()); // minimized, maximized, active, fullscreen...

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
inline QUrl MainWindow::urlFromClipboard() const
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    return QUrl(mimeData->hasText() ? mimeData->text() : QString());
}

/******************************************************************************
 ******************************************************************************/
bool MainWindow::saveFile(const QString &path)
{
    QDir::setCurrent(path);
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        QMessageBox::warning(this, tr("Cannot save file"),
                             tr("Cannot write to file %1:\n%2.")
                             .arg(path)
                             .arg(file.errorString()));
        return false;
    }

    QJsonObject json;
    // m_downloadManager->write(json);
    QJsonDocument saveDoc(json);
    file.write( saveDoc.toJson() );

    m_currentFile.setFile(path);
    this->statusBar()->showMessage(tr("File saved"), 2000);
    this->refreshTitleAndStatus();
    return true;
}

/******************************************************************************
 ******************************************************************************/
bool MainWindow::loadFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Couldn't open file.");
        QMessageBox::warning(this, tr("Error"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(path)
                             .arg(file.errorString()));
        return false;
    }
    QByteArray saveData = file.readAll();
    QJsonParseError ok;
    QJsonDocument loadDoc( QJsonDocument::fromJson(saveData, &ok) );

    if (ok.error != QJsonParseError::NoError) {
        qCritical("Couldn't parse JSON file.");
        QMessageBox::warning(this, tr("Error"),
                             tr("Cannot parse the JSON file:\n"
                                "%1\n\n"
                                "At character %2, %3.\n\n"
                                "Operation canceled.")
                             .arg(path)
                             .arg(ok.offset)
                             .arg(ok.errorString()));
        return false;
    }

    // m_downloadManager->read(loadDoc.object());
    m_currentFile = path;
    this->statusBar()->showMessage(tr("File loaded"), 5000);
    this->refreshTitleAndStatus();
    return true;
}
