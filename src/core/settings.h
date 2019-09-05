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

#ifndef CORE_SETTINGS_H
#define CORE_SETTINGS_H

#include <QtCore/QObject>
#include <QtCore/QString>

class SettingsItem;

/*!
  \class Settings
 * \brief The Settings class manages the persistence of the User preferences
 *
 * User preferences are edited with the 'Preferences' dialog.
 */
class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent);
    virtual ~Settings();

    QString database() const;
    void setDatabase(const QString &value);

    void beginRestoreDefault();
    void endRestoreDefault();

    void readSettings();
    void writeSettings();

signals:
    void changed();

private:
    QList<SettingsItem*> m_items;
    bool m_default;

    inline QString getSetting(const QString &key) const;
    inline void addSetting(const QString &key, const QString &defaultValue);
    inline void setSetting(const QString &key, const QString &value);
};

#endif // CORE_SETTINGS_H
