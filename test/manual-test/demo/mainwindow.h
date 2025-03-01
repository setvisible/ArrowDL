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

class FakeScheduler;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

    // File
    // Edit
    // View
    void removeCompleted();
    void removeSelected();
    void removeAll();

    // Download
    void add();
    void resume();
    void cancel();
    void pause();

    // Options
    // Help

private slots:
    void onJobFinished(AbstractJob *job);
    void onDataChanged();
    void onSelectionChanged();

private:
    Ui::MainWindow *ui;
    FakeScheduler *m_scheduler;

    void createActions();
    void createContextMenu();
    void propagateIcons();

    void refreshTitleAndStatus();
    void refreshMenus();
};

#endif // MAINWINDOW_H
