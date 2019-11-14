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

#include <QtCore/QUrl>
#include <QtWidgets/QDialog>


class Model;
class DownloadManager;
class Settings;

#ifdef USE_QT_WEBENGINE
class QWebEngineView;
#else
class QNetworkAccessManager;
#endif

namespace Ui {
class WizardDialog;
}

class WizardDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizardDialog(const QUrl &url, DownloadManager *downloadManager,
                          Settings *settings, QWidget *parent);
    ~WizardDialog();

protected:
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

signals:
#ifdef USE_QT_WEBENGINE
    void htmlReceived(QString content);
#endif

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void acceptPaused();
    virtual void reject() Q_DECL_OVERRIDE;

private slots:
#ifdef USE_QT_WEBENGINE
    void onLoadProgress(int progress);
    void onLoadFinished(bool finished);
    void onHtmlReceived(QString content);
#else
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onFinished();
#endif
    void onSelectionChanged();
    void onChanged(QString);
    void refreshFilters();

private:
    Ui::WizardDialog *ui;
    DownloadManager *m_downloadManager;
    Model *m_model;
#ifdef USE_QT_WEBENGINE
    QWebEngineView *m_webEngineView;
#else
    QNetworkAccessManager *m_networkAccessManager;
#endif
    Settings *m_settings;
    QUrl m_url;

    void loadUrl(const QUrl &url);
    void parseHtml(const QByteArray &downloadedData);
    void setProgressInfo(int percent, const QString &text = QString());
    void setNetworkError(const QString &errorString);

    void readSettings();
    void writeSettings();
};

#endif // DIALOGS_SELECTIONDIALOG_H
