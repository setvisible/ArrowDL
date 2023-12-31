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

#ifndef DIALOGS_ADD_URLS_DIALOG_H
#define DIALOGS_ADD_URLS_DIALOG_H

#include <QtWidgets/QDialog>

class IDownloadItem;
class DownloadManager;
class Settings;

class QLineEdit;

namespace Ui {
class AddUrlsDialog;
}

class AddUrlsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddUrlsDialog(const QString &text, DownloadManager *downloadManager, Settings *settings, QWidget *parent = nullptr);
    ~AddUrlsDialog() override;

public slots:
    void accept() override;
    virtual void acceptPaused();
    void reject() override;

private slots:
    void onChanged(QString);
    void onTextChanged();

private:
    Ui::AddUrlsDialog *ui = nullptr;
    QLineEdit *m_fakeUrlLineEdit = nullptr;
    DownloadManager *m_downloadManager = nullptr;
    Settings *m_settings = nullptr;

    void doAccept(bool started);

    IDownloadItem* createItem(const QString &url) const;
    static inline QList<IDownloadItem*> toList(IDownloadItem *item);

    void readUiSettings();
    void writeUiSettings();
};

#endif // DIALOGS_ADD_URLS_DIALOG_H
