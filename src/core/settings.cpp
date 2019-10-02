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
static const QString REGISTRY_DATABASE = "Database";
static const QString REGISTRY_FILTER_KEY = "FilterKey";
static const QString REGISTRY_FILTER_VALUE = "FilterValue";

Settings::Settings(QObject *parent) : AbstractSettings(parent)
{
    addDefaultSetting(REGISTRY_DATABASE, QString("%0/queue.json").arg(qApp->applicationDirPath()));

    addDefaultListSetting(
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

    addDefaultListSetting(
                REGISTRY_FILTER_VALUE, QStringList()
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
}

/******************************************************************************
 ******************************************************************************/
QString Settings::database() const
{
    return getSetting(REGISTRY_DATABASE);
}

void Settings::setDatabase(const QString &value)
{
    setSetting(REGISTRY_DATABASE, value);
}

/******************************************************************************
 ******************************************************************************/
QList<Filter> Settings::filters() const
{
    QList<Filter> filters;
    QStringList keys = getListSetting(REGISTRY_FILTER_KEY);
    QStringList values = getListSetting(REGISTRY_FILTER_VALUE);
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
    setListSetting(REGISTRY_FILTER_KEY, keys);
    setListSetting(REGISTRY_FILTER_VALUE, values);
}
