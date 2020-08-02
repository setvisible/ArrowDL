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

#include "urlformwidget.h"
#include "ui_urlformwidget.h"

#include <Core/ResourceItem>

UrlFormWidget::UrlFormWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::UrlFormWidget)
{
    ui->setupUi(this);

    adjustSize();
    setFixedHeight(height());

    ui->pathWidget->setPathType(PathWidget::Directory);

    connect(ui->pathWidget, SIGNAL(currentPathChanged(QString)),
            this, SIGNAL(changed(QString)), Qt::QueuedConnection);
    connect(ui->maskWidget, SIGNAL(currentMaskChanged(QString)),
            this, SIGNAL(changed(QString)), Qt::QueuedConnection);
}

UrlFormWidget::~UrlFormWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void UrlFormWidget::setExternalUrlLabelAndLineEdit(QLabel *urlLabel, QLineEdit *urlLineEdit)
{
    bool enabled = urlLabel && urlLineEdit;
    ui->urlLabel->setVisible(!enabled);
    ui->urlLineEdit->setVisible(!enabled);
    if (enabled) {
        ui->urlLabel->setText(urlLabel->text());
        ui->urlLineEdit->setText(urlLineEdit->text());

        connect(urlLineEdit, SIGNAL(textChanged(const QString &)),
                ui->urlLineEdit, SLOT(setText(const QString &)));

        urlLabel->setMinimumWidth(ui->urlLabel->width());
    }

    // Recalculate the size, when the widgets are hidden
    recalculateSize();
}

/******************************************************************************
 ******************************************************************************/
bool UrlFormWidget::isValid() const
{
    return !ui->urlLineEdit->text().isEmpty() &&
            !ui->pathWidget->currentPath().isEmpty() &&
            !ui->maskWidget->currentMask().isEmpty();
}

/******************************************************************************
 ******************************************************************************/
ResourceItem* UrlFormWidget::createResourceItem() const
{
    auto resource = new ResourceItem();
    resource->setUrl(this->url());
    resource->setCustomFileName(ui->customFileNameLineEdit->text());
    resource->setReferringPage(ui->referrerLineEdit->text());
    resource->setDescription(ui->descriptionLineEdit->text());
    resource->setDestination(ui->pathWidget->currentPath());
    resource->setMask(ui->maskWidget->currentMask());
    resource->setCheckSum(ui->hashLineEdit->text());
    return resource;
}

void UrlFormWidget::setResource(const ResourceItem *resource)
{
    if (resource) {
        ui->urlLineEdit->setText(resource->url());
        ui->customFileNameLineEdit->setText(resource->customFileName());
        ui->referrerLineEdit->setText(resource->referringPage());
        ui->descriptionLineEdit->setText(resource->description());
        ui->pathWidget->setCurrentPath(resource->destination());
        ui->maskWidget->setCurrentMask(resource->mask());
        ui->hashLineEdit->setText(resource->checkSum());
    }
}

/******************************************************************************
 ******************************************************************************/
bool UrlFormWidget::isChildrenEnabled() const
{
    return ui->customFileNameLineEdit->isEnabled();
}

void UrlFormWidget::setChildrenEnabled(bool enabled)
{
    ui->customFileNameLineEdit->setEnabled(enabled);
    ui->referrerLineEdit->setEnabled(enabled);
    ui->descriptionLineEdit->setEnabled(enabled);
    ui->pathWidget->setEnabled(enabled);
    ui->maskWidget->setEnabled(enabled);
    ui->hashLineEdit->setEnabled(enabled);
}

/******************************************************************************
 ******************************************************************************/
QString UrlFormWidget::url() const
{
    const QUrl url(ui->urlLineEdit->text());

    // Remove trailing / and \ and . in the given text.
    const QString adjusted = url.adjusted(QUrl::StripTrailingSlash).toString();

    return adjusted;
}

/******************************************************************************
 ******************************************************************************/
QString UrlFormWidget::currentPath() const
{
    return ui->pathWidget->currentPath();
}

void UrlFormWidget::setCurrentPath(const QString &path)
{
    ui->pathWidget->setCurrentPath(path);
}

/******************************************************************************
 ******************************************************************************/
QStringList UrlFormWidget::pathHistory() const
{
    return ui->pathWidget->pathHistory();
}

void UrlFormWidget::setPathHistory(const QStringList &paths)
{
    ui->pathWidget->setPathHistory(paths);
}

/******************************************************************************
 ******************************************************************************/
QString UrlFormWidget::currentMask() const
{
    return ui->maskWidget->currentMask();
}

void UrlFormWidget::setCurrentMask(const QString &text)
{
    ui->maskWidget->setCurrentMask(text);
}

/******************************************************************************
 ******************************************************************************/
void UrlFormWidget::recalculateSize()
{
    auto height = sizeHint().height();
    setMinimumHeight(height);
    setMaximumHeight(height);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}
