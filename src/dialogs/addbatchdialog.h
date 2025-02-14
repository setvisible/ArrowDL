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

#ifndef DIALOGS_ADD_BATCH_DIALOG_H
#define DIALOGS_ADD_BATCH_DIALOG_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>

class AbstractDownloadItem;
class DownloadManager;
class Settings;

namespace Ui {
class AddBatchDialog;
}

class AddBatchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddBatchDialog(const QUrl &url, DownloadManager *downloadManager, Settings *settings, QWidget *parent = nullptr);
    ~AddBatchDialog() override;

    static void quickDownload(const QUrl &url, DownloadManager *downloadManager);

public slots:
    void accept() override;
    virtual void acceptPaused();
    void reject() override;

private slots:
    void showContextMenu(const QPoint &pos);
    void insert_1_to_10();
    void insert_1_to_100();
    void insert_01_to_10();
    void insert_001_to_100();
    void insert_custom();
    void onChanged(QString);

private:
    Ui::AddBatchDialog *ui = nullptr;
    DownloadManager *m_downloadManager = nullptr;
    Settings *m_settings = nullptr;

    void doAccept(bool started);
    QMessageBox::StandardButton askBatchDownloading(QList<AbstractDownloadItem*> items);

    QList<AbstractDownloadItem*> createFileItems(const QUrl &inputUrl) const;
    AbstractDownloadItem* createFileItem(const QString &url) const;
    static inline QList<AbstractDownloadItem*> toList(AbstractDownloadItem *item);

    inline QString insertName(const QString &name) const;

    void readUiSettings();
    void writeUiSettings();
};

#endif // DIALOGS_ADD_BATCH_DIALOG_H
