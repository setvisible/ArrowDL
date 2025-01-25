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

#include "settings.h"

#include <Constants>

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QStandardPaths>
#include <QtCore/QVariant>

using namespace Qt::Literals::StringLiterals;

static const QLatin1Char STREAM_HOST_SEPARATOR = QLatin1Char(' ');
static const QList<QString> DEFAULT_STREAM_HOST_LIST =
{
    #include "settings_default_hosts.h.txt"
};
static QString defaultStreamHost() {
    return DEFAULT_STREAM_HOST_LIST.join(STREAM_HOST_SEPARATOR);
}

static QString defaultTorrentShareFolder() {
    return QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
}


Settings::Settings(QObject *parent) : AbstractSettings(parent)
{
    // Tab General
    addDefaultSettingInt(REGISTRY_EXISTING_FILE, static_cast<int>(ExistingFileOption::Skip));

    // Tab Interface
    addDefaultSettingString(REGISTRY_UI_LANGUAGE, QLatin1String(""));
    addDefaultSettingString(REGISTRY_UI_THEME, QLatin1String(""));
    addDefaultSettingBool(REGISTRY_DONT_SHOW_TUTO, false);
    addDefaultSettingBool(REGISTRY_SHOW_SYSTEM_TRAY, true);
    addDefaultSettingBool(REGISTRY_HIDE_MINIMIZED, false);
    addDefaultSettingBool(REGISTRY_SHOW_BALLOON, true);
    addDefaultSettingBool(REGISTRY_MINIMIZE_ESCAPE, false);
    addDefaultSettingBool(REGISTRY_CONFIRM_REMOVAL, true);
    addDefaultSettingBool(REGISTRY_CONFIRM_BATCH, true);

    // Tab Network
    addDefaultSettingInt(REGISTRY_MAX_SIMULTANEOUS, 4);
    addDefaultSettingInt(REGISTRY_CONCURRENT_FRAG, DEFAULT_CONCURRENT_FRAGMENTS);
    addDefaultSettingBool(REGISTRY_CUSTOM_BATCH, true);
    addDefaultSettingString(REGISTRY_CUSTOM_BATCH_BL, QLatin1String("1 -> 25"));
    addDefaultSettingString(REGISTRY_CUSTOM_BATCH_RGE, QLatin1String("[1:25]"));
    addDefaultSettingBool(REGISTRY_STREAM_HOST, true);
    addDefaultSettingString(REGISTRY_STREAM_HOST_LIST, defaultStreamHost());

    addDefaultSettingInt(REGISTRY_PROXY_TYPE, 0);
    addDefaultSettingString(REGISTRY_PROXY_HOSTNAME, QLatin1String("proxy.example.com"));
    addDefaultSettingInt(REGISTRY_PROXY_PORT, 1080);
    addDefaultSettingBool(REGISTRY_PROXY_IS_AUTH, false);
    addDefaultSettingString(REGISTRY_PROXY_USERNAME, QLatin1String(""));
    addDefaultSettingString(REGISTRY_PROXY_PASSWORD, QLatin1String(""));

    addDefaultSettingInt(REGISTRY_SOCKET_TYPE, 0);
    addDefaultSettingInt(REGISTRY_SOCKET_TIMEOUT, DEFAULT_TIMEOUT_SECS);

    addDefaultSettingBool(REGISTRY_REMOTE_CREATION, true);
    addDefaultSettingBool(REGISTRY_REMOTE_LAST_MOD, true);
    addDefaultSettingBool(REGISTRY_REMOTE_ACCESS, false);
    addDefaultSettingBool(REGISTRY_REMOTE_META_MOD, false);

    addDefaultSettingBool(REGISTRY_STREAM_WATCHED, false);
    addDefaultSettingBool(REGISTRY_STREAM_SUBTITLE, false);
    addDefaultSettingBool(REGISTRY_STREAM_THUMBNAIL, false);
    addDefaultSettingBool(REGISTRY_STREAM_DESCR, false);
    addDefaultSettingBool(REGISTRY_STREAM_METADATA, false);
    addDefaultSettingBool(REGISTRY_STREAM_COMMENT, false);
    addDefaultSettingBool(REGISTRY_STREAM_SHORTCUT, false);

    // Tab Privacy
    addDefaultSettingBool(REGISTRY_REMOVE_COMPLETED, false);
    addDefaultSettingBool(REGISTRY_REMOVE_CANCELED, false);
    addDefaultSettingBool(REGISTRY_REMOVE_PAUSED, false);
    addDefaultSettingString(REGISTRY_DATABASE, QString("%0/queue.json").arg(qApp->applicationDirPath()));
    addDefaultSettingString(REGISTRY_HTTP_USER_AGENT, httpUserAgents().at(0));
    addDefaultSettingBool(REGISTRY_HTTP_REFERRER_ON, false);
    addDefaultSettingString(REGISTRY_HTTP_REFERRER, QLatin1String("https://www.example.com/"));

    // Tab Filters
    addDefaultSettingStringList(
                REGISTRY_FILTER_KEY, QStringList()
                << QLatin1String("KEY_ALL")
                << QLatin1String("KEY_ARCHIVES")
                << QLatin1String("KEY_APPLICATIONS")
                << QLatin1String("KEY_AUDIO")
                << QLatin1String("KEY_DOCUMENTS")
                << QLatin1String("KEY_IMAGES")
                << QLatin1String("KEY_IMAGES_JPEG")
                << QLatin1String("KEY_IMAGES_PNG")
                << QLatin1String("KEY_VIDEO")
                );

    addDefaultSettingStringList(
                REGISTRY_FILTER_NAME, QStringList()
                << QLatin1String("")
                << QLatin1String("")
                << QLatin1String("")
                << QLatin1String("")
                << QLatin1String("")
                << QLatin1String("")
                << QLatin1String("")
                << QLatin1String("")
                << QLatin1String("")
                );

    addDefaultSettingStringList(
                REGISTRY_FILTER_VALUE, QStringList()
                << QLatin1String("^.*$")
                << QLatin1String("^.*\\.(?:z(?:ip|[0-9]{2})|r(?:ar|[0-9]{2})|jar|bz2"
                                 "|gz|tar|rpm|7z(?:ip)?|lzma|xz)$")
                << QLatin1String("^.*\\.(?:exe|msi|dmg|bin|xpi|iso)$")
                << QLatin1String("^.*\\.(?:mp3|wav|og(?:g|a)|flac|midi?|rm|aac|wma|mka|ape)$")
                << QLatin1String("^.*\\.(?:pdf|xlsx?|docx?|odf|odt|rtf)$")
                << QLatin1String("^.*\\.(?:jp(?:e?g|e|2)|gif|png|tiff?|bmp|ico)$")
                << QLatin1String("^.*\\.jp(e?g|e|2)$")
                << QLatin1String("^.*\\.png$")
                << QLatin1String("^.*\\.(?:mpeg|ra?m|avi|mp(?:g|e|4)|mov|divx|asf|qt"
                                 "|wmv|m\\dv|rv|vob|asx|ogm|ogv|webm|flv|mkv)$")
                );

    // Tab Torrent
    addDefaultSettingBool(REGISTRY_TORRENT_ENABLED, true);
    addDefaultSettingBool(REGISTRY_TORRENT_SHARED, false);
    addDefaultSettingString(REGISTRY_TORRENT_DIR, defaultTorrentShareFolder());
    addDefaultSettingString(REGISTRY_TORRENT_PEERS, QLatin1String(""));
    addDefaultSettingString(REGISTRY_TORRENT_ADVANCED, QLatin1String(""));

    // Tab Advanced
    addDefaultSettingInt(REGISTRY_CHECK_UPDATE, static_cast<int>(CheckUpdateBeatMode::OnceADay));

}

