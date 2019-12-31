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
#include <QtWidgets/QMessageBox>

class IDownloadItem;
class DownloadManager;
class Settings;

namespace Ui {
class AddDownloadDialog;
}

class AddDownloadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddDownloadDialog(const QUrl &url, DownloadManager *downloadManager,
                               Settings *settings, QWidget *parent = Q_NULLPTR);
    ~AddDownloadDialog() Q_DECL_OVERRIDE;

    static void quickDownload(const QUrl &url, DownloadManager *downloadManager);

public slots:
    void accept() Q_DECL_OVERRIDE;
    virtual void acceptPaused();
    void reject() Q_DECL_OVERRIDE;

private slots:
    void showContextMenu(const QPoint &pos);
    void insert_1_to_10();
    void insert_1_to_100();
    void insert_01_to_10();
    void insert_001_to_100();
    void insert_custom();
    void onChanged(QString);

private:
    Ui::AddDownloadDialog *ui;
    DownloadManager *m_downloadManager;
    Settings *m_settings;

    void doAccept(bool started);
    QMessageBox::StandardButton askBatchDownloading(QList<IDownloadItem*> items);

    QList<IDownloadItem*> createItems(const QUrl &inputUrl) const;
    IDownloadItem* createItem(const QString &url) const;
    static inline QList<IDownloadItem*> toList(IDownloadItem *item);

    void readSettings();
    void writeSettings();
};

#endif // DIALOGS_ADD_DOWNLOAD_DIALOG_H
