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

#ifndef DIALOGS_SELECTIONDIALOG_H
#define DIALOGS_SELECTIONDIALOG_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QDialog>


class Model;
class DownloadManager;

namespace Ui {
class WizardDialog;
}

class WizardDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizardDialog(const QUrl &url, DownloadManager *downloadManager, QWidget *parent);
    ~WizardDialog();

protected:
    virtual void closeEvent(QCloseEvent *event);

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void acceptPaused();
    virtual void reject() Q_DECL_OVERRIDE;

private slots:
    void onFinished(QNetworkReply* reply);
    void onSelectionChanged();

private:
    Ui::WizardDialog *ui;
    DownloadManager *m_downloadManager;
    Model *m_model;
    QNetworkAccessManager *m_networkAccessManager;
    QUrl m_url;

    void loadUrl(const QUrl &url);

    void readSettings();
    void writeSettings();
};

#endif // DIALOGS_SELECTIONDIALOG_H
