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

#include "addurlsdialog.h"
#include "ui_addurlsdialog.h"

#include <Constants>
#include <Core/DownloadFileItem>
#include <Core/DownloadManager>
#include <Core/ResourceItem>
#include <Core/Settings>
#include <Core/Theme>
#include <Widgets/UrlFormWidget>
#include <Widgets/TextEditorWidget>

#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QSettings>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QLineEdit>


AddUrlsDialog::AddUrlsDialog(
    const QString &text,
    DownloadManager *downloadManager,
    Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddUrlsDialog)
    , m_fakeUrlLineEdit(new QLineEdit(this))
    , m_downloadManager(downloadManager)
    , m_settings(settings)
{
    ui->setupUi(this);

    setWindowTitle(QString("%0 - %1").arg(STR_APPLICATION_NAME, tr("Add Urls")));

    Theme::setIcons(this, { {ui->logo, "add-urls"} });

    ui->urlFormWidget->setExternalUrlLabelAndLineEdit(ui->urlLabel, m_fakeUrlLineEdit);
    if (m_settings->isHttpReferringPageEnabled()) {
        ui->urlFormWidget->setReferringPage(m_settings->httpReferringPage());
    }

    m_fakeUrlLineEdit->setVisible(false);
    ui->urlFormWidget->hideCustomFile();

    connect(m_fakeUrlLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onChanged(QString)));
    connect(ui->editor, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    connect(ui->urlFormWidget, SIGNAL(changed(QString)), this, SLOT(onChanged(QString)));

    ui->editor->clear();
    ui->editor->append(text);
    ui->editor->setModified(false);
    ui->editor->setFocus();

    readUiSettings();
}

AddUrlsDialog::~AddUrlsDialog()
{
    writeUiSettings();
    delete ui;
}

void AddUrlsDialog::readUiSettings()
{
    QSettings settings;
    settings.beginGroup("UrlsDialog");
    resize(settings.value("DialogSize", size()).toSize());
    settings.endGroup();
}

void AddUrlsDialog::writeUiSettings()
{
    QSettings settings;
    settings.beginGroup("UrlsDialog");
    settings.setValue("DialogSize", size());
    settings.endGroup();
}

/******************************************************************************
 ******************************************************************************/
void AddUrlsDialog::accept()
{
    doAccept(true);
}

void AddUrlsDialog::acceptPaused()
{
    doAccept(false);
}

void AddUrlsDialog::reject()
{
    QDialog::reject();
}

/******************************************************************************
 ******************************************************************************/
void AddUrlsDialog::onChanged(QString)
{
    const bool enabled = ui->urlFormWidget->isValid();
    ui->startButton->setEnabled(enabled);
    ui->addPausedButton->setEnabled(enabled);
}

void AddUrlsDialog::onTextChanged()
{
    const int count = ui->editor->count();
    for (int index = 0; index < count; ++index) {
        auto text = ui->editor->at(index);
        auto simplified = text.simplified();
        if (!simplified.isEmpty()) {
            m_fakeUrlLineEdit->setText(simplified);
            return;
        }
    }
    m_fakeUrlLineEdit->setText(QString());
}

/******************************************************************************
 ******************************************************************************/
void AddUrlsDialog::doAccept(bool started)
{
    const int count = ui->editor->count();
    for (int index = 0; index < count; ++index) {
        auto text = ui->editor->at(index);
        auto simplified = text.simplified();
        if (!simplified.isEmpty()) {
            m_fakeUrlLineEdit->setText(simplified);
            const QString url = ui->urlFormWidget->url();
            m_downloadManager->append(toList(createFileItem(url)), started);
        }
    }
    QDialog::accept();
}

/******************************************************************************
 ******************************************************************************/
AbstractDownloadItem* AddUrlsDialog::createFileItem(const QString &url) const
{
    auto resource = ui->urlFormWidget->createResourceItem();
    resource->setUrl(url);
    auto item = new DownloadFileItem(m_downloadManager, resource);
    return item;
}

inline QList<AbstractDownloadItem*> AddUrlsDialog::toList(AbstractDownloadItem *item)
{
    return QList<AbstractDownloadItem*>() << item;
}
