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

#include "wizarddialog.h"
#include "ui_wizarddialog.h"

#include <Core/HtmlParser>
#include <Core/DownloadItem>
#include <Core/DownloadManager>
#include <Core/Model>
#include <Core/ResourceItem>
#include <Core/ResourceModel>
#include <Core/Settings>

#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtCore/QSettings>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>
#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

WizardDialog::WizardDialog(const QUrl &url, DownloadManager *downloadManager,
                           Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizardDialog)
    , m_downloadManager(downloadManager)
    , m_model(new Model(this))
    , m_networkAccessManager(new QNetworkAccessManager(this))
    , m_settings(settings)
{
    ui->setupUi(this);

    ui->pathWidget->setPathType(PathWidget::Directory);
    ui->linkWidget->setModel(m_model);

    connect(m_settings, SIGNAL(changed()), this, SLOT(refreshFilters()));

    connect(ui->pathWidget, SIGNAL(currentPathChanged(QString)), m_model, SLOT(setDestination(QString)));
    connect(ui->pathWidget, SIGNAL(currentPathChanged(QString)), this, SLOT(onChanged(QString)));

    connect(ui->maskWidget, SIGNAL(currentMaskChanged(QString)), m_model, SLOT(setMask(QString)));
    connect(ui->maskWidget, SIGNAL(currentMaskChanged(QString)), this, SLOT(onChanged(QString)));

    connect(ui->filterWidget, SIGNAL(regexChanged(QRegExp)), m_model, SLOT(select(QRegExp)));

    connect(m_model, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));

    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onFinished(QNetworkReply*)));

    refreshFilters();

    readSettings();
    loadUrl(url);
}

WizardDialog::~WizardDialog()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void WizardDialog::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void WizardDialog::accept()
{
    if (m_downloadManager) {
        m_downloadManager->append(m_model->selection(), true);
    }
    writeSettings();
    QDialog::accept();
}

void WizardDialog::acceptPaused()
{
    if (m_downloadManager) {
        m_downloadManager->append(m_model->selection()); /* false */
    }
    writeSettings();
    QDialog::accept();
}

void WizardDialog::reject()
{
    writeSettings();
    QDialog::reject();
}

/******************************************************************************
 ******************************************************************************/
void WizardDialog::loadUrl(const QUrl &url)
{
    if (!url.isValid()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Error: The url is not valid:\n\n%0").arg(url.toString()));
    } else {
        m_url = url;
        m_networkAccessManager->get(QNetworkRequest(m_url));
    }
}

void WizardDialog::onFinished(QNetworkReply *reply)
{
    QByteArray downloadedData = reply->readAll();
    reply->deleteLater();

    m_model->linkModel()->clear();
    m_model->contentModel()->clear();

    HtmlParser htmlParser;
    htmlParser.parse(downloadedData, m_url, m_model);

    // Force update
    m_model->setDestination(ui->pathWidget->currentPath());
    m_model->setMask(ui->maskWidget->currentMask());
    m_model->select(ui->filterWidget->regex());

    onSelectionChanged();
}

/******************************************************************************
 ******************************************************************************/
void WizardDialog::onSelectionChanged()
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
void WizardDialog::onChanged(QString)
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
void WizardDialog::refreshFilters()
{
    QList<Filter> filters = m_settings->filters();
    ui->filterWidget->clearFilters();
    foreach (auto filter, filters) {
        ui->filterWidget->addFilter(filter.title, filter.regexp);
    }
}

/******************************************************************************
 ******************************************************************************/
void WizardDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    resize(settings.value("DialogSize", QSize(800, 600)).toSize());
    ui->filterWidget->setState(settings.value("FilterState", 0).toUInt());
    ui->filterWidget->setCurrentFilter(settings.value("Filter", QString()).toString());
    ui->filterWidget->setFilterHistory(settings.value("FilterHistory", QString()).toStringList());
    ui->linkWidget->setColumnWidths(settings.value("ColumnWidths").value<QList<int> >());
    ui->pathWidget->setCurrentPath(settings.value("Path", QString()).toString());
    ui->pathWidget->setPathHistory(settings.value("PathHistory").toStringList());
    ui->maskWidget->setCurrentMask(settings.value("Mask", QString()).toString());
    settings.endGroup();
}

void WizardDialog::writeSettings()
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
