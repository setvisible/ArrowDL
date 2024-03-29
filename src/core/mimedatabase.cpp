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

#include "mimedatabase.h"

#include <Constants>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QMimeDatabase>
#include <QtCore/QMimeType>
#include <QtCore/QString>
#include <QtCore/QTemporaryFile>
#include <QtCore/QUrl>
#include <QtGui/QPixmapCache>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileIconProvider>
#include <QtWidgets/QStyle>
#include <QtCore/QDebug>


class MimeDatabaseSingleton
{
    /*
     * Remark:
     * Here we use a Singleton, in order to prepare and store the Mimetype data
     * of the plateform only once.
     */
public:
    static MimeDatabaseSingleton& getInstance();

    QPixmap fileIcon(const QUrl &url, int extend);

private:
    MimeDatabaseSingleton() { init(); }

    /*
     * We make sure the default copy constructors are unacceptable
     * otherwise we may accidentally get copies of our singleton appearing
     */
    MimeDatabaseSingleton(MimeDatabaseSingleton const&) = delete;
    void operator=(MimeDatabaseSingleton const&) = delete;

    void init();

    QFileIconProvider m_fileIconProvider;
    QMimeDatabase m_mimeDatabase;
    QPixmap m_defaultPixmap;
};

/******************************************************************************
******************************************************************************/
MimeDatabaseSingleton& MimeDatabaseSingleton::getInstance()
{
    /* Guaranteed to be destroyed and instantiated on first use. */
    static MimeDatabaseSingleton instance;
    return instance;
}

void MimeDatabaseSingleton::init()
{
    /*
     * Just little performance tip.
     * If you need to provide those icons very often,
     * create one local instance of QFileIconProvider.
     * Construction of QFileIconProvider is heavy
     */
    m_fileIconProvider.setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);
    m_defaultPixmap = fileIcon(QUrl("page.html"), DEFAULT_ICON_SIZE);
}

/******************************************************************************
******************************************************************************/
QPixmap MimeDatabase::fileIcon(const QUrl &url, int extend)
{
    return MimeDatabaseSingleton::getInstance().fileIcon(url, extend);
}

QPixmap MimeDatabaseSingleton::fileIcon(const QUrl &url, int extend)
{
    QPixmap pixmap;

    QFileInfo fileInfo(url.toString());

    if ( fileInfo.suffix().isEmpty() ||
         (fileInfo.suffix() == "exe" && fileInfo.exists())) {
        auto icon = m_fileIconProvider.icon(fileInfo);
        pixmap = icon.pixmap(extend);
        if (pixmap.isNull()) {
            pixmap = m_defaultPixmap;
        }
        return pixmap;
    }

    auto key = QString("%0 %1").arg(fileInfo.suffix(), QString::number(extend));

    if (!QPixmapCache::find(key, &pixmap)) {

        auto nativeName = url.fileName();
        Q_ASSERT(!nativeName.contains('\\'));
        Q_ASSERT(!nativeName.contains('/')); // otherwise the temporary file is not opened

        const QDir dir(QDir::tempPath());
        if (!dir.exists()) {
            qWarning("Can't find: '%s'.", dir.path().toLatin1().data());
            return {};
        }

        auto filename = dir.filePath("XXXXXX_" + nativeName);
        QTemporaryFile tempFile(filename);
        if (tempFile.open()) {
            /* This is a trick to write the file */
        }

        const QFileInfo tempFileInfo(tempFile);
        auto icon = m_fileIconProvider.icon(tempFileInfo);
        if (icon.isNull()) {
            icon = m_fileIconProvider.icon(QFileIconProvider::File);
        }
        if (icon.isNull()) {
            icon = QApplication::style()->standardIcon(QStyle::SP_FileIcon);
        }
        pixmap = icon.pixmap(extend);
        if (!pixmap.isNull()) {
            QPixmapCache::insert(key, pixmap);
            qInfo("Mimetype icon cached {key:'%s', suffix:'%s', type:'%s'}.",
                  key.toLatin1().data(),
                  fileInfo.suffix().toLatin1().data(),
                  m_fileIconProvider.type(tempFileInfo).toLatin1().data());
        }
    }
    if (pixmap.isNull()) {
        pixmap = m_defaultPixmap;
    }
    return pixmap;
}
