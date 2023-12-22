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

#ifndef WIDGETS_SYSTEM_TRAY_H
#define WIDGETS_SYSTEM_TRAY_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QSystemTrayIcon>

class Settings;

class QAction;

class SystemTray : public QWidget
{
    Q_OBJECT
public:
    explicit SystemTray(QWidget *parent = nullptr);
    ~SystemTray() override = default;

    QString title() const;
    void setTitle(const QString &title);

    QString toolTip() const;
    void setToolTip(const QString &toolTip);

    void setupContextMenu(
            QAction *actionPreferences,
            QAction *actionQuit);

    /* Settings */
    Settings* settings() const;
    void setSettings(Settings *settings);

    void showBalloon(const QString &title, const QString &message);

    void showParentWidget();
    void hideParentWidget();

protected slots:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void onSystemTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onSystemTrayMessageClicked();
    void onHideWhenMinimized();
    void onSettingsChanged();

private:
    Settings *m_settings{nullptr};
    QSystemTrayIcon *m_trayIcon;
    QAction* m_titleAction{nullptr};
    QAction* m_restoreAction{nullptr};
    QAction* m_hideWhenMinimizedAction{nullptr};

    void retranslateUi();
};

#endif // WIDGETS_SYSTEM_TRAY_H
