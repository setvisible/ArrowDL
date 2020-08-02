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

#include "addbatchdialog.h"
#include "ui_addbatchdialog.h"

#include <Globals>
#include <Core/DownloadItem>
#include <Core/DownloadManager>
#include <Core/Mask>
#include <Core/Regex>
#include <Core/ResourceItem>
#include <Core/Settings>
#include <Widgets/UrlFormWidget>

#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>


AddBatchDialog::AddBatchDialog(const QUrl &url, DownloadManager *downloadManager,
                               Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddBatchDialog)
    , m_downloadManager(downloadManager)
    , m_settings(settings)
{
    ui->setupUi(this);

    setWindowTitle(QString("%0 - %1").arg(STR_APPLICATION_NAME)
                   .arg(tr("Add Batch and Single File")));

    adjustSize();
    setFixedHeight(height());

    ui->urlFormWidget->setExternalUrlLabelAndLineEdit(ui->urlLabel, ui->urlLineEdit);

    ui->urlLineEdit->setText(url.toString());
    ui->urlLineEdit->setFocus();
    ui->urlLineEdit->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->titleListLabel->setToolTip(
                QString("<html><head/><body><p>"
                        "%0"
                        "</p><p>- "
                        "%1"
                        "</p><p>- "
                        "%2"
                        "</p><p>- "
                        "%3"
                        "</p></body></html>")
                .arg(tr("Batch descriptors:"))
                .arg(tr("Must start with '[' or '('"))
                .arg(tr("Must contain two numbers, separated by ':', '-' or a space character"))
                .arg(tr("Must end with ']' or ')'")));

    if (m_settings && m_settings->isCustomBatchEnabled()) {
        ui->tagButton_Custom->setText(m_settings->customBatchButtonLabel());
        ui->tagButton_Custom->setToolTip(m_settings->customBatchRange());
    } else {
        ui->tagButton_Custom->setVisible(false);
    }

    connect(ui->urlLineEdit, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));

    connect(ui->tagButton_1_10, SIGNAL(released()), this, SLOT(insert_1_to_10()));
    connect(ui->tagButton_1_100, SIGNAL(released()), this, SLOT(insert_1_to_100()));
    connect(ui->tagButton_01_10, SIGNAL(released()), this, SLOT(insert_01_to_10()));
    connect(ui->tagButton_001_100, SIGNAL(released()), this, SLOT(insert_001_to_100()));
    connect(ui->tagButton_Custom, SIGNAL(released()), this, SLOT(insert_custom()));

    connect(ui->urlLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onChanged(QString)));
    connect(ui->urlFormWidget, SIGNAL(changed(QString)), this, SLOT(onChanged(QString)));

    readSettings();
}

AddBatchDialog::~AddBatchDialog()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Immediate download of the url. The Dialog GUI is not displayed.
 */
void AddBatchDialog::quickDownload(const QUrl &url, DownloadManager *downloadManager)
{
    if (downloadManager == Q_NULLPTR) {
        return;
    }
    // Remove trailing / and \ and .
    const QString adjusted = url.adjusted(QUrl::StripTrailingSlash).toString();

    QSettings settings;
    settings.beginGroup("Wizard");
    const QString destinationPath = settings.value("Path", QString()).toString();
    const QString mask = settings.value("Mask", QString()).toString();
    settings.endGroup();

    auto resource = new ResourceItem();
    resource->setUrl(adjusted);
    resource->setCustomFileName(QString());
    resource->setReferringPage(QString());
    resource->setDescription(QString());
    resource->setDestination(destinationPath);
    resource->setMask(mask);
    resource->setCheckSum(QString());

    auto item = new DownloadItem(downloadManager);
    item->setResource(resource);

    downloadManager->append(toList(item), true);
}

/******************************************************************************
 ******************************************************************************/
void AddBatchDialog::accept()
{
    doAccept(true);
}

void AddBatchDialog::acceptPaused()
{
    doAccept(false);
}

void AddBatchDialog::reject()
{
    QDialog::reject();
}

/******************************************************************************
 ******************************************************************************/
QString AddBatchDialog::insertName(const QString &name) const
{
    return QString("%0 [ %1 ]").arg(tr("Insert").arg(name));
}

void AddBatchDialog::showContextMenu(const QPoint &/*pos*/)
{
    QMenu *contextMenu = ui->urlLineEdit->createStandardContextMenu();
    QAction *first = contextMenu->actions().first();

    QAction action_1_to_10(insertName(tr("1 -> 10")), contextMenu);
    QAction action_1_to_100(insertName(tr("1 -> 100")), contextMenu);
    QAction action_01_to_10(insertName(tr("01 -> 10")), contextMenu);
    QAction action_001_to_100(insertName(tr("001 -> 100")), contextMenu);
    QAction action_custom(insertName(ui->tagButton_Custom->text()), contextMenu);

    connect(&action_1_to_10, SIGNAL(triggered()), this, SLOT(insert_1_to_10()));
    connect(&action_1_to_100, SIGNAL(triggered()), this, SLOT(insert_1_to_100()));
    connect(&action_01_to_10, SIGNAL(triggered()), this, SLOT(insert_01_to_10()));
    connect(&action_001_to_100, SIGNAL(triggered()), this, SLOT(insert_001_to_100()));
    connect(&action_custom, SIGNAL(triggered()), this, SLOT(insert_custom()));

    contextMenu->insertAction(first, &action_1_to_10);
    contextMenu->insertAction(first, &action_1_to_100);
    contextMenu->insertAction(first, &action_01_to_10);
    contextMenu->insertAction(first, &action_001_to_100);
    if (ui->tagButton_Custom->isVisible()) {
        contextMenu->insertAction(first, &action_custom);
    }
    contextMenu->insertSeparator(first);

    contextMenu->exec(QCursor::pos());
    contextMenu->deleteLater();
}

