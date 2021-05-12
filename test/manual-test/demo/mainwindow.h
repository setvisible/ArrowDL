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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>

class FakeDownloadManager;
class IDownloadItem;
typedef QList<IDownloadItem*> DownloadRange;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow();

public slots:

    // File

    // Edit
    void selectAll();
    void selectNone();
    void invertSelection();
    void selectCompleted();
    void manageMirrors();
    void oneMoreSegment();
    void oneFewerSegment();

    // View
//    void showInformation();
//    void openFile();
//    void openFile(FakeDownloadItem *downloadItem);
//    void renameFile();
//    void deleteFile();
//    void openDirectory();
    void removeWaiting();
    void removeAll();
    void removeCompleted();
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
//    void showPreferences();

    // Help
//    void about();

private slots:
    void onJobAddedOrRemoved(DownloadRange downloadItem);
    void onJobStateChanged(IDownloadItem *downloadItem);
    void onSelectionChanged();

private:
    Ui::MainWindow *ui;
    FakeDownloadManager *m_downloadManager;

    void createActions();
    void createContextMenu();
    void propagateIcons();

    void refreshTitleAndStatus();
    void refreshMenus();
};

#endif // MAINWINDOW_H
