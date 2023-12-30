/* - DownZemAll! - Copyright (C) 2019-present Sebastien Vavassori
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
#include <Core/ResourceItem>
#include <Core/Settings>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QSaveFile>
#include <QtCore/QDate>
#include <QtCore/QTime>

static IFileAccessManager *s_fileAccessManager = nullptr;

static ExistingFileOption existingFileOption()
{
    auto option = ExistingFileOption::Overwrite;
    if (s_fileAccessManager) {
        auto settings = s_fileAccessManager->settings();
        if (settings) {
            option = settings->existingFileOption();
        }
    }
    return option;
}

File::File(QObject *parent) : QObject(parent)
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
File::OpenFlag File::open(ResourceItem *resource)
{
    Q_ASSERT(resource);
    auto target = resource->localFileUrl();
    auto fileName = target.toLocalFile();

    auto flag = open(fileName);
    resource->setCustomFileName(customFileName());
    return flag;
}

File::OpenFlag File::open(const QString &fileName)
{
    // Check Path
    const QFileInfo fi(fileName);
    auto localFilePath = fi.absolutePath();
    QDir().mkpath(localFilePath);

    // Check Existing File
    auto safeFileName = fileName;
    if (QFile::exists(safeFileName)) {

        auto option = existingFileOption();

        while (s_fileAccessManager && option == ExistingFileOption::Ask) {
            option = s_fileAccessManager->aboutToModify(safeFileName);
        }

        if (option == ExistingFileOption::Rename) {
            safeFileName = nextAvailableName(fileName);

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
bool File::isOpen() const
{
    return m_file && m_file->isOpen();
}

/*!
 * \brief Rename file to the given resource file name.
 * If rename is a success, return true. Otherwise return false.
 */
bool File::rename(ResourceItem *resource)
{
    /* Flush and close the previous temporary file */
    QByteArray data;
    if (m_file && m_file->isOpen()) {
        m_file->flush();
        m_file->commit();
        QString oldFile = m_file->fileName();

        QFile inputFile(this);
        inputFile.setFileName(oldFile);
        if (inputFile.open(QIODevice::ReadOnly)) {
            data = inputFile.readAll();
            inputFile.close();
        }
        m_file->deleteLater();
        m_file = nullptr;
        QFile::remove(oldFile);
    }
    /* Open a new temporary file and append previous data */
    File::OpenFlag flag = open(resource);
    if (flag == Open) {
        write(data);
        return true;
    }
    return false;
}

/******************************************************************************
 ******************************************************************************/
void File::setCreationFileTime(const QDateTime &newDate)
{
    if (m_file && m_file->isOpen()) {
        m_file->setFileTime(newDate, QFileDevice::FileBirthTime);
    }
}

void File::setLastModifiedFileTime(const QDateTime &newDate)
{
    if (m_file && m_file->isOpen()) {
        m_file->setFileTime(newDate, QFileDevice::FileModificationTime);
    }
}

void File::setAccessFileTime(const QDateTime &newDate)
{
    if (m_file && m_file->isOpen()) {
        m_file->setFileTime(newDate, QFileDevice::FileAccessTime);
    }
}

void File::setMetadataChangeFileTime(const QDateTime &newDate)
{
    if (m_file && m_file->isOpen()) {
        m_file->setFileTime(newDate, QFileDevice::FileMetadataChangeTime);
    }
}

/******************************************************************************
 ******************************************************************************/
inline QString File::nextAvailableName(const QString &name)
{
    const QFileInfo fi(name);
    auto prefix = QString("%0/%1 (").arg(fi.absolutePath(), fi.baseName());
    auto suffix = QString(").%0").arg(fi.completeSuffix());
    auto newFileName = name;
    int increment = 0;
    do {
        newFileName = QString("%0%1%2").arg(prefix, QString::number(increment), suffix);
        increment++;
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
        auto commited = m_file->commit();
        m_file->deleteLater();
        m_file = nullptr;
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
        m_file = nullptr;
    }
}

/******************************************************************************
 ******************************************************************************/
QString File::customFileName() const
{
    if (m_file) {
        QFileInfo fi(m_file->fileName());
        return fi.completeBaseName();
    }
    return {};
}
