/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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

#ifndef DIALOGS_ADD_STREAM_DIALOG_H
#define DIALOGS_ADD_STREAM_DIALOG_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>

class IDownloadItem;
class DownloadManager;
class Settings;

class StreamInfos;
class StreamInfoDownloader;

typedef QSharedPointer<StreamInfos> StreamInfosPtr;

namespace Ui {
class AddStreamDialog;
}

class AddStreamDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddStreamDialog(const QUrl &url, DownloadManager *downloadManager,
                             Settings *settings, QWidget *parent = Q_NULLPTR);
    ~AddStreamDialog() Q_DECL_OVERRIDE;

public slots:
    void accept() Q_DECL_OVERRIDE;
    virtual void acceptPaused();
    void reject() Q_DECL_OVERRIDE;

private slots:
    void onContinueClicked();
    void onChanged(QString);

    void onError(QString errorMessage);
    void onCollected(StreamInfosPtr infos);

private:
    Ui::AddStreamDialog *ui;
    DownloadManager *m_downloadManager;
    StreamInfoDownloader *m_streamInfoDownloader;
    Settings *m_settings;

    void doAccept(bool started);

    IDownloadItem* createItem() const;
    static inline QList<IDownloadItem*> toList(IDownloadItem *item);

    void setGuiEnabled(bool enabled);

    void readSettings();
    void writeSettings();
};

#endif // DIALOGS_ADD_STREAM_DIALOG_H
