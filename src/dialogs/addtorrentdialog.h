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

#ifndef DIALOGS_ADD_TORRENT_DIALOG_H
#define DIALOGS_ADD_TORRENT_DIALOG_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>

class AbstractJob;
class DownloadManager;
class Settings;

namespace Ui {
class AddTorrentDialog;
}

class AddTorrentDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddTorrentDialog(const QUrl &url, DownloadManager *downloadManager, Settings *settings, QWidget *parent = nullptr);
    ~AddTorrentDialog() override;

    static bool isTorrentUrl(const QUrl &url);

public slots:
    void accept() override;
    virtual void acceptPaused();
    void reject() override;

private slots:
    void onChanged(QString);

private:
    Ui::AddTorrentDialog *ui = nullptr;
    DownloadManager *m_downloadManager = nullptr;
    Settings *m_settings = nullptr;

    void doAccept(bool started);

    AbstractJob* createTorrentItem(const QString &url) const;
    static inline QList<AbstractJob*> toList(AbstractJob *item);

    void readUiSettings();
    void writeUiSettings();
};

#endif // DIALOGS_ADD_TORRENT_DIALOG_H
