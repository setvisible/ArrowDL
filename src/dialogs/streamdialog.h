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
    explicit StreamDialog(QWidget *parent = nullptr);
    ~StreamDialog() override;

private slots:
    void onOkButtonReleased();

    void onError(const QString &errorMessage);
    void onCollected(const QStringList &extractors, const QStringList &descriptions);

private:
    Ui::StreamDialog *ui = nullptr;

    inline void askStreamVersionAsync();
    inline void askStreamExtractorsAsync();
};

#endif // DIALOGS_STREAM_DIALOG_H
