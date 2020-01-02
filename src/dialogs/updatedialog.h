/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
 */

#ifndef DIALOGS_UPDATE_DIALOG_H
#define DIALOGS_UPDATE_DIALOG_H

#include <CAutoUpdaterGithub>

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
    ~UpdateDialog() Q_DECL_OVERRIDE;

private slots:
    void check();
    void install();

    void onUpdateAvailable(CAutoUpdaterGithub::ChangeLog changelog);
    void onUpdateDownloadProgress(float percentageDownloaded);
    void onUpdateDownloadFinished();
    void onUpdateError(QString errorMessage);

private:
    Ui::UpdateDialog *ui;
    UpdateChecker *m_updateChecker;
};

#endif // DIALOGS_UPDATE_DIALOG_H
