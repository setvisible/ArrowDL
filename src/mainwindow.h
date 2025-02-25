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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Core/AbstractJob>

#include <QtWidgets/QMainWindow>

class DownloadManager;
class StreamManager;
class FileAccessManager;
class Settings;
class UpdateChecker;
class SystemTray;

using DownloadRange = QList<AbstractJob *>;

QT_BEGIN_NAMESPACE
class QLabel;
class QMimeData;
QT_END_NAMESPACE

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
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    bool saveFile(const QString &path);
    bool loadFile(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

public slots:

    // File
    void importFromFile();
    void exportSelectedToFile();

    // Edit
    void copy();

    // View
    void showInformation();
    void openFile();
    void openFile(AbstractJob *downloadItem);
    void renameFile();
    void deleteFile();
    void openDirectory();
    void removeCompleted();
    void removeSelected();
    void removeAll();

    // Download
    void handleMessage(const QString &message);
    void home();
    void addContent();
    void addContent(const QUrl &url);
    void addContent(const QString &message);
    void addBatch();
    void addBatch(const QUrl &url);
    void addStream();
    void addStream(const QUrl &url);
    void addTorrent();
    void addTorrent(const QUrl &url);
    void addUrls();
    void addUrls(const QString &text);
    void resume();
    void cancel();
    void pause();

    // Options
    void showPreferences();

    // Help
    void onUpdateAvailableForConsole();
    void checkForUpdates();
    void showTutorial();
    void about();
    void aboutCompiler();
    void aboutStream();
    void aboutWebsite();

private slots:
    void onDataChanged();
    void onJobFinished(AbstractJob *downloadItem);
    void onJobRenamed(const QString &oldName, const QString &newName, bool success);
    void onSelectionChanged();
    void onTorrentContextChanged();

private:
    Ui::MainWindow *ui = nullptr;
    DownloadManager *m_downloadManager = nullptr;
    StreamManager *m_streamManager = nullptr;
    FileAccessManager *m_fileAccessManager = nullptr;
    Settings *m_settings = nullptr;
    QLabel *m_statusBarLabel = nullptr;
#ifdef USE_QT_WINEXTRAS
    QWinTaskbarButton *m_winTaskbarButton = nullptr;
    QWinTaskbarProgress *m_winTaskbarProgress = nullptr;
#endif
    UpdateChecker *m_updateChecker = nullptr;
    SystemTray *m_systemTray = nullptr;

    void readSettings();
    void writeSettings();

    void createActions();
    void createContextMenu();
    void createStatusbar();
    void createSystemTray();
    void propagateToolTips();
    void propagateIcons();

    void refreshTitleAndStatus();
    void refreshMenus();
    void refreshSplitter();

    inline bool askConfirmation(const QString &text);

    inline QUrl droppedUrl(const QMimeData* mimeData) const;
    inline QString fromClipboard() const;
    inline QUrl urlFromClipboard() const;

    inline void setWorkingDirectory(const QString &path);

    QString askSaveFileName(const QString &fileFilter, const QString &title = tr("Save As"));
    QString askOpenFileName(const QString &fileFilter, const QString &title = tr("Open"));
};

#endif // MAINWINDOW_H
