/* - DownZemAll! - Copyright (C) 2019-present Sebastien Vavassori
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
    explicit UpdateDialog(UpdateChecker *updateChecker, QWidget *parent = Q_NULLPTR);
    ~UpdateDialog() override;

private slots:
    void check();
    void install();

    void onUpdateAvailable(const UpdateChecker::ChangeLog &changelog);
    void onDownloadProgress(qsizetype bytesReceived, qsizetype bytesTotal);
    void onUpdateDownloadFinished();
    void onUpdateError(const QString &errorMessage);

private:
    Ui::UpdateDialog *ui;
    UpdateChecker *m_updateChecker;
};

#endif // DIALOGS_UPDATE_DIALOG_H
