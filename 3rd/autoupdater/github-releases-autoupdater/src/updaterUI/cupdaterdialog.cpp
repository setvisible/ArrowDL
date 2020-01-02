#include "cupdaterdialog.h"

#include "ui_cupdaterdialog.h"

#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include <QPushButton>
#include <QStringBuilder>

CUpdaterDialog::CUpdaterDialog(QWidget *parent, const QString& githubRepoAddress,
							   const QString& versionString, bool silentCheck) :
	QDialog(parent),
	ui(new Ui::CUpdaterDialog),
	_silent(silentCheck),
	_updater(githubRepoAddress, versionString)
{
	ui->setupUi(this);

	if (_silent) {
		hide();
	}

	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &CUpdaterDialog::applyUpdate);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Install");

	ui->stackedWidget->setCurrentIndex(0);
	ui->progressBar->setMaximum(0);
	ui->progressBar->setValue(0);
	ui->lblPercentage->setVisible(false);

	_updater.setUpdateStatusListener(this);
	_updater.checkForUpdates();
}

CUpdaterDialog::~CUpdaterDialog()
{
	delete ui;
}

void CUpdaterDialog::applyUpdate()
{
#ifdef _WIN32
	ui->progressBar->setMaximum(100);
	ui->progressBar->setValue(0);
	ui->lblPercentage->setVisible(true);
	ui->lblOperationInProgress->setText("Downloading the update...");
	ui->stackedWidget->setCurrentIndex(0);

	_updater.downloadAndInstallUpdate(_latestUpdateUrl);
#else
	QMessageBox msg(
		QMessageBox::Question,
		tr("Manual update required"),
		tr("Automatic update is not supported on this operating system. Do you want to download and install the update manually?"),
		QMessageBox::Yes | QMessageBox::No,
		this);

	if (msg.exec() == QMessageBox::Yes)
		QDesktopServices::openUrl(QUrl(_latestUpdateUrl));
#endif
}

// If no updates are found, the changelog is empty
void CUpdaterDialog::onUpdateAvailable(CAutoUpdaterGithub::ChangeLog changelog)
{
	if (!changelog.empty())
	{
		ui->stackedWidget->setCurrentIndex(1);
		for (const auto& changelogItem: changelog)
			ui->changeLogViewer->append("<b>" % changelogItem.versionString % "</b>" % '\n' % changelogItem.versionChanges % "<p></p>");

		_latestUpdateUrl = changelog.front().versionUpdateUrl;
		show();
	}
	else
	{
		accept();
		if (!_silent)
			QMessageBox::information(this, tr("No update available"), tr("You already have the latest version of the program."));
	}
}

// percentageDownloaded >= 100.0f means the download has finished
void CUpdaterDialog::onUpdateDownloadProgress(float percentageDownloaded)
{
	ui->progressBar->setValue((int)percentageDownloaded);
	ui->lblPercentage->setText(QString::number(percentageDownloaded, 'f', 2) + " %");
}

void CUpdaterDialog::onUpdateDownloadFinished()
{
	accept();
}

void CUpdaterDialog::onUpdateError(QString errorMessage)
{
	reject();
	if (!_silent)
		QMessageBox::critical(this, tr("Error checking for updates"), tr(errorMessage.toUtf8().data()));
}
