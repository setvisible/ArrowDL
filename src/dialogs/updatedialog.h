/* - ArrowDL - Copyright (C) 2019-present Sebastien Vavassori
 */

#ifndef DIALOGS_UPDATE_DIALOG_H
#define DIALOGS_UPDATE_DIALOG_H

#include <Core/UpdateChecker>

#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>

class UpdateChecker;

namespace Ui {
class UpdateDialog;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(UpdateChecker *updateChecker, QWidget *parent = nullptr);
    ~UpdateDialog() override;

private slots:
    void check();
    void install();

    void onUpdateAvailableForGui(const UpdateChecker::ChangeLog &changelog);
    void onDownloadProgress(qsizetype bytesReceived, qsizetype bytesTotal);
    void onUpdateDownloadFinished();
    void onUpdateError(const QString &errorMessage);

private:
    Ui::UpdateDialog *ui = nullptr;
    UpdateChecker *m_updateChecker = nullptr;
};

#endif // DIALOGS_UPDATE_DIALOG_H
