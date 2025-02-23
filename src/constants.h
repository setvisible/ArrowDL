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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "version.h"

#include <QtCore/QString>
#include <chrono>

const int DEFAULT_WIDTH = 1000;
const int DEFAULT_HEIGHT = 700;
const int DEFAULT_X = 100;
const int DEFAULT_Y = 100;

const std::chrono::milliseconds TIMEOUT_TUTORIAL(250);
const std::chrono::milliseconds TIMEOUT_STATUSBAR(2000);
const std::chrono::milliseconds TIMEOUT_STATUSBAR_LONG(5000);

const int DIALOG_WIDTH = 600;

const int DEFAULT_TIMEOUT_SECS = 30; // ref.: QNetworkConfigurationPrivate::DefaultTimeout

const std::chrono::milliseconds TIMEOUT_COUNT_DOWN(1000);
const std::chrono::milliseconds TIMEOUT_INFO(150);

const int SELECTION_DISPLAY_LIMIT = 10;
const int MSEC_SPEED_DISPLAY_TIME = 2000;

const int MSEC_AUTO_SAVE = 3000; ///< Autosave the queue every 3 seconds.

/*
 * Remark:
 * Characters '<' and '>' are unlikely to be used as value for data or directory path.
 * If a collision appears, the only risk is to reset the faulty parameter
 * to its default value.
 */
const QLatin1StringView UNDEFINED("<UNDEFINED>");
const QLatin1StringView VALUE_TRUE("<TRUE>");
const QLatin1StringView VALUE_FALSE("<FALSE>");

const QLatin1StringView SETTING_GROUP_PREFERENCE("Preference");

// ********************************************************
// Inpired by: QtCreator source code
// Utils::FileNameValidatingLineEdit::validateFileName
// ********************************************************
/*
 * Naming a file like a device name will break on Windows,
 * even if it is "com1.txt".
 * Since we are cross-platform, we generally disallow such file names.
 */
#define WINDOWS_RESERVED_DEVICE_NAMES \
    "CON|PRN|AUX|NUL" \
    "|COM1|COM2|COM3|COM4|COM5|COM6|COM7|COM8|COM9" \
    "|LPT1|LPT2|LPT3|LPT4|LPT5|LPT6|LPT7|LPT8|LPT9"

/*
 * Validate a file base name, check for forbidden characters/strings.
 */
#define PRINTABLE_ASCII_CHARS "<>:\"|?*"
#define SLASHES "/\\"

const char S_FORBIDDEN_CHARS_SUB_DIR[] = PRINTABLE_ASCII_CHARS;
const char S_FORBIDDEN_CHARS_NO_SUB_DIR[] = PRINTABLE_ASCII_CHARS SLASHES;

const QString S_SUBSTITUTE_CHAR('_');
const QLatin1StringView S_SUBSTITUTE_FILE_NAME("file");

/*
 * This list of legal characters for filenames is limited to avoid injections
 * of special or invisible characters that could be not supported by the OS.
 */
const QLatin1StringView C_LEGAL_CHARS("-+' @()[]{}°#,.&");

const QLatin1StringView S_KEY_REGULAR("regular");
const QLatin1StringView S_KEY_STREAM("stream");
const QLatin1StringView S_KEY_TORRENT("torrent");

const QString SYMBOL_INFINITE = QString::fromUtf8("\xE2\x88\x9E");

const qint64 ONE_DAY_IN_SECONDS = 24 * 60 * 60;

const QLatin1StringView NAME("*name*");
const QLatin1StringView EXT("*ext*");
const QLatin1StringView URL("*url*");
const QLatin1StringView CURL("*curl*");
const QLatin1StringView FLATURL("*flaturl*");
const QLatin1StringView SUBDIRS("*subdirs*");
const QLatin1StringView FLATSUBDIRS("*flatsubdirs*");
const QLatin1StringView QSTRING("*qstring*");

const int MSEC_MESSAGE_TIMEOUT = 2000;

const int DEFAULT_ICON_SIZE = 32;
const int ICON_SIZE = 16;
const int ICON_WIDTH = 19;

const int MAX_REDIRECTS_ALLOWED = 5;

const int COLUMN_MINIMUM_WIDTH = 10;
const int COLUMN_DEFAULT_WIDTH = 100;
const int VERTICAL_HEADER_WIDTH = 22;
const int COLUMN_MAX_WIDTH = 1000;

