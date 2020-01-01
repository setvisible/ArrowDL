#pragma once

#include "../cautoupdatergithub.h"

#include <QDialog>

namespace Ui {
	class CUpdaterDialog;
}

class CAutoUpdaterGithub;

class CUpdaterDialog : public QDialog, private CAutoUpdaterGithub::UpdateStatusListener
{
public:
	explicit CUpdaterDialog(QWidget *parent, const QString& githubRepoAddress, const QString& versionString, bool silentCheck = false);
	~CUpdaterDialog() override;

private:
	void applyUpdate();

private:
	// If no updates are found, the changelog is empty
	void onUpdateAvailable(CAutoUpdaterGithub::ChangeLog changelog) override;
	// percentageDownloaded >= 100.0f means the download has finished
	void onUpdateDownloadProgress(float percentageDownloaded) override;
	void onUpdateDownloadFinished() override;
	void onUpdateError(QString errorMessage) override;

private:
	Ui::CUpdaterDialog *ui;
	const bool _silent;

	QString _latestUpdateUrl;
	CAutoUpdaterGithub _updater;
};

