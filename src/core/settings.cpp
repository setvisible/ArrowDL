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
static const QString REGISTRY_EXISTING_FILE    = "ExistingFile";

// Tab Interface
static const QString REGISTRY_DONT_SHOW_TUTO   = "DontShowTutorial";
static const QString REGISTRY_CONFIRM_REMOVAL  = "ConfirmRemoval";
static const QString REGISTRY_CONFIRM_BATCH    = "ConfirmBatchDownload";

// Tab Network
static const QString REGISTRY_MAX_SIMULTANEOUS = "MaxSimultaneous";
static const QString REGISTRY_CUSTOM_BATCH     = "CustomBatchEnabled";
static const QString REGISTRY_CUSTOM_BATCH_BL  = "CustomBatchButtonLabel";
static const QString REGISTRY_CUSTOM_BATCH_RGE = "CustomBatchRange";

// Tab Privacy
static const QString REGISTRY_REMOVE_COMPLETED = "PrivacyRemoveCompleted";
static const QString REGISTRY_REMOVE_CANCELED  = "PrivacyRemoveCanceled";
static const QString REGISTRY_REMOVE_PAUSED    = "PrivacyRemovePaused";
static const QString REGISTRY_DATABASE         = "Database";

// Tab Filters
static const QString REGISTRY_FILTER_KEY       = "FilterKey";
static const QString REGISTRY_FILTER_VALUE     = "FilterValue";

// Tab Schedule

// Tab Advanced



Settings::Settings(QObject *parent) : AbstractSettings(parent)
{
    // Tab General
    addDefaultSettingInt(REGISTRY_EXISTING_FILE, static_cast<int>(ExistingFileOption::Skip));

    // Tab Interface
    addDefaultSettingBool(REGISTRY_DONT_SHOW_TUTO, false);
    addDefaultSettingBool(REGISTRY_CONFIRM_REMOVAL, true);
    addDefaultSettingBool(REGISTRY_CONFIRM_BATCH, true);

    // Tab Network
    addDefaultSettingInt(REGISTRY_MAX_SIMULTANEOUS, 4);
    addDefaultSettingBool(REGISTRY_CUSTOM_BATCH, true);
    addDefaultSettingString(REGISTRY_CUSTOM_BATCH_BL, QLatin1String("1 -> 25"));
    addDefaultSettingString(REGISTRY_CUSTOM_BATCH_RGE, QLatin1String("[1:25]"));

    // Tab Privacy
    addDefaultSettingBool(REGISTRY_REMOVE_COMPLETED, false);
    addDefaultSettingBool(REGISTRY_REMOVE_CANCELED, false);
    addDefaultSettingBool(REGISTRY_REMOVE_PAUSED, false);
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

/******************************************************************************
 ******************************************************************************/
// Tab General
ExistingFileOption Settings::existingFileOption() const
{
    int value = getSettingInt(REGISTRY_EXISTING_FILE);
    return (value >= 0 && value < static_cast<int>(ExistingFileOption::LastOption))
            ? static_cast<ExistingFileOption>(value)
            : ExistingFileOption::Skip;
}

void Settings::setExistingFileOption(ExistingFileOption option)
{
    setSettingInt(REGISTRY_EXISTING_FILE, static_cast<int>(option));
}

/******************************************************************************
 ******************************************************************************/
// Tab Interface
bool Settings::isDontShowTutorialEnabled() const
{
    return getSettingBool(REGISTRY_DONT_SHOW_TUTO);
}

void Settings::setDontShowTutorialEnabled(bool enabled)
{
    setSettingBool(REGISTRY_DONT_SHOW_TUTO, enabled);
}

bool Settings::isConfirmRemovalEnabled() const
{
    return getSettingBool(REGISTRY_CONFIRM_REMOVAL);
}

void Settings::setConfirmRemovalEnabled(bool enabled)
{
    setSettingBool(REGISTRY_CONFIRM_REMOVAL, enabled);
}

bool Settings::isConfirmBatchDownloadEnabled() const
{
    return getSettingBool(REGISTRY_CONFIRM_BATCH);
}

void Settings::setConfirmBatchDownloadEnabled(bool enabled)
{
    setSettingBool(REGISTRY_CONFIRM_BATCH, enabled);
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

bool Settings::isCustomBatchEnabled() const
{
    return getSettingBool(REGISTRY_CUSTOM_BATCH);
}

void Settings::setCustomBatchEnabled(bool enabled)
{
    setSettingBool(REGISTRY_CUSTOM_BATCH, enabled);
}

QString Settings::customBatchButtonLabel() const
{
    return getSettingString(REGISTRY_CUSTOM_BATCH_BL);
}

void Settings::setCustomBatchButtonLabel(const QString &text)
{
    setSettingString(REGISTRY_CUSTOM_BATCH_BL, text);
}

QString Settings::customBatchRange() const
{
    return getSettingString(REGISTRY_CUSTOM_BATCH_RGE);
}

void Settings::setCustomBatchRange(const QString &text)
{
    setSettingString(REGISTRY_CUSTOM_BATCH_RGE, text);
}

/******************************************************************************
 ******************************************************************************/
// Tab Privacy
bool Settings::isRemoveCompletedEnabled() const
{
    return getSettingBool(REGISTRY_REMOVE_COMPLETED);
}

void Settings::setRemoveCompletedEnabled(bool enabled)
{
    setSettingBool(REGISTRY_REMOVE_COMPLETED, enabled);
}

bool Settings::isRemoveCanceledEnabled() const
{
    return getSettingBool(REGISTRY_REMOVE_CANCELED);
}

void Settings::setRemoveCanceledEnabled(bool enabled)
{
    setSettingBool(REGISTRY_REMOVE_CANCELED, enabled);
}

bool Settings::isRemovePausedEnabled() const
{
    return getSettingBool(REGISTRY_REMOVE_PAUSED);
}

void Settings::setRemovePausedEnabled(bool enabled)
{
    setSettingBool(REGISTRY_REMOVE_PAUSED, enabled);
}

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

