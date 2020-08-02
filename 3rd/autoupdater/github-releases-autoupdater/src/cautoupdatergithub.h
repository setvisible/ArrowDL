#pragma once

#include <QtCore/QFile>
#include <QtCore/QString>

#include <functional>
#include <vector>

class QNetworkReply;

class CAutoUpdaterGithub : public QObject
{
public:
	struct VersionEntry {
		QString versionString;
		QString versionChanges;
		QString versionUpdateUrl;
	};

	using ChangeLog = std::vector<VersionEntry>;

	struct UpdateStatusListener {
		virtual ~UpdateStatusListener() = default;

		// If no updates are found, the changelog is empty
		virtual void onUpdateAvailable(ChangeLog changelog) = 0;
		virtual void onUpdateDownloadProgress(float percentageDownloaded) = 0;
		virtual void onUpdateDownloadFinished() = 0;
		virtual void onUpdateError(QString errorMessage) = 0;
	};

public:
	// If the string comparison functior is not supplied, case-insensitive natural sorting is used (using QCollator)
	CAutoUpdaterGithub(const QString& githubRepositoryAddress,
					   const QString& currentVersionString,
					   const std::function<QNetworkReply* (const QUrl&)>& callbackNetworkGet,
					   const std::function<bool (const QString&)>& addressMatcher = std::function<bool (const QString&)>(),
					   const std::function<bool (const QString&, const QString&)>& versionStringComparatorLessThan = std::function<bool (const QString&, const QString&)>());

	CAutoUpdaterGithub& operator=(const CAutoUpdaterGithub& other) = delete;

	void setUpdateStatusListener(UpdateStatusListener* listener);

	void checkForUpdates();

	QString installTempDir() const;
	void downloadAndInstallUpdate(const QString& updateUrl);

private:
	void updateCheckRequestFinished();
	void updateDownloaded();
	void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void onNewDataDownloaded();

private:
	QFile _downloadedBinaryFile;
	const QString _updatePageAddress;
	const QString _currentVersionString;
	const std::function<QNetworkReply* (const QUrl&)> _callbackNetworkGet;
	const std::function<bool (const QString&)> _addressMatcher;
	const std::function<bool (const QString&, const QString&)> _lessThanVersionStringComparator;

	UpdateStatusListener* _listener = nullptr;
};

