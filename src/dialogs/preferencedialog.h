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

#ifndef DIALOGS_PREFERENCE_DIALOG_H
#define DIALOGS_PREFERENCE_DIALOG_H

#include <Core/Settings>
#include <QtWidgets/QDialog>

class Settings;
class QTableWidgetItem;

namespace Ui {
class PreferenceDialog;
}

class PreferenceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferenceDialog(Settings *settings, QWidget *parent);
    ~PreferenceDialog() override;

signals:
    void checkUpdate();

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

public slots:
    void accept() override;
    void reject() override;
    virtual void restoreDefaultSettings();

private slots:
    void filterSelectionChanged();
    void filterChanged(QTableWidgetItem *item);
    void filterReset();
    void filterContextMenu(const QPoint &pos);
    void filterAdded();
    void filterUpdated();
    void filterRemoved();

    void languageChanged(int value);
    void resetLanguage();
    void themeChanged();

    void maxSimultaneousDownloadSlided(int value);
    void concurrentFragmentSlided(int value);

    void proxyTypeChanged(int index);
    void proxyAuthToggled(bool checked);
    void proxyShowPwdToggled(bool checked);

    void bandwidthSettingsChanged(int value);
    void setBandwidthSettings();

    void onStreamCleanCacheButtonReleased();
    void cleaned();

private:
    Ui::PreferenceDialog *ui;
    Settings *m_settings;

    void connectUi();
    void initializeUi();
    void initializeWarnings();
    void refreshTitle();
    void restylizeUi();

    void read();
    void write();
    void readSettings();
    void writeSettings();

    QStringList streamHosts() const;
    void setStreamHosts(const QStringList &streamHosts);
    void setupStreamToolTip();
    void setupHttpToolTips();

    void retranslateComboBox();

    QList<Filter> filters() const;
    void setFilters(const QList<Filter> &filters);
    void addFilter(const Filter & filter);
    void addFilter(const QString & key, const QString & name, const QString & regex);
    void retranslateFilters();

    ExistingFileOption existingFileOption() const;
    void setExistingFileOption(ExistingFileOption option);
};

#endif // DIALOGS_PREFERENCE_DIALOG_H
