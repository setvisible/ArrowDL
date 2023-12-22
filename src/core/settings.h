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

#ifndef CORE_SETTINGS_H
#define CORE_SETTINGS_H

#include <Core/AbstractSettings>

enum class ExistingFileOption{
    Rename = 0,
    Overwrite,
    Skip,
    Ask,

    LastOption // for safe cast
};

enum class CheckUpdateBeatMode{
    Never = 0,
    OnceADay = 1,
    OnceAWeek = 2
};

class Filter
{
public:
    QString key() const { return m_key; }
    void setKey(const QString &key) { m_key = key; }

    QString name() const;
    void setName(const QString &name) { m_name = name; }

    QString regex() const { return m_regex; }
    void setRegex(const QString &regex) { m_regex = regex; }

private:
    QString m_key;
    QString m_name;
    QString m_regex;
};

/*!
 * User preferences are edited with the 'Preferences' dialog.
 */
class Settings : public AbstractSettings
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent);
    ~Settings() override = default;

    // Tab General
    ExistingFileOption existingFileOption() const;
    void setExistingFileOption(ExistingFileOption option);

    // Tab Interface
    QString language() const;
    void setLanguage(const QString &language);

    QMap<QString, QVariant> theme() const;
    void setTheme(const QMap<QString, QVariant> &map);

    bool isDontShowTutorialEnabled() const;
    void setDontShowTutorialEnabled(bool enabled);

    bool isSystemTrayIconEnabled() const;
    void setSystemTrayIconEnabled(bool enabled);

    bool isHideWhenMinimizedEnabled() const;
    void setHideWhenMinimizedEnabled(bool enabled);

    bool isSystemTrayBalloonEnabled() const;
    void setSystemTrayBalloonEnabled(bool enabled);

    bool isMinimizeEscapeEnabled() const;
    void setMinimizeEscapeEnabled(bool enabled);

    bool isConfirmRemovalEnabled() const;
    void setConfirmRemovalEnabled(bool enabled);

    bool isConfirmBatchDownloadEnabled() const;
    void setConfirmBatchDownloadEnabled(bool enabled);

    bool isStreamHostEnabled() const;
    void setStreamHostEnabled(bool enabled);

    QStringList streamHosts() const;
    void setStreamHosts(const QStringList &hosts);

    // Tab Network
    int maxSimultaneousDownloads() const;
    void setMaxSimultaneousDownloads(int number);

    int concurrentFragments() const;
    void setConcurrentFragments(int fragments);

    bool isCustomBatchEnabled() const;
    void setCustomBatchEnabled(bool enabled);

    QString customBatchButtonLabel() const;
    void setCustomBatchButtonLabel(const QString &text);

    QString customBatchRange() const;
    void setCustomBatchRange(const QString &text);

    int proxyType() const;
    void setProxyType(int number);

    QString proxyHostName() const;
    void setProxyHostName(const QString &text);

    int proxyPort() const;
    void setProxyPort(int number);

    bool isProxyAuthEnabled() const;
    void setProxyAuthEnabled(bool enabled);

    QString proxyUser() const;
    void setProxyUser(const QString &text);

    QString proxyPassword() const;
    void setProxyPwd(const QString &text);

    int connectionProtocol() const;
    void setConnectionProtocol(int number);

    int connectionTimeout() const;
    void setConnectionTimeout(int number);

    bool isRemoteCreationTimeEnabled() const;
    void setRemoteCreationTimeEnabled(bool enabled);

    bool isRemoteLastModifiedTimeEnabled() const;
    void setRemoteLastModifiedTimeEnabled(bool enabled);

    bool isRemoteAccessTimeEnabled() const;
    void setRemoteAccessTimeEnabled(bool enabled);

    bool isRemoteMetadataChangeTimeEnabled() const;
    void setRemoteMetadataChangeTimeEnabled(bool enabled);

    bool isStreamMarkWatchedEnabled() const;
    void setStreamMarkWatchedEnabled(bool enabled);

    bool isStreamSubtitleEnabled() const;
    void setStreamSubtitleEnabled(bool enabled);

    bool isStreamThumbnailEnabled() const;
    void setStreamThumbnailEnabled(bool enabled);

    bool isStreamDescriptionEnabled() const;
    void setStreamDescriptionEnabled(bool enabled);

    bool isStreamMetadataEnabled() const;
    void setStreamMetadataEnabled(bool enabled);

    bool isStreamCommentEnabled() const;
    void setStreamCommentEnabled(bool enabled);

    bool isStreamShortcutEnabled() const;
    void setStreamShortcutEnabled(bool enabled);

    // Tab Privacy
    bool isRemoveCompletedEnabled() const;
    void setRemoveCompletedEnabled(bool enabled);

    bool isRemoveCanceledEnabled() const;
    void setRemoveCanceledEnabled(bool enabled);

    bool isRemovePausedEnabled() const;
    void setRemovePausedEnabled(bool enabled);

    QString database() const;
    void setDatabase(const QString &value);

    QString httpUserAgent() const;
    void setHttpUserAgent(const QString &value);
    static QStringList httpUserAgents();

    bool isHttpReferringPageEnabled() const;
    void setHttpReferringPageEnabled(bool enabled);

    QString httpReferringPage() const;
    void setHttpReferringPage(const QString &value);

    // Tab Filters
    QList<Filter> filters();
    void setFilters(const QList<Filter> &filters);

    QList<Filter> defaultFilters(bool defaults = true);
    static QString translateFilter(const QString &key, const QString &defaultName);

    // Tab Torrent
    bool isTorrentEnabled() const;
    void setTorrentEnabled(bool enabled);

    bool isTorrentShareFolderEnabled() const;
    void setTorrentShareFolderEnabled(bool enabled);

    QString shareFolder() const;
    void setShareFolder(const QString &value);

    QString torrentPeers() const;
    void setTorrentPeers(const QString &value);

    QMap<QString, QVariant> torrentSettings() const;
    void setTorrentSettings(const QMap<QString, QVariant> &map);

    // Tab Advanced
    CheckUpdateBeatMode checkUpdateBeatMode() const;
    void setCheckUpdateBeatMode(CheckUpdateBeatMode mode);
};

#endif // CORE_SETTINGS_H
