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

#ifndef DIALOGS_TUTORIAL_DIALOG_H
#define DIALOGS_TUTORIAL_DIALOG_H

#include <QtWidgets/QDialog>

class Settings;

namespace Ui {
class TutorialDialog;
}

class TutorialDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TutorialDialog(Settings *settings, QWidget *parent = nullptr);
    ~TutorialDialog() override;

public slots:
    void closeEvent(QCloseEvent *) override;

private:
    Ui::TutorialDialog *ui = nullptr;
    Settings *m_settings = nullptr;

    void readSettings();
    void writeSettings();
};

#endif // DIALOGS_TUTORIAL_DIALOG_H
