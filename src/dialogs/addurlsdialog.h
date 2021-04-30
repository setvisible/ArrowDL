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
    explicit AddUrlsDialog(const QString &text, DownloadManager *downloadManager,
                           Settings *settings, QWidget *parent = Q_NULLPTR);
    ~AddUrlsDialog() Q_DECL_OVERRIDE;

public slots:
    void accept() Q_DECL_OVERRIDE;
    virtual void acceptPaused();
    void reject() Q_DECL_OVERRIDE;

private slots:
    void onChanged(QString);
    void onTextChanged();

private:
    Ui::AddUrlsDialog *ui;
    QLineEdit *m_fakeUrlLineEdit;
    DownloadManager *m_downloadManager;
    Settings *m_settings;

    void doAccept(bool started);

    IDownloadItem* createItem(const QString &url) const;
    static inline QList<IDownloadItem*> toList(IDownloadItem *item);

    void readSettings();
    void writeSettings();
};

#endif // DIALOGS_ADD_URLS_DIALOG_H
