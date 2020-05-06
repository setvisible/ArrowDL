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

#ifndef DIALOGS_STREAM_DIALOG_H
#define DIALOGS_STREAM_DIALOG_H

#include <QtWidgets/QDialog>

namespace Ui {
class StreamDialog;
}

class StreamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StreamDialog(QWidget *parent = Q_NULLPTR);
    ~StreamDialog() Q_DECL_OVERRIDE;

private slots:
    void on_okButton_released();

    void onError(QString errorMessage);
    void onCollected(QStringList extractors, QStringList descriptions);

private:
    Ui::StreamDialog *ui;

    inline void askStreamVersionAsync();
    inline void askStreamExtractorsAsync();
};

#endif // DIALOGS_STREAM_DIALOG_H
