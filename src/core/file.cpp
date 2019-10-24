/* - DownZemAll! - Copyright (C) 2019 Sebastien Vavassori
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

#include "file.h"

#include <Core/IFileAccessManager>
#include <Core/Settings>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QSaveFile>

static IFileAccessManager *s_fileAccessManager = Q_NULLPTR;


File::File(QObject *parent) : QObject(parent)
  , m_file(Q_NULLPTR)
{
}

File::~File()
{
    cancel();
}

/******************************************************************************
 ******************************************************************************/
void File::setFileAccessManager(IFileAccessManager *manager)
{
    s_fileAccessManager = manager;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Opens the given fileName, returning Open if successful; otherwise Error or Skip.
 */
File::OpenFlag File::open(const QString &fileName)
{
    // Check Path
    const QFileInfo fi(fileName);
    const QString localFilePath = fi.absolutePath();
    QDir().mkpath(localFilePath);

    // Check Existing File
    QString safeFileName = fileName;
    if (QFile::exists(safeFileName)) {

        ExistingFileOption option = ExistingFileOption::Overwrite;
        if (s_fileAccessManager) {
            const Settings* settings = s_fileAccessManager->settings();
            if (settings) {
                option = settings->existingFileOption();
            }
        }

        while (s_fileAccessManager && option == ExistingFileOption::Ask) {
            option = s_fileAccessManager->aboutToModify(safeFileName);
        }

        if (option == ExistingFileOption::Rename) {
            safeFileName = rename(fileName);

        } else if (option == ExistingFileOption::Overwrite) {
            QFile::remove(safeFileName);

        } else if (option == ExistingFileOption::Skip) {
            return Skip;

        } else {
            Q_UNREACHABLE();
        }
    }

    // Create and open file
    if (m_file) {
        cancel();
    }
    m_file = new QSaveFile(this);
    m_file->setFileName(safeFileName);
    if (m_file->isOpen() || m_file->open(QIODevice::WriteOnly)) {
        return Open;
    }
    return Error;
}

/******************************************************************************
 ******************************************************************************/
QString File::rename(const QString &name) const
{
    const QFileInfo fi(name);

    const QString prefix = QString("%0/%1 (")
            .arg(fi.absolutePath())
            .arg(fi.baseName());

    const QString suffix = QString(").%0")
            .arg(fi.completeSuffix());

    QString newFileName = name;
    int i = 0;
    do {
        newFileName = QString("%0%1%2").arg(prefix).arg(i).arg(suffix);
        i++;
    } while (QFile::exists(newFileName));
    return newFileName;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Writes the given bytes of data to the device.
 * \param data
 */
void File::write(const QByteArray &data)
{
    if (m_file) {
        m_file->write(data);
    }
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Finish writing the file (flush) and close it.
 *
 * Returns true if the temporary file is renamed to the final file.
 * Other returns false.
 *
 * It is mandatory to call this at the end of the saving operation,
 * otherwise the file will be discarded.
 */
bool File::commit()
{
    if (m_file) {
        const bool commited = m_file->commit();
        m_file->deleteLater();
        m_file = Q_NULLPTR;
        return commited;
    }
    return false;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Cancel writing (close) and remove the temporary file (if exists).
 */
void File::cancel()
{
    if (m_file) {
        m_file->cancelWriting();
        m_file->deleteLater();
        m_file = Q_NULLPTR;
    }
}