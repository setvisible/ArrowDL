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

#ifndef DIALOGS_ADD_DOWNLOAD_DIALOG_H
#define DIALOGS_ADD_DOWNLOAD_DIALOG_H

#include <QtWidgets/QDialog>

class JobManager;
class ResourceItem;

namespace Ui {
class AddDownloadDialog;
}

class AddDownloadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddDownloadDialog(const QUrl &url, JobManager *jobManager, QWidget *parent = 0);
    ~AddDownloadDialog();

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void acceptPaused();
    virtual void reject() Q_DECL_OVERRIDE;

private:
    Ui::AddDownloadDialog *ui;    
    JobManager *m_jobManager;
};

#endif // DIALOGS_ADD_DOWNLOAD_DIALOG_H
