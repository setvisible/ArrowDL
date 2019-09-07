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
    addDefaultSetting("Database", QString("%0/queue.json").arg(qApp->applicationDirPath()));

    addDefaultListSetting(
                "FilterKey", QStringList()
                << "All Files"
                << "Archives (zip, rar...)"
                << "Application (exe, xpi...)"
                << "Audio (mp3, wav...)"
                << "Documents (pdf, odf...)"
                << "Images (jpg, png...)"
                << "Images JPEG"
                << "Images PNG"
                << "Video (mpeg, avi...)"
                );

    addDefaultListSetting(
                "FilterValue", QStringList()
                << "^.*$"
                << "^.*\\.(?:z(?:ip|[0-9]{2})|r(?:ar|[0-9]{2})|jar|bz2|gz|tar|rpm|7z(?:ip)?|lzma|xz)$"
                << "^.*\\.(?:exe|msi|dmg|bin|xpi|iso)$"
                << "^.*\\.(?:mp3|wav|og(?:g|a)|flac|midi?|rm|aac|wma|mka|ape)$"
                << "^.*\\.(?:pdf|xlsx?|docx?|odf|odt|rtf)$"
                << "^.*\\.(?:jp(?:e?g|e|2)|gif|png|tiff?|bmp|ico)$"
                << "^.*\\.jp(e?g|e|2)$"
                << "^.*\\.png$"
                << "^.*\\.(?:mpeg|ra?m|avi|mp(?:g|e|4)|mov|divx|asf|qt|wmv|m\\dv|rv|vob|asx|ogm|ogv|webm|flv|mkv)$"
                );
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
    setSetting("Database", value);
}

/******************************************************************************
 ******************************************************************************/
QList<Filter> Settings::filters() const
{
    QList<Filter> filters;
    QStringList keys = getListSetting("FilterKey");
    QStringList values = getListSetting("FilterValue");
    const int count = qMin(keys.count(), values.count());
    for (int i = 0; i < count; ++i) {
        Filter filter;
        filter.title = keys.at(i);
        filter.regexp = values.at(i);
        if (!filter.title.isEmpty()) {
            filters.append(filter);
        }
    }
    return filters;
}

void Settings::setFilters(const QList<Filter> &filters)
{
    QStringList keys;
    QStringList values;
    foreach (auto filter, filters) {
        keys.append(filter.title);
        values.append(filter.regexp);
    }
    setListSetting("FilterKey", keys);
    setListSetting("FilterValue", values);
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

void Settings::addDefaultSetting(const QString &key, const QString &defaultValue)
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

/******************************************************************************
 ******************************************************************************/
QStringList Settings::getListSetting(const QString &key) const
{
    QStringList ret;
    for (int i = 0; i < m_items.count(); ++i) {
        const QString subkey = QString("%0%1").arg(key).arg(i);
        foreach (auto item, m_items) {
            if (item->key == subkey) {
                ret << (m_default ? item->defaultValue : item->value);
            }
        }
    }
    return ret;
}

void Settings::addDefaultListSetting(const QString &key, const QStringList &defaultValue)
{
    for (int i = 0; i < defaultValue.count(); ++i) {
        const QString subkey = QString("%0%1").arg(key).arg(i);
        const QString subvalue = defaultValue.at(i);
        addDefaultSetting(subkey, subvalue);
    }
}

void Settings::setListSetting(const QString &key, const QStringList &value)
{
    for (int i = 0; i < value.count(); ++i) {
        const QString subkey = QString("%0%1").arg(key).arg(i);
        const QString subvalue = value.at(i);
        setSetting(subkey, subvalue);
    }
}
