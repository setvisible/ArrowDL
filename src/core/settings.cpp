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
#include <QtCore/QDebug>

/*!
 * Registry Keys. They must be unique
 */
// Tab General

// Tab Interface
static const QString REGISTRY_START_MINIMIZED  = "StartMinimized";
static const QString REGISTRY_CONFIRM_REMOVAL  = "ConfirmRemoval";

// Tab Network
static const QString REGISTRY_MAX_SIMULTANEOUS = "MaxSimultaneous";

// Tab Privacy
static const QString REGISTRY_DATABASE         = "Database";

// Tab Filters
static const QString REGISTRY_FILTER_KEY       = "FilterKey";
static const QString REGISTRY_FILTER_VALUE     = "FilterValue";

// Tab Schedule

// Tab Advanced



Settings::Settings(QObject *parent) : AbstractSettings(parent)
{
    // Tab General

    // Tab Interface
    addDefaultSettingBool(REGISTRY_START_MINIMIZED, false);
    addDefaultSettingBool(REGISTRY_CONFIRM_REMOVAL, true);

    // Tab Network
    addDefaultSettingInt(REGISTRY_MAX_SIMULTANEOUS, 4);

    // Tab Privacy
    addDefaultSettingString(
                REGISTRY_DATABASE,
                QString("%0/queue.json").arg(qApp->applicationDirPath()));

    // Tab Filters
    addDefaultSettingStringList(
                REGISTRY_FILTER_KEY, QStringList()
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

    addDefaultSettingStringList(
                REGISTRY_FILTER_VALUE, QStringList()
                << "^.*$"
                << "^.*\\.(?:z(?:ip|[0-9]{2})|r(?:ar|[0-9]{2})|jar|bz2"
                   "|gz|tar|rpm|7z(?:ip)?|lzma|xz)$"
                << "^.*\\.(?:exe|msi|dmg|bin|xpi|iso)$"
                << "^.*\\.(?:mp3|wav|og(?:g|a)|flac|midi?|rm|aac|wma|mka|ape)$"
                << "^.*\\.(?:pdf|xlsx?|docx?|odf|odt|rtf)$"
                << "^.*\\.(?:jp(?:e?g|e|2)|gif|png|tiff?|bmp|ico)$"
                << "^.*\\.jp(e?g|e|2)$"
                << "^.*\\.png$"
                << "^.*\\.(?:mpeg|ra?m|avi|mp(?:g|e|4)|mov|divx|asf|qt"
                   "|wmv|m\\dv|rv|vob|asx|ogm|ogv|webm|flv|mkv)$"
                );

    // Tab Schedule

    // Tab Advanced

}

Settings::~Settings()
{
}

/******************************************************************************
 ******************************************************************************/
// Tab General

/******************************************************************************
 ******************************************************************************/
// Tab Interface
bool Settings::isStartMinimizedEnabled() const
{
    return getSettingBool(REGISTRY_START_MINIMIZED);
}

void Settings::setStartMinimizedEnabled(bool enabled)
{
    setSettingBool(REGISTRY_START_MINIMIZED, enabled);
}

bool Settings::isConfirmRemovalEnabled() const
{
    return getSettingBool(REGISTRY_CONFIRM_REMOVAL);
}

void Settings::setConfirmRemovalEnabled(bool enabled)
{
    setSettingBool(REGISTRY_CONFIRM_REMOVAL, enabled);
}

/******************************************************************************
 ******************************************************************************/
// Tab Network
int Settings::maxSimultaneousDownloads() const
{
    return getSettingInt(REGISTRY_MAX_SIMULTANEOUS);
}

void Settings::setMaxSimultaneousDownloads(int number)
{
    setSettingInt(REGISTRY_MAX_SIMULTANEOUS, number);
}

/******************************************************************************
 ******************************************************************************/
// Tab Privacy
QString Settings::database() const
{
    return getSettingString(REGISTRY_DATABASE);
}

void Settings::setDatabase(const QString &value)
{
    setSettingString(REGISTRY_DATABASE, value);
}

/******************************************************************************
 ******************************************************************************/
// Tab Filters
QList<Filter> Settings::filters() const
{
    QList<Filter> filters;
    QStringList keys = getSettingStringList(REGISTRY_FILTER_KEY);
    QStringList values = getSettingStringList(REGISTRY_FILTER_VALUE);
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
    setSettingStringList(REGISTRY_FILTER_KEY, keys);
    setSettingStringList(REGISTRY_FILTER_VALUE, values);
}

/******************************************************************************
 ******************************************************************************/
// Tab Schedule

/******************************************************************************
 ******************************************************************************/
// Tab Advanced

