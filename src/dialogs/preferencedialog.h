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

#include <QtWidgets/QDialog>

class Settings;

namespace Ui {
class PreferenceDialog;
}

class PreferenceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferenceDialog(Settings *settings, QWidget *parent = 0);
    ~PreferenceDialog();

protected:
    virtual void closeEvent(QCloseEvent *event);

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    virtual void restoreDefaultSettings();

private:
    Ui::PreferenceDialog *ui;
    Settings *m_settings;

    void initializeGui();
    void read();
    void write();
    void readSettings();
    void writeSettings();
};

#endif // DIALOGS_PREFERENCE_DIALOG_H
