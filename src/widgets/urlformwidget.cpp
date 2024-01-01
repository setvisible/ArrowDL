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

#include "urlformwidget.h"
#include "ui_urlformwidget.h"

#include <Core/ResourceItem>

#include <QtCore/QDebug>
#include <QtCore/QSettings>


UrlFormWidget::UrlFormWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::UrlFormWidget)
{
    ui->setupUi(this);

    setSizePolicy(QSizePolicy::Expanding, // The widget should get as much horizontal space as possible
                  QSizePolicy::Maximum);  // The widget cannot be higher than the size provided by sizeHint()

    connect(ui->collapseButton, SIGNAL(released()),
            this, SLOT(onCollapseButtonReleased()));

    ui->pathWidget->setPathType(PathWidget::Directory);

    connect(ui->pathWidget, SIGNAL(currentPathChanged(QString)),
            this, SIGNAL(changed(QString)), Qt::QueuedConnection);
    connect(ui->maskWidget, SIGNAL(currentMaskChanged(QString)),
            this, SIGNAL(changed(QString)), Qt::QueuedConnection);

    readSettings();
}

UrlFormWidget::~UrlFormWidget()
{
    writeSettings();
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void UrlFormWidget::onCollapseButtonReleased()
{
    setCollapsed(!isCollapsed());
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

        connect(urlLineEdit, SIGNAL(textChanged(QString)), ui->urlLineEdit, SLOT(setText(QString)));

        urlLabel->setMinimumWidth(ui->urlLabel->width());
    }
    updateGeometry();
}

void UrlFormWidget::hideCustomFile()
{
    ui->customFileNameLabel->setVisible(false);
    ui->customFileNameLineEdit->setVisible(false);
    updateGeometry();
}

/******************************************************************************
 ******************************************************************************/
void UrlFormWidget::setReferringPage(const QString &referringPage)
{
    ui->referringPageLineEdit->setText(referringPage);
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
    resource->setReferringPage(ui->referringPageLineEdit->text());
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
        ui->referringPageLineEdit->setText(resource->referringPage());
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
    ui->referringPageLineEdit->setEnabled(enabled);
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
    auto adjusted = url.adjusted(QUrl::StripTrailingSlash).toString();

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
bool UrlFormWidget::isCollapsible() const
{
    return m_isCollapsible;
}

void UrlFormWidget::setCollapsible(bool enabled)
{
    m_isCollapsible = enabled;
    ui->collapseButton->setVisible(enabled);
    if (!enabled) {
        setCollapsed(false);
    }
    updateGeometry();
}

/******************************************************************************
 ******************************************************************************/
bool UrlFormWidget::isCollapsed() const
{
    return m_isCollapsed;
}

void UrlFormWidget::setCollapsed(bool collapsed)
{
    m_isCollapsed = collapsed;
    ui->collapseButton->setArrowType(m_isCollapsed ? Qt::UpArrow : Qt::DownArrow);
    ui->customFileNameLabel->setVisible(!m_isCollapsed);
    ui->customFileNameLineEdit->setVisible(!m_isCollapsed);
    ui->descriptionLabel->setVisible(!m_isCollapsed);
    ui->descriptionLineEdit->setVisible(!m_isCollapsed);
    ui->referringPageLabel->setVisible(!m_isCollapsed);
    ui->referringPageLineEdit->setVisible(!m_isCollapsed);
    ui->maskLabel->setVisible(!m_isCollapsed);
    ui->maskWidget->setVisible(!m_isCollapsed);
    ui->hashLabel->setVisible(!m_isCollapsed);
    ui->hashLineEdit->setVisible(!m_isCollapsed);
    updateGeometry();
}

/******************************************************************************
 ******************************************************************************/
void UrlFormWidget::readSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    if (isCollapsible()) {
        setCollapsed(settings.value("Collapsed", false).toBool());
    }
    setCurrentPath(settings.value("Path", QString()).toString());
    setPathHistory(settings.value("PathHistory").toStringList());
    setCurrentMask(settings.value("Mask", QString()).toString());
    settings.endGroup();
}

void UrlFormWidget::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Wizard");
    if (isCollapsible()) {
        settings.setValue("Collapsed", isCollapsed());
    }
    settings.setValue("Path", currentPath());
    settings.setValue("PathHistory", pathHistory());
    settings.setValue("Mask", currentMask());
    settings.endGroup();
}