const int COLUMN_0_DEFAULT_WIDTH = 300;
const int ROW_DEFAULT_HEIGHT = 22;
//const int ROW_DEFAULT_HEIGHT = 18;

const int MIN_PROGRESS = 0;
const int MAX_PROGRESS = 100;

const int VERSION_MARKER = 0xff;

const int COLUMN_ID_WIDTH = 10;
const int COLUMN_NAME_WIDTH = 200;

const int ELIDE_CHAR_COUNT = 30;

const int COL_0_FILE_NAME       = 0;
const int COL_1_WEBSITE_DOMAIN  = 1;
const int COL_2_PROGRESS_BAR    = 2;
const int COL_3_PERCENT         = 3;
const int COL_4_FILE_SIZE       = 4;
const int COL_4_SIZE            = 4; // to remove
const int COL_5_ESTIMATED_TIME  = 5;
const int COL_6_SPEED           = 6;
// const int COL_7_SEGMENTS          =  7; /* hidden */
// const int COL_8_MASK              =  8; /* hidden */
// const int COL_9_SAVE_PATH         =  9; /* hidden */
// const int COL_10_CHECKSUM         = 10; /* hidden */
const int COL_COUNT             = COL_6_SPEED + 1;

const int MAX_HISTORY_COUNT = 10;

const int CHECKBOX_SIZE = 12;
const int CHECKBOX_WIDTH = 16;
const int THUMBNAIL_WIDTH = 16;

const qsizetype MAX_PEER_LIST_COUNT = 1024;

/*!
 * Registry Keys. They must be unique
 */
// Tab General
const QLatin1StringView REGISTRY_EXISTING_FILE    ("ExistingFile");

// Tab Interface
const QLatin1StringView REGISTRY_UI_LANGUAGE      ("Language");
const QLatin1StringView REGISTRY_UI_THEME         ("Theme");
const QLatin1StringView REGISTRY_DONT_SHOW_TUTO   ("DontShowTutorial");
const QLatin1StringView REGISTRY_SHOW_SYSTEM_TRAY ("SystemTrayIconEnabled");
const QLatin1StringView REGISTRY_HIDE_MINIMIZED   ("HideWhenMinimized");
const QLatin1StringView REGISTRY_SHOW_BALLOON     ("SystemTrayBalloonEnabled");
const QLatin1StringView REGISTRY_MINIMIZE_ESCAPE  ("MinimizeWhenEscapePressed");
const QLatin1StringView REGISTRY_CONFIRM_REMOVAL  ("ConfirmRemoval");
const QLatin1StringView REGISTRY_CONFIRM_BATCH    ("ConfirmBatchDownload");
const QLatin1StringView REGISTRY_PROXY_TYPE       ("ProxyType");
const QLatin1StringView REGISTRY_PROXY_HOSTNAME   ("ProxyHostName");
const QLatin1StringView REGISTRY_PROXY_PORT       ("ProxyPort");
const QLatin1StringView REGISTRY_PROXY_IS_AUTH    ("ProxyAuth");
const QLatin1StringView REGISTRY_PROXY_USERNAME   ("ProxyUser");
const QLatin1StringView REGISTRY_PROXY_PASSWORD   ("ProxyPwd");
const QLatin1StringView REGISTRY_SOCKET_TYPE      ("SocketType");
const QLatin1StringView REGISTRY_SOCKET_TIMEOUT   ("SocketTimeout");
const QLatin1StringView REGISTRY_REMOTE_CREATION  ("RemoteCreationTime");
const QLatin1StringView REGISTRY_REMOTE_LAST_MOD  ("RemoteLastModifiedTime");
const QLatin1StringView REGISTRY_REMOTE_ACCESS    ("RemoteAccessTime");
const QLatin1StringView REGISTRY_REMOTE_META_MOD  ("RemoteMetadataChangeTime");
const QLatin1StringView REGISTRY_STREAM_WATCHED   ("StreamMarkWatchedEnabled");
const QLatin1StringView REGISTRY_STREAM_SUBTITLE  ("StreamSubtitleEnabled");
const QLatin1StringView REGISTRY_STREAM_THUMBNAIL ("StreamThumbnailEnabled");
const QLatin1StringView REGISTRY_STREAM_DESCR     ("StreamDescriptionEnabled");
const QLatin1StringView REGISTRY_STREAM_METADATA  ("StreamMetaDataEnabled");
const QLatin1StringView REGISTRY_STREAM_COMMENT   ("StreamCommentEnabled");
const QLatin1StringView REGISTRY_STREAM_SHORTCUT  ("StreamShortcutEnabled");

