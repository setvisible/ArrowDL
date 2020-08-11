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

#ifndef DIALOGS_BATCH_RENAME_DIALOG_H
#define DIALOGS_BATCH_RENAME_DIALOG_H

#include <QtCore/QList>
#include <QtWidgets/QDialog>

class IDownloadItem;
class DownloadItem;

namespace Ui {
class BatchRenameDialog;
}

class BatchRenameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BatchRenameDialog(const QList<IDownloadItem*> &items, QWidget *parent);
    ~BatchRenameDialog() Q_DECL_OVERRIDE;

public slots:
    void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;
    void accept() Q_DECL_OVERRIDE;

private slots:
    void onComboboxChanged(int index);

private:
    Ui::BatchRenameDialog *ui;
    QList<IDownloadItem *> m_items;

    void renameToDefault();
    void renameToEnumeration();
    void rename(DownloadItem *downloadItem, const QString &newName);

    int currentRadio() const;
    void setCurrentRadio(int index);

    void readSettings();
    void writeSettings();
};

#endif // DIALOGS_BATCH_RENAME_DIALOG_H
