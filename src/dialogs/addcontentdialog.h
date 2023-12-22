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

#ifndef DIALOGS_ADD_CONTENT_DIALOG_H
#define DIALOGS_ADD_CONTENT_DIALOG_H

#include <QtCore/QUrl>
#include <QtWidgets/QDialog>


class Model;
class DownloadManager;
class Settings;

#ifdef USE_QT_WEBENGINE
class QWebEngineView;
#endif

namespace Ui {
class AddContentDialog;
}

class AddContentDialog : public QDialog
{
    Q_OBJECT

    enum Bypass { None, Start, StartPaused }; // Dirty hack to NOT show the dialog

public:
    explicit AddContentDialog(DownloadManager *downloadManager,
                              Settings *settings, QWidget *parent);
    ~AddContentDialog() override;

    bool loadResources(const QString &message);
    void loadUrl(const QUrl &url);

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
#ifdef USE_QT_WEBENGINE
    void htmlReceived(QString content);
#endif

public slots:
    int exec() override;
    void accept() override;
    virtual void acceptPaused();
    void reject() override;

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
    Ui::AddContentDialog *ui;
    DownloadManager *m_downloadManager;
    Model *m_model;
#ifdef USE_QT_WEBENGINE
    QWebEngineView *m_webEngineView;
#endif
    Settings *m_settings;
    QUrl m_url;
    Bypass m_bypass = None;

    void parseResources(const QString &message);
    void parseHtml(const QByteArray &downloadedData);
    void setProgressInfo(int percent, const QString &text = QString());
    void setNetworkError(const QString &errorString);

    void start(bool started);

    void readSettings();
    void writeSettings();
};

#endif // DIALOGS_ADD_CONTENT_DIALOG_H
