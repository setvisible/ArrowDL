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
#include <Core/JobClient>
#include <Core/JobManager>
#include <Core/Model>
#include <Core/ResourceItem>
#include <Core/ResourceModel>

#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtCore/QSettings>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>
#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

WizardDialog::WizardDialog(const QUrl &url, JobManager *jobManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizardDialog)
    , m_jobManager(jobManager)
    , m_model(new Model())
    , m_networkAccessManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    ui->linkWidget->setModel(m_model);
    connect(ui->browserWidget, SIGNAL(textChanged(QString)), m_model, SLOT(setMask(QString)));
    connect(ui->maskWidget,    SIGNAL(textChanged(QString)), m_model, SLOT(setMask(QString)));
    connect(ui->filterWidget,  SIGNAL(filterChanged(QString)), m_model, SLOT(onFilterChanged(QString)));

    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onFinished(QNetworkReply*)));

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
    if (m_jobManager) {
        m_jobManager->append(m_model->selection(), true);
    }
    writeSettings();
    QDialog::accept();
}

void WizardDialog::acceptPaused()
{
    if (m_jobManager) {
        m_jobManager->append(m_model->selection()); /* false */
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
    m_model->setDestination(ui->browserWidget->text());
    m_model->setMask(ui->maskWidget->text());
}

/******************************************************************************
 ******************************************************************************/
void WizardDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    resize(settings.value("DialogSize", QSize(800, 600)).toSize());
    ui->filterWidget->setState(settings.value("FilterState", 0).toUInt());
    ui->filterWidget->setText(settings.value("FilterText", "*.htm *.html").toString());
    settings.endGroup();
}

void WizardDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    settings.setValue("DialogSize", size());
    settings.setValue("FilterState", ui->filterWidget->state());
    settings.setValue("FilterText", ui->filterWidget->text());
    settings.endGroup();
}
