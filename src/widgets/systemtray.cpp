/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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

#include "systemtray.h"

#include <Core/Settings>

#include <QtCore/QDebug>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>


SystemTray::SystemTray(QWidget *parent) : QWidget(parent)
  , m_settings(Q_NULLPTR)
  , m_trayIcon(new QSystemTrayIcon(this))
  , m_titleAction(Q_NULLPTR)
  , m_hideWhenMinimizedAction(Q_NULLPTR)
{
    Q_ASSERT(parent);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qDebug() << Q_FUNC_INFO << "System Tray Icon not supported.";
    }

    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &SystemTray::onSystemTrayIconActivated);

    connect(m_trayIcon, &QSystemTrayIcon::messageClicked,
            this, &SystemTray::onSystemTrayMessageClicked);

    m_trayIcon->setIcon(QIcon(":/icons/logo/icon16.png"));
    m_trayIcon->hide();

}

/******************************************************************************
 ******************************************************************************/
QString SystemTray::title() const
{
    return m_titleAction ? m_titleAction->text() : QString();
}

void SystemTray::setTitle(const QString &title)
{
    if (m_titleAction) {
        m_titleAction->setText(title);
    }
}

/******************************************************************************
 ******************************************************************************/
QString SystemTray::toolTip() const
{
    return m_trayIcon->toolTip();
}

void SystemTray::setToolTip(const QString &toolTip)
{
    m_trayIcon->setToolTip(toolTip);
}

/******************************************************************************
 ******************************************************************************/
void SystemTray::setupContextMenu(
        QAction *actionPreferences,
        QAction *actionQuit)
{
    auto menu = new QMenu(this);

    m_titleAction = new QAction(QLatin1String("-"), this);
    m_titleAction->setEnabled(false);
    m_titleAction->setIconVisibleInMenu(false);

    auto restoreAction = new QAction(tr("&Restore"), this);
    auto font = restoreAction->font();
    font.setBold(true);
    restoreAction->setFont(font);
    connect(restoreAction, &QAction::triggered, parentWidget(), &QWidget::showNormal);

    m_hideWhenMinimizedAction = new QAction(tr("&Hide when Minimized"), this);
    m_hideWhenMinimizedAction->setCheckable(true);
    connect(m_hideWhenMinimizedAction, SIGNAL(triggered()), this, SLOT(onHideWhenMinimized()));

    menu->addAction(m_titleAction);
    menu->addSeparator();
    menu->addAction(restoreAction);
    menu->addAction(actionPreferences);
    menu->addAction(m_hideWhenMinimizedAction);
    menu->addSeparator();
    menu->addAction(actionQuit);

    m_trayIcon->setContextMenu(menu);
}

void SystemTray::onHideWhenMinimized()
{
    m_settings->setHideWhenMinimizedEnabled(m_hideWhenMinimizedAction->isChecked());
}

/******************************************************************************
 ******************************************************************************/
Settings *SystemTray::settings() const
{
    return m_settings;
}

void SystemTray::setSettings(Settings *settings)
{
    if (m_settings) {
        disconnect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
    }
    m_settings = settings;
    if (m_settings) {
        connect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
    }
}

void SystemTray::onSettingsChanged()
{
    m_trayIcon->setVisible(m_settings->isSystemTrayIconEnabled());
    if (m_hideWhenMinimizedAction) {
        m_hideWhenMinimizedAction->setChecked(m_settings->isHideWhenMinimizedEnabled());
    }
}

/******************************************************************************
 ******************************************************************************/
void SystemTray::showBalloon(const QString &title, const QString &message)
{
    if (m_settings->isSystemTrayBalloonEnabled()) {
        if (QSystemTrayIcon::supportsMessages()) {
#if QT_VERSION >= 0x050900
            m_trayIcon->showMessage(title, message, QIcon(":/icons/logo/icon48.png"));
#else
            m_trayIcon->showMessage(title, message);
#endif
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void SystemTray::showParentWidget()
{
    Q_ASSERT(parentWidget());
    auto parent = this->parentWidget();

    parent->show();

    // de-minimize
    if (parent->isMinimized()) {
        parent->setWindowState((parent->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    }
}

void SystemTray::hideParentWidget()
{
    Q_ASSERT(parentWidget());
    auto parent = this->parentWidget();

    if (m_settings->isHideWhenMinimizedEnabled()) {
        parent->hide();
    } else {
        parent->showMinimized();
    }
}


/******************************************************************************
 ******************************************************************************/
void SystemTray::onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    Q_ASSERT(parentWidget());
    auto parent = this->parentWidget();
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:
        if (!parent->isVisible() || parent->isMinimized()) {
            showParentWidget();
        } else {
            hideParentWidget();
        }
        break;
    default:
        ;
    }
}

void SystemTray::onSystemTrayMessageClicked()
{
    showParentWidget();
}
