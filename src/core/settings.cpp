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

#include "settings.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QSettings>
#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

/*!
 * Characters that are unlikely to be used as value for data or directory path.
 * If a collision appears, the only risk is to reset the faulty parameter
 * to its default value.
 */
static const QString UNDEFINED = "<UNDEFINED>";

struct SettingsItem
{
    QString key;
    QString value;
    QString defaultValue;
};

Settings::Settings(QObject *parent) : QObject(parent)
  , m_default(false)
{
    addSetting("Database", tr("%0/queue.json").arg(qApp->applicationDirPath()));
}

Settings::~Settings()
{
    qDeleteAll(m_items);
}

/******************************************************************************
 ******************************************************************************/
QString Settings::database() const
{
    return getSetting("Database");
}

void Settings::setDatabase(const QString &value)
{
    return setSetting("Database", value);
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Restore the default settings
 */
void Settings::beginRestoreDefault()
{
    m_default = true;
}

void Settings::endRestoreDefault()
{
    m_default = false;
}

void Settings::readSettings()
{
    QSettings settings;
    settings.beginGroup("Preference");
    foreach (auto item, m_items) {
        const QString value = settings.value(item->key, UNDEFINED).toString();
        item->value = (value != UNDEFINED) ? value : item->defaultValue;
    }
    settings.endGroup();
    emit changed();
}

void Settings::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Preference");
    foreach (auto item, m_items) {
        if (item->value != item->defaultValue) {
            settings.setValue(item->key, item->value);
        }
    }
    settings.endGroup();
}

/******************************************************************************
 ******************************************************************************/
QString Settings::getSetting(const QString &key) const
{
    foreach (auto item, m_items) {
        if (item->key == key) {
            return m_default ? item->defaultValue : item->value;
        }
    }
    Q_UNREACHABLE(); // must find the entry!
}

void Settings::addSetting(const QString &key, const QString &defaultValue)
{
    foreach (auto item, m_items) {
        if (item->key == key) {
            item->defaultValue = defaultValue;
            return;
        }
    }
    SettingsItem *item = new SettingsItem();
    item->key = key;
    item->defaultValue = defaultValue;
    m_items.append(item);
}

void Settings::setSetting(const QString &key, const QString &value)
{
    Q_ASSERT(value != UNDEFINED);
    foreach (auto item, m_items) {
        if (item->key == key) {
            if (item->value != value) {
                item->value = value;
                emit changed();
            }
            return;
        }
    }
    Q_UNREACHABLE(); // must find the entry!
}
