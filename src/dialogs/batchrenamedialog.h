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

#ifndef DIALOGS_BATCH_RENAME_DIALOG_H
#define DIALOGS_BATCH_RENAME_DIALOG_H

#include <QtCore/QList>
#include <QtWidgets/QDialog>
#include <QtWidgets/QStyledItemDelegate>

class AbstractDownloadItem;
class DownloadFileItem;

namespace Ui {
class BatchRenameDialog;
}

class BatchRenameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BatchRenameDialog(const QList<AbstractDownloadItem*> &items, QWidget *parent);
    ~BatchRenameDialog() override;

public slots:
    void closeEvent(QCloseEvent *) override;
    void accept() override;

private slots:
    void onComboboxChanged(int index);

private:
    Ui::BatchRenameDialog *ui = nullptr;
    QList<AbstractDownloadItem *> m_items = {};

    void renameToDefault();
    void renameToEnumeration();
    void rename(DownloadFileItem *downloadItem, const QString &newName);

    int currentRadio() const;
    void setCurrentRadio(int index);

    void readSettings();
    void writeSettings();
};

class PopupItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // DIALOGS_BATCH_RENAME_DIALOG_H
