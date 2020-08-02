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

#include "addcontentdialog.h"
#include "ui_addcontentdialog.h"

#include <Globals>
#include <Core/HtmlParser>
#include <Core/DownloadItem>
#include <Core/DownloadManager>
#include <Core/Model>
#include <Core/NetworkManager>
#include <Core/ResourceItem>
#include <Core/ResourceModel>
#include <Core/Settings>
#include <Ipc/InterProcessCommunication>

#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QtMath>
#include <QtCore/QUrl>
#include <QtCore/QSettings>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>

#ifdef USE_QT_WEBENGINE
#  include <QtWebEngineWidgets/QWebEngineView>
#  include <QtWebEngineWidgets/QWebEngineSettings>
#else
#  include <QtNetwork/QNetworkReply>
#endif

#define C_DEFAULT_WIDTH         800
#define C_DEFAULT_HEIGHT        600
#define C_DEFAULT_INDEX         0

#define C_COLUMN_DOWNLOAD_WIDTH     400
#define C_COLUMN_MASK_WIDTH         200


static QList<IDownloadItem*> createItems(QList<ResourceItem*> resources, DownloadManager *downloadManager)
{
    QList<IDownloadItem*> items;
    foreach (auto resource, resources) {
        auto item = new DownloadItem(downloadManager);
        item->setResource(resource);
        items << item;
    }
    return items;
}


AddContentDialog::AddContentDialog(DownloadManager *downloadManager,
                                   Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddContentDialog)
    , m_downloadManager(downloadManager)
    , m_model(new Model(this))
    #ifdef USE_QT_WEBENGINE
    , m_webEngineView(Q_NULLPTR)
    #endif
    , m_settings(settings)
{
    ui->setupUi(this);

    setWindowTitle(QString("%0 - %1").arg(STR_APPLICATION_NAME).arg(tr("Web Page Content")));

    ui->pathWidget->setPathType(PathWidget::Directory);
    ui->linkWidget->setModel(m_model);

    connect(m_settings, SIGNAL(changed()), this, SLOT(refreshFilters()));

    connect(ui->pathWidget, SIGNAL(currentPathChanged(QString)), m_model, SLOT(setDestination(QString)));
    connect(ui->pathWidget, SIGNAL(currentPathChanged(QString)), this, SLOT(onChanged(QString)));

    connect(ui->maskWidget, SIGNAL(currentMaskChanged(QString)), m_model, SLOT(setMask(QString)));
    connect(ui->maskWidget, SIGNAL(currentMaskChanged(QString)), this, SLOT(onChanged(QString)));

    connect(ui->filterWidget, SIGNAL(regexChanged(QRegExp)), m_model, SLOT(select(QRegExp)));

    connect(m_model, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));

    refreshFilters();

    readSettings();
}

AddContentDialog::~AddContentDialog()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void AddContentDialog::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

int AddContentDialog::exec()
{
    switch (m_bypass) {
    case None:
        return QDialog::exec();

    case Start:
        start(true);
        break;

    case StartPaused:
        start(false);
        break;
    }
    return DialogCode::Accepted;
}

void AddContentDialog::accept()
{
    start(true);
    writeSettings();
    QDialog::accept();
}

void AddContentDialog::acceptPaused()
{
    start(false);
    writeSettings();
    QDialog::accept();
}

void AddContentDialog::reject()
{
    writeSettings();
    QDialog::reject();
}

/******************************************************************************
 ******************************************************************************/
void AddContentDialog::start(bool started)
{
    if (m_downloadManager) {
        QList<IDownloadItem*> items = createItems(m_model->selection(), m_downloadManager);
        m_downloadManager->append(items, started);
    }
}

/******************************************************************************
 ******************************************************************************/
void AddContentDialog::loadResources(const QString &message)
{
    parseResources(message);
}

/******************************************************************************
 ******************************************************************************/
