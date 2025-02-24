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

#ifndef DIALOGS_ADD_STREAM_DIALOG_H
#define DIALOGS_ADD_STREAM_DIALOG_H

#include <Core/Stream>

#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>

class AbstractJob;
class DownloadManager;
class Settings;

namespace Ui {
class AddStreamDialog;
}

class AddStreamDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddStreamDialog(const QUrl &url, DownloadManager *downloadManager, Settings *settings, QWidget *parent = nullptr);
    ~AddStreamDialog() override;

    static bool isStreamUrl(const QUrl &url, const Settings *settings);

public slots:
    void accept() override;
    virtual void acceptPaused();
    void reject() override;

private slots:
    void onContinueClicked();
    void onChanged(QString);

    void onError(const QString &errorMessage);
    void onCollected(const QList<StreamObject> &streamObjects);

private:
    Ui::AddStreamDialog *ui = nullptr;
    DownloadManager *m_downloadManager = nullptr;
    StreamAssetDownloader *m_streamObjectDownloader = nullptr;
    Settings *m_settings = nullptr;

    void doAccept(bool started);

    QList<AbstractJob*> createStreamItems() const;
    AbstractJob* createStreamItem(const StreamObject &streamObject) const;

    void setGuiEnabled(bool enabled);

    void readUiSettings();
    void writeUiSettings();
};

#endif // DIALOGS_ADD_STREAM_DIALOG_H
