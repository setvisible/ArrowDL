#include "updateinstaller.hpp"

#include <QProcess>

bool UpdateInstaller::install(const QString& downloadedUpdateFilePath)
{
	return QProcess::startDetached('\"' + downloadedUpdateFilePath + '\"');
}
