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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>

class IDownloadItem;
class DownloadManager;
class FileAccessManager;
class Settings;
class UpdateChecker;

typedef QList<IDownloadItem*> DownloadRange;

class QLabel;

#ifdef USE_QT_WINEXTRAS
class QWinTaskbarButton;
class QWinTaskbarProgress;
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow() Q_DECL_OVERRIDE;

    bool saveFile(const QString &path);
    bool loadFile(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

public slots:

    // File
    void openWizard();
    void openWizard(const QUrl &url);
    void openWizard(const QString &message);
    void handleMessage(const QString &message);

    void importFromFile();
    void exportSelectedToFile();

    // Edit
    void selectAll();
    void selectNone();
    void invertSelection();
    void selectCompleted();
    void copy();
    void manageMirrors();
    void oneMoreSegment();
    void oneFewerSegment();

    // View
    void showInformation();
    void openFile();
    void openFile(IDownloadItem *downloadItem);
    void renameFile();
    void deleteFile();
    void openDirectory();
    void cleanGoneFiles();
    void removeAll();
    void removeCompletedDownloads();
    void removeDownloads();
    void removeSelected();
    void removeDuplicates();
    void removeFailed();
    void removePaused();

    // Download
    void add();
    void resume();
    void cancel();
    void pause();
    void up();
    void top();
    void down();
    void bottom();

    // Options
    void speedLimit();
    void addDomainSpecificLimit();
    void forceStart();
    void showPreferences();

    // Help
    void checkForUpdates();
    void showTutorial();
    void about();
    void aboutCompiler();

private slots:
    void onJobAddedOrRemoved(DownloadRange range);
    void onJobStateChanged(IDownloadItem *downloadItem);
    void onJobFinished(IDownloadItem *downloadItem);
    void onJobRenamed(QString oldName, QString newName, bool success);
    void onSelectionChanged();

private:
    Ui::MainWindow *ui;
    DownloadManager *m_downloadManager;
    FileAccessManager *m_fileAccessManager;
    Settings *m_settings;
    QLabel *m_statusBarLabel;
#ifdef USE_QT_WINEXTRAS
    QWinTaskbarButton *m_winTaskbarButton = Q_NULLPTR;
    QWinTaskbarProgress *m_winTaskbarProgress = Q_NULLPTR;
#endif
    UpdateChecker *m_updateChecker;

    void readSettings();
    void writeSettings();

    void createActions();
    void createContextMenu();
    void createStatusbar();

    void refreshTitleAndStatus();
    void refreshMenus();

    inline bool askConfirmation(const QString &text);

    inline QUrl urlFromClipboard() const;

    inline void setWorkingDirectory(const QString &path);

    QString askSaveFileName(const QString &fileFilter, const QString &title = tr("Save As"));
    QString askOpenFileName(const QString &fileFilter, const QString &title = tr("Open"));
};

#endif // MAINWINDOW_H