void AddContentDialog::loadUrl(const QUrl &url)
{
    if (!url.isValid()) {
        QMessageBox::warning(this, tr("Warning"),
                             QString("%0\n\n%1")
                             .arg(tr("Error: The url is not valid:"))
                             .arg(url.toString()));
    } else {
        m_url = url;

#ifdef USE_QT_WEBENGINE
        qDebug() << Q_FUNC_INFO << "GOOGLE GUMBO + QT WEB ENGINE";
        if (!m_webEngineView) {
            m_webEngineView = new QWebEngineView(this);

            connect(m_webEngineView, SIGNAL(loadProgress(int)), SLOT(onLoadProgress(int)));
            connect(m_webEngineView, SIGNAL(loadFinished(bool)), SLOT(onLoadFinished(bool)));

            /* Only load source, not media */
            QWebEngineSettings *settings =  m_webEngineView->settings()->globalSettings();
            settings->setAttribute(QWebEngineSettings::AutoLoadImages, false);
#if QT_VERSION >= 0x050700
            settings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
            m_webEngineView->page()->setAudioMuted(true);
#endif
#if QT_VERSION >= 0x051000
            settings->setAttribute(QWebEngineSettings::ShowScrollBars, false);
#endif
#if QT_VERSION >= 0x051300
            settings->setAttribute(QWebEngineSettings::PdfViewerEnabled, false);
#endif
        }
        m_webEngineView->load(m_url);
#else
        qDebug() << Q_FUNC_INFO << "GOOGLE GUMBO";
        NetworkManager *networkManager = m_downloadManager->networkManager();
        QNetworkReply *reply = networkManager->get(m_url);
        connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress(qint64,qint64)));
        connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
#endif
        setProgressInfo(0, tr("Connecting..."));
    }
}

/******************************************************************************
 ******************************************************************************/
#ifdef USE_QT_WEBENGINE
void AddContentDialog::onLoadProgress(int progress)
{
    /* Between 1% and 90% */
    progress = qMin(qFloor(0.90 * progress), 90);
    setProgressInfo(progress, tr("Downloading..."));
}

void AddContentDialog::onLoadFinished(bool finished)
{
    if (finished) {
        /*
         * Hack to retrieve the HTML page content from QWebEnginePage
         * and send it to the Gumbo HTML5 Parser.
         */
        connect(this, SIGNAL(htmlReceived(QString)), this, SLOT(onHtmlReceived(QString)));
        m_webEngineView->page()->toHtml([this](const QString &result) mutable
        {
            emit htmlReceived(result);
        });
        m_webEngineView->setVisible(false);

    } else {
        setNetworkError("");
    }
}

void AddContentDialog::onHtmlReceived(QString content)
{
    QByteArray downloadedData = content.toUtf8();
    parseHtml(downloadedData);
}
#else
void AddContentDialog::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    /* Between 1% and 90% */
    int percent = 1;
    if (bytesTotal > 0) {
        percent = qMin(qFloor(90.0 * bytesReceived / bytesTotal), 90);
    }
    setProgressInfo(percent, tr("Downloading..."));
}

void AddContentDialog::onFinished()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    if (reply && reply->error() == QNetworkReply::NoError) {
        QByteArray downloadedData = reply->readAll();
        reply->deleteLater();
        parseHtml(downloadedData);
    } else {
        setNetworkError(reply->errorString());
    }
}
#endif

/******************************************************************************
 ******************************************************************************/
