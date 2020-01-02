#include "updateinstaller.hpp"

#include <QtCore/QProcess>

bool UpdateInstaller::install(const QString& downloadedUpdateFilePath)
{
	return QProcess::startDetached('\"' + downloadedUpdateFilePath + '\"');
}
