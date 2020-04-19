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

#ifndef DIALOGS_PREFERENCE_DIALOG_H
#define DIALOGS_PREFERENCE_DIALOG_H

#include <Core/Settings>
#include <QtWidgets/QDialog>

class Settings;

namespace Ui {
class PreferenceDialog;
}

class PreferenceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferenceDialog(Settings *settings, QWidget *parent);
    ~PreferenceDialog() Q_DECL_OVERRIDE;

signals:
    void checkUpdate();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

public slots:
    void accept() Q_DECL_OVERRIDE;
    void reject() Q_DECL_OVERRIDE;
    virtual void restoreDefaultSettings();

private slots:
    void filterSelectionChanged();
    void filterTextChanged();
    void maxSimultaneousDownloadSlided(int value);

private:
    Ui::PreferenceDialog *ui;
    Settings *m_settings;

    void initializeGui();
    void initializeWarnings();
    void read();
    void write();
    void readSettings();
    void writeSettings();

    QStringList streamHosts() const;
    void setStreamHosts(const QStringList &streamHosts);

    QList<Filter> filters() const;
    void setFilters(const QList<Filter> &filters);

    ExistingFileOption existingFileOption() const;
    void setExistingFileOption(ExistingFileOption option);
};

#endif // DIALOGS_PREFERENCE_DIALOG_H
