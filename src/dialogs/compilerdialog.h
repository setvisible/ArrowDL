/* - DownZemAll! - Copyright (C) 2019-present Sebastien Vavassori
 */

#ifndef DIALOGS_COMPILER_DIALOG_H
#define DIALOGS_COMPILER_DIALOG_H

#include <QtWidgets/QDialog>

namespace Ui {
class CompilerDialog;
}

class CompilerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompilerDialog(QWidget *parent = nullptr);
    ~CompilerDialog() override;

private slots:
    void onOkButtonReleased();

private:
    Ui::CompilerDialog *ui = nullptr;

    inline void populateOpenSSL();
    inline QString getLibraryInfo(const QString &libraryName);
    inline QString getVersionString(const QString &fName);

    inline void askStreamVersionAsync();
};

#endif // DIALOGS_COMPILER_DIALOG_H