void AddContentDialog::parseResources(const QString &message)
{
    setProgressInfo(10, tr("Collecting links..."));

    m_model->linkModel()->clear();
    m_model->contentModel()->clear();

    InterProcessCommunication::Options options;
    InterProcessCommunication::parseMessage(message, m_model, &options);

    setProgressInfo(99, tr("Finished"));

    // Force update
    m_model->setDestination(ui->pathWidget->currentPath());
    m_model->setMask(ui->maskWidget->currentMask());
    m_model->select(ui->filterWidget->regex());

    onSelectionChanged();

    setProgressInfo(100);

    if (options.testFlag(InterProcessCommunication::NoOptions)) {
        m_bypass = None;
    } else {
        if (options.testFlag(InterProcessCommunication::QuickLinks)) {
            m_model->setCurrentTab(Model::LINK);
        } else if (options.testFlag(InterProcessCommunication::QuickMedia)) {
            m_model->setCurrentTab(Model::CONTENT);
        }

        if (options.testFlag(InterProcessCommunication::StartPaused)) {
            m_bypass = StartPaused;
        } else {
            m_bypass = Start;
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void AddContentDialog::parseHtml(const QByteArray &downloadedData)
{
    setProgressInfo(90, tr("Collecting links..."));

    m_model->linkModel()->clear();
    m_model->contentModel()->clear();

    qDebug() << m_url;
    qDebug() << "---------------------";
    qDebug() << downloadedData;
    qDebug() << "---------------------";

    HtmlParser::parse(downloadedData, m_url, m_model);

    setProgressInfo(99, tr("Finished"));

    // Force update
    m_model->setDestination(ui->pathWidget->currentPath());
    m_model->setMask(ui->maskWidget->currentMask());
    m_model->select(ui->filterWidget->regex());

    onSelectionChanged();

    setProgressInfo(100);
}

void AddContentDialog::setNetworkError(const QString &errorString)
{
    const QFontMetrics fontMetrics = this->fontMetrics();
    const QString elidedUrl =
            fontMetrics.elidedText(m_url.toString(), Qt::ElideRight,
                                   ui->progressPage->width() - 200);

    const QString message =
            QString("%0\n\n%1\n\n%2")
            .arg(tr("The wizard can't connect to URL:"))
            .arg(elidedUrl)
            .arg(errorString);

    setProgressInfo(-1, message);
}

void AddContentDialog::setProgressInfo(int percent, const QString &text)
{
    if (percent < 0) {
        ui->stackedWidget->setCurrentIndex(1);
        ui->progressBar->setValue(0);
        ui->progressBar->setVisible(false);
        ui->progressLabel->setText(text);

    } else if (percent >= 0 && percent < 100) {
        ui->stackedWidget->setCurrentIndex(1);
        ui->progressBar->setValue(percent);
        ui->progressBar->setVisible(true);
        ui->progressLabel->setText(text);

    } else { // percent >= 100
        ui->stackedWidget->setCurrentIndex(0);
    }
}

/******************************************************************************
 ******************************************************************************/
void AddContentDialog::onSelectionChanged()
{
    const ResourceModel *currentModel = m_model->currentModel();
    const int selectionCount = currentModel->selectedResourceItems().count();
    if (selectionCount == 0) {
        ui->tipLabel->setText(tr("After selecting links, click on Start!"));
    } else {
        const int count = currentModel->resourceItems().count();
        ui->tipLabel->setText(tr("Selected links: %0 of %1").arg(selectionCount).arg(count));
    }
    onChanged(QString());
}

/******************************************************************************
 ******************************************************************************/
void AddContentDialog::onChanged(QString)
{
    const ResourceModel *currentModel = m_model->currentModel();
    const int selectionCount = currentModel->selectedResourceItems().count();
    const bool enabled =
            !ui->pathWidget->currentPath().isEmpty() &&
            !ui->maskWidget->currentMask().isEmpty() &&
            selectionCount > 0;
    ui->startButton->setEnabled(enabled);
    ui->addPausedButton->setEnabled(enabled);
}

/******************************************************************************
 ******************************************************************************/
void AddContentDialog::refreshFilters()
{
    QList<Filter> filters = m_settings->filters();
    ui->filterWidget->clearFilters();
    foreach (auto filter, filters) {
        ui->filterWidget->addFilter(filter.name(), filter.regex());
    }
}

/******************************************************************************
 ******************************************************************************/
void AddContentDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    resize(settings.value("DialogSize", QSize(C_DEFAULT_WIDTH, C_DEFAULT_HEIGHT)).toSize());
    ui->filterWidget->setState(settings.value("FilterState", 0).toUInt());
    ui->filterWidget->setCurrentFilter(settings.value("Filter", QString()).toString());
    ui->filterWidget->setFilterHistory(settings.value("FilterHistory", QString()).toStringList());

    QList<int> defaultWidths = {-1, C_COLUMN_DOWNLOAD_WIDTH, -1, -1, C_COLUMN_MASK_WIDTH};
    QVariant variant = QVariant::fromValue(defaultWidths);
    ui->linkWidget->setColumnWidths(settings.value("ColumnWidths", variant).value<QList<int> >());

    ui->pathWidget->setCurrentPath(settings.value("Path", QString()).toString());
    ui->pathWidget->setPathHistory(settings.value("PathHistory").toStringList());
    ui->maskWidget->setCurrentMask(settings.value("Mask", QString()).toString());
    settings.endGroup();
}

void AddContentDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    settings.setValue("DialogSize", size());
    settings.setValue("FilterState", ui->filterWidget->state());
    settings.setValue("Filter", ui->filterWidget->currentFilter());
    settings.setValue("FilterHistory", ui->filterWidget->filterHistory());
    settings.setValue("ColumnWidths", QVariant::fromValue(ui->linkWidget->columnWidths()));
    settings.setValue("Path", ui->pathWidget->currentPath());
    settings.setValue("PathHistory", ui->pathWidget->pathHistory());
    settings.setValue("Mask", ui->maskWidget->currentMask());
    settings.endGroup();
}
