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

#ifndef CORE_FILE_ACCESS_MANAGER_H
#define CORE_FILE_ACCESS_MANAGER_H

#include <Core/IFileAccessManager>
#include <QtWidgets/QWidget>

#include <Core/Settings>

class FileAccessManager : public QObject, public IFileAccessManager
{    
    Q_OBJECT

public:
    explicit FileAccessManager(QWidget *parent = Q_NULLPTR);
    virtual ~FileAccessManager() Q_DECL_OVERRIDE;

    virtual Settings* settings() const Q_DECL_OVERRIDE;
    void setSettings(Settings *settings);

    virtual ExistingFileOption aboutToModify(const QString &filename) Q_DECL_OVERRIDE;

private:
    QWidget *m_parent;
    Settings *m_settings;
};

#endif // CORE_FILE_ACCESS_MANAGER_H
