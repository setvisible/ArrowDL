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

#ifndef DIALOGS_INFORMATION_DIALOG_H
#define DIALOGS_INFORMATION_DIALOG_H

#include <QtCore/QList>
#include <QtWidgets/QDialog>

class AbstractJob;

namespace Ui {
class InformationDialog;
}

class InformationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InformationDialog(const QList<AbstractJob *> &jobs, QWidget *parent);
    ~InformationDialog() override;

public slots:
    void accept() override;

private slots:
    void wrapLog(bool enabled);

private:
    Ui::InformationDialog *ui = nullptr;
    QList<AbstractJob *> m_jobs = {};

    void initialize(const QList<AbstractJob*> &jobs);

    void readUiSettings();
    void writeUiSettings();
};

#endif // DIALOGS_INFORMATION_DIALOG_H