/******************************************************************************
 ******************************************************************************/
// Tab General
ExistingFileOption Settings::existingFileOption() const
{
    auto value = getSettingInt(REGISTRY_EXISTING_FILE);
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
QString Settings::language() const
{
    return getSettingString(REGISTRY_UI_LANGUAGE);
}

void Settings::setLanguage(const QString &language)
{
    setSettingString(REGISTRY_UI_LANGUAGE, language);
}

/*!
 * \brief Defines the GUI's theme
 * (platform style, icon set theme, light/dark color scheme...)
 */
QMap<QString, QVariant> Settings::theme() const
{
    auto str = getSettingString(REGISTRY_UI_THEME);
    return deserialize(str);
}

void Settings::setTheme(const QMap<QString, QVariant> &map)
{
    auto value = serialize(map);
    setSettingString(REGISTRY_UI_THEME, value);
}

bool Settings::isDontShowTutorialEnabled() const
{
    return getSettingBool(REGISTRY_DONT_SHOW_TUTO);
}

void Settings::setDontShowTutorialEnabled(bool enabled)
{
    setSettingBool(REGISTRY_DONT_SHOW_TUTO, enabled);
}

bool Settings::isSystemTrayIconEnabled() const
{
    return getSettingBool(REGISTRY_SHOW_SYSTEM_TRAY);
}

void Settings::setSystemTrayIconEnabled(bool enabled)
{
    setSettingBool(REGISTRY_SHOW_SYSTEM_TRAY, enabled);
}

bool Settings::isHideWhenMinimizedEnabled() const
{
    return getSettingBool(REGISTRY_HIDE_MINIMIZED);
}

void Settings::setHideWhenMinimizedEnabled(bool enabled)
{
    setSettingBool(REGISTRY_HIDE_MINIMIZED, enabled);
}

bool Settings::isSystemTrayBalloonEnabled() const
{
    return getSettingBool(REGISTRY_SHOW_BALLOON);
}

void Settings::setSystemTrayBalloonEnabled(bool enabled)
{
    setSettingBool(REGISTRY_SHOW_BALLOON, enabled);
}

bool Settings::isMinimizeEscapeEnabled() const
{
    return getSettingBool(REGISTRY_MINIMIZE_ESCAPE);
}

void Settings::setMinimizeEscapeEnabled(bool enabled)
{
    setSettingBool(REGISTRY_MINIMIZE_ESCAPE, enabled);
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

bool Settings::isStreamHostEnabled() const
{
    return getSettingBool(REGISTRY_STREAM_HOST);
}

void Settings::setStreamHostEnabled(bool enabled)
{
    setSettingBool(REGISTRY_STREAM_HOST, enabled);
}

QStringList Settings::streamHosts() const
{
    return getSettingString(REGISTRY_STREAM_HOST_LIST)
            .split(STREAM_HOST_SEPARATOR, Qt::SkipEmptyParts);
}

void Settings::setStreamHosts(const QStringList &hosts)
{
    setSettingString(REGISTRY_STREAM_HOST_LIST,
                     hosts.join(STREAM_HOST_SEPARATOR));
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

int Settings::concurrentFragments() const
{
    return getSettingInt(REGISTRY_CONCURRENT_FRAG);
}

void Settings::setConcurrentFragments(int fragments)
{
    setSettingInt(REGISTRY_CONCURRENT_FRAG, fragments);
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

int Settings::proxyType() const
{
    return getSettingInt(REGISTRY_PROXY_TYPE);
}

void Settings::setProxyType(int number)
{
    setSettingInt(REGISTRY_PROXY_TYPE, number);
}

QString Settings::proxyHostName() const
{
    return getSettingString(REGISTRY_PROXY_HOSTNAME);
}

void Settings::setProxyHostName(const QString &text)
{
    setSettingString(REGISTRY_PROXY_HOSTNAME, text);
}

int Settings::proxyPort() const
{
    return getSettingInt(REGISTRY_PROXY_PORT);
}

void Settings::setProxyPort(int number)
{
    setSettingInt(REGISTRY_PROXY_PORT, number);
}

bool Settings::isProxyAuthEnabled() const
{
    return getSettingBool(REGISTRY_PROXY_IS_AUTH);
}

void Settings::setProxyAuthEnabled(bool enabled)
{
    setSettingBool(REGISTRY_PROXY_IS_AUTH, enabled);
}

QString Settings::proxyUser() const
{
    return getSettingString(REGISTRY_PROXY_USERNAME);
}

void Settings::setProxyUser(const QString &text)
{
    setSettingString(REGISTRY_PROXY_USERNAME, text);
}

QString Settings::proxyPassword() const
{
    return getSettingString(REGISTRY_PROXY_PASSWORD);
}

void Settings::setProxyPwd(const QString &text)
{
    setSettingString(REGISTRY_PROXY_PASSWORD, text);
}

int Settings::connectionProtocol() const
{
    return getSettingInt(REGISTRY_SOCKET_TYPE);
}

void Settings::setConnectionProtocol(int number)
{
    setSettingInt(REGISTRY_SOCKET_TYPE, number);
}

int Settings::connectionTimeout() const
{
    return getSettingInt(REGISTRY_SOCKET_TIMEOUT);
}

void Settings::setConnectionTimeout(int number)
{
    setSettingInt(REGISTRY_SOCKET_TIMEOUT, number);
}

bool Settings::isRemoteCreationTimeEnabled() const
{
    return getSettingBool(REGISTRY_REMOTE_CREATION);
}

void Settings::setRemoteCreationTimeEnabled(bool enabled)
{
    setSettingBool(REGISTRY_REMOTE_CREATION, enabled);
}

bool Settings::isRemoteLastModifiedTimeEnabled() const
{
    return getSettingBool(REGISTRY_REMOTE_LAST_MOD);
}

void Settings::setRemoteLastModifiedTimeEnabled(bool enabled)
{
    setSettingBool(REGISTRY_REMOTE_LAST_MOD, enabled);
}

bool Settings::isRemoteAccessTimeEnabled() const
{
    return getSettingBool(REGISTRY_REMOTE_ACCESS);
}

void Settings::setRemoteAccessTimeEnabled(bool enabled)
{
    setSettingBool(REGISTRY_REMOTE_ACCESS, enabled);
}

bool Settings::isRemoteMetadataChangeTimeEnabled() const
{
    return getSettingBool(REGISTRY_REMOTE_META_MOD);
}

void Settings::setRemoteMetadataChangeTimeEnabled(bool enabled)
{
    setSettingBool(REGISTRY_REMOTE_META_MOD, enabled);
}

bool Settings::isStreamMarkWatchedEnabled() const
{
    return getSettingBool(REGISTRY_STREAM_WATCHED);
}

void Settings::setStreamMarkWatchedEnabled(bool enabled)
{
    setSettingBool(REGISTRY_STREAM_WATCHED, enabled);
}

bool Settings::isStreamSubtitleEnabled() const
{
    return getSettingBool(REGISTRY_STREAM_SUBTITLE);
}

void Settings::setStreamSubtitleEnabled(bool enabled)
{
    setSettingBool(REGISTRY_STREAM_SUBTITLE, enabled);
}

bool Settings::isStreamThumbnailEnabled() const
{
    return getSettingBool(REGISTRY_STREAM_THUMBNAIL);
}

void Settings::setStreamThumbnailEnabled(bool enabled)
{
    setSettingBool(REGISTRY_STREAM_THUMBNAIL, enabled);
}

bool Settings::isStreamDescriptionEnabled() const
{
    return getSettingBool(REGISTRY_STREAM_DESCR);
}

void Settings::setStreamDescriptionEnabled(bool enabled)
{
    setSettingBool(REGISTRY_STREAM_DESCR, enabled);
}

bool Settings::isStreamMetadataEnabled() const
{
    return getSettingBool(REGISTRY_STREAM_METADATA);
}

void Settings::setStreamMetadataEnabled(bool enabled)
{
    setSettingBool(REGISTRY_STREAM_METADATA, enabled);
}

bool Settings::isStreamCommentEnabled() const
{
    return getSettingBool(REGISTRY_STREAM_COMMENT);
}

void Settings::setStreamCommentEnabled(bool enabled)
{
    setSettingBool(REGISTRY_STREAM_COMMENT, enabled);
}

bool Settings::isStreamShortcutEnabled() const
{
    return getSettingBool(REGISTRY_STREAM_SHORTCUT);
}

void Settings::setStreamShortcutEnabled(bool enabled)
{
    setSettingBool(REGISTRY_STREAM_SHORTCUT, enabled);
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

QString Settings::httpUserAgent() const
{
    return getSettingString(REGISTRY_HTTP_USER_AGENT);
}

void Settings::setHttpUserAgent(const QString &value)
{
    setSettingString(REGISTRY_HTTP_USER_AGENT, value);
}

QStringList Settings::httpUserAgents()
{
    return QStringList()
           // Last update: January 2025

           // Chrome on Windows
           << QLatin1String("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/132.0.0.0 Safari/537.36")

           // Edge on Windows
           << QLatin1String("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/132.0.0.0 Safari/537.36 Edg/131.0.2903.86")

           // Firefox on Windows
            << QLatin1String("Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:134.0) Gecko/20100101 Firefox/134.0")

           // Firefox on Linux
            << QLatin1String("Mozilla/5.0 (X11; Linux i686; rv:134.0) Gecko/20100101 Firefox/134.0")

           // Firefox on Android
           << QLatin1String("Mozilla/5.0 (Android 15; Mobile; rv:68.0) Gecko/68.0 Firefox/134.0")

           // Other
            << QLatin1String("Version/10.0 Mobile/14E304 Safari/602.1")
            << QLatin1String("ArrowDL/3.x")
            << QLatin1String("Java1.1.4")
            << QLatin1String("Lynx/2.8rel.3 libwww-FM/2.14")
            << QLatin1String("HyperBrowser (Cray; I; OrganicOS 9.7.42beta-27)");
}

bool Settings::isHttpReferringPageEnabled() const
{
    return getSettingBool(REGISTRY_HTTP_REFERRER_ON);
}

void Settings::setHttpReferringPageEnabled(bool enabled)
{
    setSettingBool(REGISTRY_HTTP_REFERRER_ON, enabled);
}

QString Settings::httpReferringPage() const
{
    return getSettingString(REGISTRY_HTTP_REFERRER);
}

void Settings::setHttpReferringPage(const QString &value)
{
    setSettingString(REGISTRY_HTTP_REFERRER, value);
}

/******************************************************************************
 ******************************************************************************/
// Tab Filters
QList<Filter> Settings::filters()
{
    return defaultFilters(false);
}

void Settings::setFilters(const QList<Filter> &filters)
{
    QStringList keys;
    QStringList names;
    QStringList values;
    for (auto filter : filters) {
        keys.append(filter.key());
        names.append(filter.name());
        values.append(filter.regex());
    }
    setSettingStringList(REGISTRY_FILTER_KEY, keys);
    setSettingStringList(REGISTRY_FILTER_NAME, names);
    setSettingStringList(REGISTRY_FILTER_VALUE, values);
}

/******************************************************************************
 ******************************************************************************/
QList<Filter> Settings::defaultFilters(bool defaults)
{
    QList<Filter> filters;

    if (defaults) {
        beginRestoreDefault();
    }
    auto keys = getSettingStringList(REGISTRY_FILTER_KEY);
    auto names = getSettingStringList(REGISTRY_FILTER_NAME);
    auto values = getSettingStringList(REGISTRY_FILTER_VALUE);
    if (defaults) {
        endRestoreDefault();
    }

    auto count = qMin(qMin(keys.count(), values.count()), names.count());
    for (auto i = 0; i < count; ++i) {
        Filter filter;
        filter.setKey(keys.at(i));
        filter.setName(names.at(i));
        filter.setRegex(values.at(i));
        filters.append(filter);
    }
    return filters;
}

/******************************************************************************
 ******************************************************************************/
QString Settings::translateFilter(const QString &key, const QString &defaultName)
{
    if (!key.isEmpty()) {
        if (key == QLatin1String("KEY_ALL")         ) { return tr("All Files"); }
        if (key == QLatin1String("KEY_ARCHIVES")    ) { return tr("Archives (zip, rar...)"); }
        if (key == QLatin1String("KEY_APPLICATIONS")) { return tr("Application (exe, xpi...)"); }
        if (key == QLatin1String("KEY_AUDIO")       ) { return tr("Audio (mp3, wav...)"); }
        if (key == QLatin1String("KEY_DOCUMENTS")   ) { return tr("Documents (pdf, odf...)"); }
        if (key == QLatin1String("KEY_IMAGES")      ) { return tr("Images (jpg, png...)"); }
        if (key == QLatin1String("KEY_IMAGES_JPEG") ) { return tr("Images JPEG"); }
        if (key == QLatin1String("KEY_IMAGES_PNG")  ) { return tr("Images PNG"); }
        if (key == QLatin1String("KEY_VIDEO")       ) { return tr("Video (mpeg, avi...)"); }
    }
    return defaultName;
}

QString Filter::name() const
{
    return Settings::translateFilter(m_key, m_name);
}

/******************************************************************************
 ******************************************************************************/
// Tab Torrent
bool Settings::isTorrentEnabled() const
{
    return getSettingBool(REGISTRY_TORRENT_ENABLED);
}

void Settings::setTorrentEnabled(bool enabled)
{
    setSettingBool(REGISTRY_TORRENT_ENABLED, enabled);
}

bool Settings::isTorrentShareFolderEnabled() const
{
    return getSettingBool(REGISTRY_TORRENT_SHARED);
}

void Settings::setTorrentShareFolderEnabled(bool enabled)
{
    setSettingBool(REGISTRY_TORRENT_SHARED, enabled);
}

QString Settings::shareFolder() const
{
    return getSettingString(REGISTRY_TORRENT_DIR);
}

void Settings::setShareFolder(const QString &value)
{
    setSettingString(REGISTRY_TORRENT_DIR, value);
}

QString Settings::torrentPeers() const
{
    return getSettingString(REGISTRY_TORRENT_PEERS);
}

void Settings::setTorrentPeers(const QString &value)
{
    setSettingString(REGISTRY_TORRENT_PEERS, value);
}

/* Other (advanced) settings */
QMap<QString, QVariant> Settings::torrentSettings() const
{
    auto str = getSettingString(REGISTRY_TORRENT_ADVANCED);
    return deserialize(str);
}

void Settings::setTorrentSettings(const QMap<QString, QVariant> &map)
{
    auto value = serialize(map);
    setSettingString(REGISTRY_TORRENT_ADVANCED, value);
}

/******************************************************************************
 ******************************************************************************/
// Tab Advanced
CheckUpdateBeatMode Settings::checkUpdateBeatMode() const
{
    return static_cast<CheckUpdateBeatMode>(getSettingInt(REGISTRY_CHECK_UPDATE));
}

void Settings::setCheckUpdateBeatMode(CheckUpdateBeatMode mode)
{
    setSettingInt(REGISTRY_CHECK_UPDATE, static_cast<int>(mode));
}