// Tab Network
const QLatin1StringView REGISTRY_MAX_SIMULTANEOUS ("MaxSimultaneous");
const QLatin1StringView REGISTRY_CUSTOM_BATCH     ("CustomBatchEnabled");
const QLatin1StringView REGISTRY_CUSTOM_BATCH_BL  ("CustomBatchButtonLabel");
const QLatin1StringView REGISTRY_CUSTOM_BATCH_RGE ("CustomBatchRange");
const QLatin1StringView REGISTRY_STREAM_HOST      ("StreamHostEnabled");
const QLatin1StringView REGISTRY_STREAM_HOST_LIST ("StreamHosts");

// Tab Privacy
const QLatin1StringView REGISTRY_REMOVE_COMPLETED ("PrivacyRemoveCompleted");
const QLatin1StringView REGISTRY_REMOVE_CANCELED  ("PrivacyRemoveCanceled");
const QLatin1StringView REGISTRY_REMOVE_PAUSED    ("PrivacyRemovePaused");
const QLatin1StringView REGISTRY_DATABASE         ("Database");
const QLatin1StringView REGISTRY_HTTP_USER_AGENT  ("HttpUserAgent");
const QLatin1StringView REGISTRY_HTTP_REFERRER_ON ("HttpReferringPageEnabled");
const QLatin1StringView REGISTRY_HTTP_REFERRER    ("HttpReferringPage");

// Tab Filters
const QLatin1StringView REGISTRY_FILTER_KEY       ("FilterKey");
const QLatin1StringView REGISTRY_FILTER_NAME      ("FilterName");
const QLatin1StringView REGISTRY_FILTER_VALUE     ("FilterValue");

// Tab Torrent
const QLatin1StringView REGISTRY_TORRENT_ENABLED  ("TorrentEnabled");
const QLatin1StringView REGISTRY_TORRENT_SHARED   ("TorrentShareFolderEnabled");
const QLatin1StringView REGISTRY_TORRENT_DIR      ("TorrentShareFolder");
const QLatin1StringView REGISTRY_TORRENT_PEERS    ("TorrentPeerList");
const QLatin1StringView REGISTRY_TORRENT_ADVANCED ("TorrentAdvanced");

// Tab Advanced
const QLatin1StringView REGISTRY_CHECK_UPDATE     ("CheckUpdate");


#if defined Q_OS_WIN
const QLatin1StringView C_PROGRAM_NAME("yt-dlp.exe");
#else
const QLatin1StringView C_PROGRAM_NAME("yt-dlp");
#endif

const QLatin1StringView C_WEBSITE_URL("https://github.com/yt-dlp/yt-dlp");
const int C_EXIT_SUCCESS = 0;

const QLatin1StringView C_NONE("none");

const QLatin1StringView C_WARNING_msg_header_01("WARNING:");
const QLatin1StringView C_WARNING_msg_header_02("\\033[0;33mWARNING:\\033[0m");
const QLatin1StringView C_ERROR_msg_header_01("ERROR:");
const QLatin1StringView C_ERROR_msg_header_02("\\033[0;31mERROR:\\033[0m");

const QLatin1StringView C_WARNING_merge_output_format("Requested formats are incompatible for merge and will be merged into mkv.");

const QLatin1StringView C_DOWNLOAD_msg_header("[download]");
const QLatin1StringView C_DOWNLOAD_next_section("Destination:");
const QLatin1StringView C_MERGER_msg_header("[Merger]");


const QLatin1StringView THEME_DEFAULT("default");
const QLatin1StringView THEME_FLAT("flat");
const QLatin1StringView SCHEME_LIGHT("light");
const QLatin1StringView SCHEME_DARK("dark");


const QLatin1StringView CAPTION_DEFAULT("default");
const QLatin1StringView CAPTION_AUTOMATIC("automatic");

#endif // CONSTANTS_H
