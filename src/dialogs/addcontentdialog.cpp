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

#include "addcontentdialog.h"
#include "ui_addcontentdialog.h"

#include <Constants>
#include <Core/HtmlParser>
#include <Core/JobFile>
#include <Core/Scheduler>
#include <Core/Model>
#include <Core/NetworkManager>
#include <Core/ResourceItem>
#include <Core/ResourceModel>
#include <Core/Settings>
#include <Core/Theme>
#include <Ipc/InterProcessCommunication>

#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QtMath>
#include <QtCore/QRegularExpression>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>

#ifdef USE_QT_WEBENGINE
#  include <QtWebEngineWidgets/QWebEngineView>
#  include <QtWebEngineWidgets/QWebEngineSettings>
#else
#  include <QtNetwork/QNetworkReply>
#endif

constexpr int default_width = 800;
constexpr int default_height = 600;
constexpr int column_download_width = 400;
constexpr int column_mask_width = 200;


static QList<AbstractJob*> createJobs(
    const QList<ResourceItem*> &resources,
    Scheduler *scheduler,
    const Settings *settings)
{
    QList<AbstractJob*> jobs;
    for (auto resource : resources) {
        if (settings && settings->isHttpReferringPageEnabled()) {
            resource->setReferringPage(settings->httpReferringPage());
        }
        auto job = new JobFile(scheduler, resource);
        jobs << job;
    }
    return jobs;
}


AddContentDialog::AddContentDialog(Scheduler *scheduler, Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddContentDialog)
    , m_scheduler(scheduler)
    , m_model(new Model(this))
    #ifdef USE_QT_WEBENGINE
    , m_webEngineView(nullptr)
    #endif
    , m_settings(settings)
{
    ui->setupUi(this);

    setWindowTitle(QString("%0 - %1").arg(STR_APPLICATION_NAME, tr("Web Page Content")));

    Theme::setIcons(this, { {ui->preferenceButton, "preference"} });

    ui->pathWidget->setPathType(PathWidget::Directory);
    ui->linkWidget->setModel(m_model);

    connect(m_settings, SIGNAL(changed()), this, SLOT(refreshFilters()));

    connect(ui->pathWidget, SIGNAL(currentPathChanged(QString)), m_model, SLOT(setDestination(QString)));
    connect(ui->pathWidget, SIGNAL(currentPathChanged(QString)), this, SLOT(onChanged(QString)));

    connect(ui->maskWidget, SIGNAL(currentMaskChanged(QString)), m_model, SLOT(setMask(QString)));
    connect(ui->maskWidget, SIGNAL(currentMaskChanged(QString)), this, SLOT(onChanged(QString)));

    connect(ui->filterWidget, SIGNAL(regexChanged(QRegularExpression)), m_model, SLOT(select(QRegularExpression)));

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
    if (m_scheduler) {
        auto jobs = createJobs(m_model->selection(), m_scheduler, m_settings);
        m_scheduler->append(jobs, started);
    }
}

/******************************************************************************
 ******************************************************************************/
bool AddContentDialog::loadResources(const QString &message)
{
    parseResources(message);
    return m_bypass == None;
}

/******************************************************************************
 ******************************************************************************/
void AddContentDialog::loadUrl(const QUrl &url)
{
    if (!url.isValid()) {
        QMessageBox::warning(this, tr("Warning"),
                             QString("%0\n\n%1").arg(
                                 tr("Error: The url is not valid:"),
                                 url.toString()));
    } else {
        m_url = url;

#ifdef USE_QT_WEBENGINE
        qInfo("Loading URL. HTML parser is Chromium.");
        if (!m_webEngineView) {
            m_webEngineView = new QWebEngineView(this);

            connect(m_webEngineView, SIGNAL(loadProgress(int)), SLOT(onLoadProgress(int)));
            connect(m_webEngineView, SIGNAL(loadFinished(bool)), SLOT(onLoadFinished(bool)));

            /* Only load source, not media */
            QWebEngineSettings *settings =  m_webEngineView->settings()->globalSettings();
            settings->setAttribute(QWebEngineSettings::AutoLoadImages, false);
            settings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
            m_webEngineView->page()->setAudioMuted(true);
            settings->setAttribute(QWebEngineSettings::ShowScrollBars, false);
            settings->setAttribute(QWebEngineSettings::PdfViewerEnabled, false);
        }
        m_webEngineView->load(m_url);
#else
        qInfo("Loading URL. HTML parser is Google Gumbo.");
        NetworkManager *networkManager = m_scheduler->networkManager();
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
    /* Between 0% and 90% */
    progress = qMin(qCeil(0.90 * qreal(progress)), 90);
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
        percent = qMin(qCeil(90 * static_cast<qreal>(bytesReceived) / static_cast<qreal>(bytesTotal)), 90);
    }
    setProgressInfo(percent, tr("Downloading..."));
}

void AddContentDialog::onFinished()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        if (reply->error() == QNetworkReply::NoError) {
            auto downloadedData = reply->readAll();
            reply->deleteLater();
            parseHtml(downloadedData);
        } else {
            setNetworkError(reply->errorString());
        }
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

    const QString message = QString("%0\n\n%1\n\n%2").arg(
                tr("The wizard can't connect to URL:"),
                elidedUrl,
                errorString);

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
    auto currentModel = m_model->currentModel();
    auto selectionCount = currentModel->selection().count();
    if (selectionCount == 0) {
        ui->tipLabel->setText(tr("After selecting links, click on Start!"));
    } else {
        auto count = currentModel->items().count();
        ui->tipLabel->setText(tr("Selected links: %0 of %1").arg(
                                  QString::number(selectionCount),
                                  QString::number(count)));
    }
    onChanged({});
}

/******************************************************************************
 ******************************************************************************/
void AddContentDialog::onChanged(const QString &value)
{
    Q_UNUSED(value)
    auto currentModel = m_model->currentModel();
    auto selectionCount = currentModel->selection().count();
    auto enabled =
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
    for (auto filter : filters) {
        ui->filterWidget->addFilter(filter.name(), filter.regex());
    }
}

/******************************************************************************
 ******************************************************************************/
void AddContentDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    resize(settings.value("DialogSize", QSize(default_width, default_height)).toSize());
    ui->filterWidget->setState(settings.value("FilterState", 0).toUInt());
    ui->filterWidget->setCurrentFilter(settings.value("Filter", QString()).toString());
    ui->filterWidget->setFilterHistory(settings.value("FilterHistory", QString()).toStringList());

    QList<int> defaultWidths = {-1, column_download_width, -1, -1, column_mask_width};
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