void AddBatchDialog::insert_1_to_10()
{
    ui->urlLineEdit->insert("[1:10]");
}

void AddBatchDialog::insert_01_to_10()
{
    ui->urlLineEdit->insert("[01:10]");
}

void AddBatchDialog::insert_1_to_100()
{
    ui->urlLineEdit->insert("[1:100]");
}

void AddBatchDialog::insert_001_to_100()
{
    ui->urlLineEdit->insert("[001:100]");
}

void AddBatchDialog::insert_custom()
{
    ui->urlLineEdit->insert(ui->tagButton_Custom->toolTip());
}

/******************************************************************************
 ******************************************************************************/
void AddBatchDialog::onChanged(QString)
{
    const bool enabled = ui->urlFormWidget->isValid();
    ui->startButton->setEnabled(enabled);
    ui->addPausedButton->setEnabled(enabled);
}

/******************************************************************************
 ******************************************************************************/
void AddBatchDialog::doAccept(bool started)
{
    const QString input = ui->urlFormWidget->url();
    const QUrl url = Mask::fromUserInput(input);

    // Remove trailing / and \ and .
    const QString adjusted = url.adjusted(QUrl::StripTrailingSlash).toString();

    if (Regex::hasBatchDescriptors(adjusted)) {
        QList<IDownloadItem*> items = createItems(url);

        QMessageBox::StandardButton answer = askBatchDownloading(items);

        if (answer == QMessageBox::Ok) {
            m_downloadManager->append(items, started);
            QDialog::accept();

        } else if (answer == QMessageBox::Apply) {
            m_downloadManager->append(toList(createItem(adjusted)), started);
            QDialog::accept();

        } else {
            return;
        }

    } else {
        m_downloadManager->append(toList(createItem(adjusted)), started);
        QDialog::accept();
    }
    writeSettings();
}

/******************************************************************************
 ******************************************************************************/
QMessageBox::StandardButton AddBatchDialog::askBatchDownloading(QList<IDownloadItem*> items)
{
    if (!m_settings || m_settings->isConfirmBatchDownloadEnabled()) {

        auto firstItem = dynamic_cast<DownloadItem*>(items.first());
        auto lastItem = dynamic_cast<DownloadItem*>(items.last());

        QMessageBox msgBox(this);
        msgBox.setModal(true);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(tr("Download Batch"));
        msgBox.setText(tr("It seems that you are using some batch descriptors."));
        msgBox.setInformativeText(
                    QString("%0\n\n"
                            "%1\n"
                            "...\n"
                            "%2")
                    .arg(tr("Do you really want to start %0 downloads?").arg(items.count()))
                    .arg(firstItem->resource()->url())
                    .arg(lastItem->resource()->url()));

        QPushButton *batchButton = msgBox.addButton(tr("Download Batch"), QMessageBox::ActionRole);
        QPushButton *singleButton = msgBox.addButton(tr("Single Download"), QMessageBox::ActionRole);
        QPushButton *cancelButton = msgBox.addButton(QMessageBox::Cancel);
        msgBox.setDefaultButton(batchButton);

        QCheckBox *cb = new QCheckBox(tr("Don't ask again, always download batch"));
        msgBox.setCheckBox(cb);
        QObject::connect(cb, &QCheckBox::stateChanged, [this](int state){
            if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
                m_settings->setConfirmBatchDownloadEnabled(false);
            }
        });

        msgBox.exec();
        auto clicked = msgBox.clickedButton();
        if (clicked == batchButton) {
            return QMessageBox::Ok;
        }
        if (clicked == singleButton) {
            return QMessageBox::Apply;
        }
        if (clicked == cancelButton) {
            return QMessageBox::Cancel;
        }
    }
    return QMessageBox::Ok;
}

/******************************************************************************
 ******************************************************************************/
QList<IDownloadItem*> AddBatchDialog::createItems(const QUrl &inputUrl) const
{
    QList<IDownloadItem*> items;
    const QStringList urls = Regex::interpret(inputUrl);
    foreach (auto url, urls) {
        items << createItem(url);
    }
    return items;
}

IDownloadItem* AddBatchDialog::createItem(const QString &url) const
{
    auto resource = ui->urlFormWidget->createResourceItem();
    resource->setUrl(url);

    auto item = new DownloadItem(m_downloadManager);
    item->setResource(resource);
    return item;
}

inline QList<IDownloadItem*> AddBatchDialog::toList(IDownloadItem *item)
{
    return QList<IDownloadItem*>() << item;
}

/******************************************************************************
 ******************************************************************************/
void AddBatchDialog::readSettings()
{
    // TODO move to urlFormWidget ?
    QSettings settings;
    settings.beginGroup("Wizard");
    ui->urlFormWidget->setCurrentPath(settings.value("Path", QString()).toString());
    ui->urlFormWidget->setPathHistory(settings.value("PathHistory").toStringList());
    ui->urlFormWidget->setCurrentMask(settings.value("Mask", QString()).toString());
    settings.endGroup();
}

void AddBatchDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    settings.setValue("Path", ui->urlFormWidget->currentPath());
    settings.setValue("PathHistory", ui->urlFormWidget->pathHistory());
    settings.setValue("Mask", ui->urlFormWidget->currentMask());
    settings.endGroup();
}
