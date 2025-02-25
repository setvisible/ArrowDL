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

#ifndef CORE_FILE_H
#define CORE_FILE_H

#include <QtCore/QObject>

class FileAccessManager;
class ResourceItem;
class Settings;
class QSaveFile;

class File : public QObject
{
    Q_OBJECT

public:
    enum OpenFlag {
        Open,
        Skip,
        Error
    };

    explicit File(QObject *parent = nullptr);
    ~File() override;

    static void setFileAccessManager(FileAccessManager *manager);

    OpenFlag open(ResourceItem *resource);

    void write(const QByteArray &data);
    bool commit();
    void cancel();

    bool isOpen() const;
    bool rename(ResourceItem *resource);
    QString customFileName() const;

    void setCreationFileTime(const QDateTime &newDate);
    void setLastModifiedFileTime(const QDateTime &newDate);
    void setAccessFileTime(const QDateTime &newDate);
    void setMetadataChangeFileTime(const QDateTime &newDate);

private:
    QSaveFile *m_file = nullptr;

    inline OpenFlag open(const QString &fileName);
    static inline QString nextAvailableName(const QString &name);
};

#endif // CORE_FILE_H
