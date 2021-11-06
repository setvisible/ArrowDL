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

#include "stream.h"

#include <Core/Format>

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QChar>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonParseError>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QMap>
#include <QtCore/QtMath>
#include <QtCore/QRegularExpression>
#include <QtCore/QUrl>
#ifdef QT_TESTLIB_LIB
#  include <QtTest/QTest>
#endif

#include <algorithm> /* std::sort */

#if defined Q_OS_WIN
static const QString C_PROGRAM_NAME  = QLatin1String("yt-dlp.exe");
#else
static const QString C_PROGRAM_NAME  = QLatin1String("yt-dlp");
#endif

static const QString C_WEBSITE_URL   = QLatin1String("https://github.com/yt-dlp/yt-dlp");
static const int     C_EXIT_SUCCESS  = 0;

/*
 * This list of legal characters for filenames is limited to avoid injections
 * of special or invisible characters that could be not supported by the OS.
 */
static const QString C_LEGAL_CHARS   = QLatin1String("-+' @()[]{}°#,.&");

static const QString C_NONE          = QLatin1String("none");

static const QString C_WARNING_msg_header_01 = QLatin1String("WARNING:");
static const QString C_WARNING_msg_header_02 = QLatin1String("\\033[0;33mWARNING:\\033[0m");
static const QString C_ERROR_msg_header_01 = QLatin1String("ERROR:");
static const QString C_ERROR_msg_header_02 = QLatin1String("\\033[0;31mERROR:\\033[0m");

static const QString C_WARNING_merge_output_format = QLatin1String(
            "Requested formats are incompatible for merge and will be merged into mkv.");

static const QString C_DOWNLOAD_msg_header = QLatin1String("[download]");
static const QString C_DOWNLOAD_next_section = QLatin1String("Destination:");


static QString s_youtubedl_version = QString();
static bool s_youtubedl_last_modified_time_enabled = true;
static QString s_youtubedl_user_agent = QString();
static int s_youtubedl_socket_type = 0;
static int s_youtubedl_socket_timeout = 0;

static void debug(QObject *sender, QProcess::ProcessError error);
static QString generateErrorMessage(QProcess::ProcessError error);
static QString toString(QProcess *process);

static void debugPrintProcessCommand(QProcess *process);

static QString standardToString(const QByteArray &ba)
{
    return QString::fromLatin1(ba).simplified();
}


Stream::Stream(QObject *parent) : QObject(parent)
  , m_process(new QProcess(this))
{
    connect(m_process, SIGNAL(started()), this, SLOT(onStarted()));
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
#endif
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(onStandardOutputReady()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(onStandardErrorReady()));

    clear();
}

Stream::~Stream()
{
    m_process->kill();
    m_process->deleteLater();
}

/******************************************************************************
 ******************************************************************************/
QString Stream::version()
{
    if (s_youtubedl_version.isEmpty()) {
        auto arguments = QStringList()
                << QLatin1String("--no-colors")
                << QLatin1String("--version");
        QProcess process;
        process.setWorkingDirectory(qApp->applicationDirPath());
        process.start(C_PROGRAM_NAME, arguments);
        if (!process.waitForStarted()) {
            return QLatin1String("unknown");
        }
        if (!process.waitForFinished()) {
            return QLatin1String("unknown");
        }
        const QString result = process.readAll();
        s_youtubedl_version = result.simplified();
    }
    return s_youtubedl_version;
}

QString Stream::website()
{
    return C_WEBSITE_URL;
}

void Stream::setLastModifiedTimeEnabled(bool enabled)
{
    s_youtubedl_last_modified_time_enabled = enabled;
}

void Stream::setUserAgent(const QString &userAgent)
{
    s_youtubedl_user_agent = userAgent;
}

void Stream::setConnectionProtocol(int index)
{
    s_youtubedl_socket_type = index;
}

void Stream::setConnectionTimeout(int secs)
{
    s_youtubedl_socket_timeout = secs ? secs > 0 : 0;
}

/******************************************************************************
 ******************************************************************************/
void AskStreamVersionThread::stop()
{
    stopped = true;
}

void AskStreamVersionThread::run()
{
    // Stream::version() is blocking and time expensive
    QString result = Stream::version();
    if (!stopped) {
        emit resultReady(result);
    }
}

/******************************************************************************
 ******************************************************************************/
static inline bool matches(const QString &host, const QString &regexHost)
{
    /*
     * matches("www.absnews.com", "absnews:videos");    // == false
     * matches("www.absnews.com", "absnews.com");       // == true
     * matches("videos.absnews.com", "absnews:videos"); // == true
     * matches("videos.absnews.com", "absnews.com:videos"); // == true
     */
    static QRegularExpression delimiters("[.|:]");

    auto domains = host.split('.', QString::SkipEmptyParts);
    auto mandatoryDomains = regexHost.split(delimiters, QString::SkipEmptyParts);

    foreach (auto mandatory, mandatoryDomains) {
        bool found = false;
        foreach (auto domain, domains) {
            if (QString::compare(domain, mandatory, Qt::CaseInsensitive) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

bool Stream::matchesHost(const QString &host, const QStringList &regexHosts)
{
    foreach (auto regexHost, regexHosts) {
        if (matches(host, regexHost)) {
            return true;
        }
    }
    return false;
}

/******************************************************************************
 ******************************************************************************/
void Stream::clear()
{
    m_url.clear();
    m_outputPath.clear();
    m_selectedFormatId = StreamFormatId();
    m_bytesReceived = 0;
    m_bytesReceivedCurrentSection = 0;
    m_bytesTotal = 0;
    m_bytesTotalCurrentSection = 0;

    m_fileBaseName.clear();
    m_fileExtension.clear();
}

bool Stream::isEmpty()
{
    return m_selectedFormatId.isEmpty();
}

/******************************************************************************
 ******************************************************************************/
void Stream::initialize(const StreamObject &streamObject)
{
    m_selectedFormatId = streamObject.formatId();
    m_bytesReceived = 0;
    m_bytesReceivedCurrentSection = 0;
    m_bytesTotal = 0;
    m_bytesTotalCurrentSection = streamObject.guestimateFullSize();
    m_fileBaseName = streamObject.fileBaseName();
    m_fileExtension = streamObject.suffix();
}

/******************************************************************************
 ******************************************************************************/
QString Stream::url() const
{
    return m_url;
}

void Stream::setUrl(const QString &url)
{
    m_url = url;
}

/******************************************************************************
 ******************************************************************************/
QString Stream::localFullOutputPath() const
{
    return m_outputPath;
}

void Stream::setLocalFullOutputPath(const QString &outputPath)
{
    m_outputPath = outputPath;
}

/******************************************************************************
 ******************************************************************************/
QString Stream::referringPage() const
{
    return m_referringPage;
}

void Stream::setReferringPage(const QString &referringPage)
{
    m_referringPage = referringPage;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \remark The first format must contain the video.
 * If the video is 299 and the audio is 251,
 * then pass "299+251", not "251+299".
 *
 */
StreamFormatId Stream::selectedFormatId() const
{
    return m_selectedFormatId;
}

void Stream::setSelectedFormatId(const StreamFormatId &formatId)
{
    m_selectedFormatId = formatId;
}

/******************************************************************************
 ******************************************************************************/
qint64 Stream::fileSizeInBytes() const
{
    return _q_bytesTotal();
}

void Stream::setFileSizeInBytes(qint64 fileSizeInBytes)
{
    m_bytesTotal = fileSizeInBytes;
}

/******************************************************************************
 ******************************************************************************/
StreamObjectConfig Stream::config() const
{
    return m_config;
}

void Stream::setConfig(const StreamObjectConfig &config)
{
    m_config = config;
}

/******************************************************************************
 ******************************************************************************/
QString Stream::fileName() const
{
    if (m_fileExtension.isEmpty()) {
        return m_fileBaseName;
    }
    return QString("%0.%1").arg(m_fileBaseName, m_fileExtension);
}

/******************************************************************************
 ******************************************************************************/
QStringList Stream::arguments() const
{
    QStringList arguments;
    arguments << m_url;

    // Alphabetic order
    arguments << QLatin1String("--ignore-config");
    arguments << QLatin1String("--ignore-errors");
    arguments << QLatin1String("--no-cache-dir");
    arguments << QLatin1String("--no-colors"); // BUGFIX '--no-color' for youtube-dl
    arguments << QLatin1String("--no-check-certificate");
    arguments << QLatin1String("--no-overwrites");  /// \todo only if "overwrite" user-setting is unset
    arguments << QLatin1String("--no-continue");
    arguments << QLatin1String("--no-part"); // No .part file: write directly into output file
    arguments << QLatin1String("--no-playlist"); // No need to download playlist
    // arguments << QLatin1String("--prefer-insecure");
    arguments << QLatin1String("--restrict-filenames"); // ASCII filename only

    if (m_config.overview.skipVideo) {
        arguments << QLatin1String("--skip-download");
    }
    if (m_config.overview.markWatched) {
        arguments << QLatin1String("--mark-watched");
    }
    if (m_config.subtitle.writeDefaultSubtitle) {
        arguments << QLatin1String("--write-subs");
    }
    if (m_config.thumbnail.writeDefaultThumbnail) {
        arguments << QLatin1String("--write-thumbnail");
    }
    if (m_config.comment.writeComment) {
        arguments << QLatin1String("--write-comments");
    }
    if (m_config.metadata.writeDescription) {
        arguments << QLatin1String("--write-description");
    }
    if (m_config.metadata.writeMetadata) {
        arguments << QLatin1String("--write-info-json");
    }
    if (m_config.metadata.writeInternetShortcut) {
        arguments << QLatin1String("--write-link");
    }

    arguments << QLatin1String("--format") << m_selectedFormatId.toString();

    /* Global settings */
    if (!s_youtubedl_last_modified_time_enabled) {
        arguments << QLatin1String("--no-mtime");
    }
    if (!s_youtubedl_user_agent.isEmpty()) {
        // --user-agent option requires non-empty argument
        arguments << QLatin1String("--user-agent") << s_youtubedl_user_agent;
    }
    if (s_youtubedl_socket_timeout > 0) {
        arguments << QLatin1String("--socket-timeout") << QString::number(s_youtubedl_socket_timeout);
    }
    switch (s_youtubedl_socket_type) {
    case 1: arguments << QLatin1String("--force-ipv4"); break;
    case 2: arguments << QLatin1String("--force-ipv6"); break;
    default:
        break;
    }
    if (!m_referringPage.isEmpty()) {
        arguments << QLatin1String("--referer") << m_referringPage;
    }
    if (isMergeFormat(m_fileExtension)) {
        arguments << QLatin1String("--merge-output-format") << m_fileExtension;
    }

    arguments << QLatin1String("--output") << m_outputPath;
    return arguments;
}

QString Stream::command(int indent) const
{
    auto args = arguments();

    // Inline command
    QString cmd = C_PROGRAM_NAME + " ";
    foreach (auto argument, args) {
        auto quote = argument.contains(' ') ? QString('\"') : QString();
        cmd += quote + argument + quote + " ";
    }
    cmd = cmd.trimmed();
    cmd += "\n\n";

    // Smartly wrapped arguments
    cmd += C_PROGRAM_NAME + " ";
    foreach (auto argument, args) {
        if (argument.startsWith("--")) {
            cmd += "\\\n" + QString().fill(' ', indent);
        }
        auto quote = argument.contains(' ') ? QString('\"') : QString();
        cmd += quote + argument + quote + " ";
    }
    cmd = cmd.trimmed();
    cmd += "\n";
    return cmd;
}

/******************************************************************************
 ******************************************************************************/
void Stream::start()
{
    if (!isEmpty() && m_process->state() == QProcess::NotRunning) {
        // Usage: yt-dlp.exe [OPTIONS] URL [URL...]
        m_process->setWorkingDirectory(qApp->applicationDirPath());
        m_process->start(C_PROGRAM_NAME, arguments());
        debugPrintProcessCommand(m_process);
    }
}

void Stream::abort()
{
    m_process->kill();
    emit downloadFinished();
}

/******************************************************************************
 ******************************************************************************/
void Stream::onStarted()
{
}

void Stream::onError(QProcess::ProcessError error)
{
    // Issue with the process configuration, or argument,
    // or invalid path to "yt-dlp" program,
    // but not due to an invalid user input like invalid URL.
    debug(sender(), error);
}

void Stream::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // qDebug() << Q_FUNC_INFO << exitCode << exitStatus;
    if (exitStatus == QProcess::NormalExit) {
        if (exitCode == C_EXIT_SUCCESS) {
            emit downloadProgress(_q_bytesTotal(), _q_bytesTotal());
            emit downloadFinished();
        } else {
            auto errorMessage = standardToString(m_process->readAllStandardError());
            emit downloadError(errorMessage);
        }
    } else {
        emit downloadError(tr("The process crashed."));
    }
}

void Stream::onStandardOutputReady()
{
    parseStandardOutput(standardToString(m_process->readAllStandardOutput()));
}

void Stream::onStandardErrorReady()
{
    parseStandardError(standardToString(m_process->readAllStandardError()));
}

/******************************************************************************
 ******************************************************************************/
void Stream::parseStandardOutput(const QString &data)
{
    auto tokens = data.split(QChar::Space, QString::SkipEmptyParts);
    if (tokens.isEmpty()) {
        return;
    }
    if (tokens.at(0).toLower() != C_DOWNLOAD_msg_header) {
        return;
    }
    if ( tokens.count() > 2 &&
         tokens.at(1) == C_DOWNLOAD_next_section) {
        m_bytesReceived += m_bytesReceivedCurrentSection;
        emit downloadProgress(m_bytesReceived, _q_bytesTotal());
        return;
    }

    if ( tokens.count() > 3 &&
         tokens.at(1).contains(QChar('%')) &&
         tokens.at(2) == QLatin1String("of")) {

        auto percentToken = tokens.at(1);
        auto sizeToken = tokens.at(3);

        auto percent = Format::parsePercentDecimal(percentToken);
        if (percent < 0) {
            qWarning("Can't parse '%s'.", percentToken.toLatin1().data());
            return;
        }

        m_bytesTotalCurrentSection = Format::parseBytes(sizeToken);
        if (m_bytesTotalCurrentSection < 0) {
            qWarning("Can't parse '%s'.", sizeToken.toLatin1().data());
            return;
        }
        m_bytesReceivedCurrentSection = qCeil((percent * m_bytesTotalCurrentSection) / 100.0);
    }

    auto received = m_bytesReceived + m_bytesReceivedCurrentSection;
    emit downloadProgress(received, _q_bytesTotal());
}

void Stream::parseStandardError(const QString &data)
{
    // qDebug() << Q_FUNC_INFO << data;

    if ( data.startsWith(C_ERROR_msg_header_01, Qt::CaseInsensitive) ||
         data.startsWith(C_ERROR_msg_header_02, Qt::CaseInsensitive)) {

        emit downloadError(data);

    } else if ( data.startsWith(C_WARNING_msg_header_01, Qt::CaseInsensitive) ||
                data.startsWith(C_WARNING_msg_header_02, Qt::CaseInsensitive)) {

        if (data.contains(C_WARNING_merge_output_format, Qt::CaseInsensitive)) {

            /// \todo change extension ?
            /// \todo propose: mkv, mp4, ogg, webm, flv (= merge formats if merge is required)
            m_fileExtension = QLatin1String("mkv");
            emit downloadMetadataChanged();
        }
    }
}

qint64 Stream::_q_bytesTotal() const
{
    return m_bytesTotal > 0 ? m_bytesTotal : m_bytesTotalCurrentSection;
}

bool Stream::isMergeFormat(const QString &suffix) const
{
    static QStringList validFormat = { // See option --merge-output-format FORMAT
                                       QLatin1String("mkv"),
                                       QLatin1String("mp4"),
                                       QLatin1String("ogg"),
                                       QLatin1String("webm"),
                                       QLatin1String("flv")
                                     };
    return validFormat.contains(suffix.toLower());
}

/******************************************************************************
 ******************************************************************************/
StreamCleanCache::StreamCleanCache(QObject *parent) : QObject(parent)
  , m_process(new QProcess(this))
  , m_isCleaned(false)
{
    connect(m_process, SIGNAL(started()), this, SLOT(onStarted()));
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
#endif
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onFinished(int, QProcess::ExitStatus)));
}

StreamCleanCache::~StreamCleanCache()
{
    m_process->kill();
    m_process->deleteLater();
}

void StreamCleanCache::runAsync()
{
    if (m_process->state() == QProcess::NotRunning) {
        auto arguments = QStringList()
                << QLatin1String("--no-colors")
                << QLatin1String("--rm-cache-dir");
        m_process->setWorkingDirectory(qApp->applicationDirPath());
        m_process->start(C_PROGRAM_NAME, arguments);
        debugPrintProcessCommand(m_process);
    }
}

QUrl StreamCleanCache::cacheDir()
{
    // Try to get the .cache from $XDG_CACHE_HOME, if it's not set,
    // it has to be in ~/.cache as per XDG standard
    QString dir = QString::fromUtf8(getenv("XDG_CACHE_HOME"));
    if (dir.isEmpty()) {
        dir = QDir::cleanPath(QDir::homePath() + QLatin1String("/.cache"));
    }
    return QUrl::fromLocalFile(dir);
}

bool StreamCleanCache::isCleaned() const
{
    return m_isCleaned;
}

void StreamCleanCache::onStarted()
{
}

void StreamCleanCache::onError(QProcess::ProcessError error)
{
    debug(sender(), error);
}

void StreamCleanCache::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        if (exitCode == C_EXIT_SUCCESS) {
            qInfo("Cache cleaned.");
        } else {
            qWarning("Can't clean the cache.");
        }
    } else {
        qWarning("The process (YT-DLP) has crashed.");
    }
    // Even if crashed or not cleaned,
    // the process is set to done, to avoid being redone.
    m_isCleaned = true;
    emit done();
}

/******************************************************************************
 ******************************************************************************/
StreamObjectDownloader::StreamObjectDownloader(QObject *parent) : QObject(parent)
  , m_processDumpJson(new QProcess(this))
  , m_processFlatList(new QProcess(this))
  , m_streamCleanCache(new StreamCleanCache(this))
  , m_url(QString())
  , m_cancelled(false)
{
    connect(m_processDumpJson, SIGNAL(started()), this, SLOT(onStarted()));
    connect(m_processFlatList, SIGNAL(started()), this, SLOT(onStarted()));
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    connect(m_processDumpJson, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
    connect(m_processFlatList, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
#endif
    connect(m_processDumpJson, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onFinishedDumpJson(int, QProcess::ExitStatus)));
    connect(m_processFlatList, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onFinishedFlatList(int, QProcess::ExitStatus)));

    // Cache cleaner
    connect(m_streamCleanCache, SIGNAL(done()), this, SLOT(onCacheCleaned()));
}


StreamObjectDownloader::~StreamObjectDownloader()
{
    m_processDumpJson->kill();
    m_processDumpJson->deleteLater();
    m_processFlatList->kill();
    m_processFlatList->deleteLater();
}

void StreamObjectDownloader::runAsync(const QString &url)
{
    /*
     * We run 2 processes (--dump-json and --flat-playlist) in parallel
     * in order to optimize time:
     * --dump-json     : gets the JSON data of each stream
     * --flat-playlist : gets the ordered playlist
     */
    m_url = url;
    m_cancelled = false;
    m_dumpMap.clear();
    m_flatList.clear();

    runAsyncDumpJson();
    runAsyncFlatList();
}

void StreamObjectDownloader::runAsyncDumpJson()
{
    if (m_processDumpJson->state() == QProcess::NotRunning) {
        auto arguments = QStringList()
                << QLatin1String("--dump-json")
                << QLatin1String("--yes-playlist")
                << QLatin1String("--no-colors")
                << QLatin1String("--no-check-certificate")
                << QLatin1String("--ignore-config")
                << QLatin1String("--ignore-errors") // skip errors, like unavailable videos in a playlist
                << m_url;
        if (!s_youtubedl_user_agent.isEmpty()) {
            // --user-agent option requires non-empty argument
            arguments << QLatin1String("--user-agent") << s_youtubedl_user_agent;
        }
        m_processDumpJson->setWorkingDirectory(qApp->applicationDirPath());
        m_processDumpJson->start(C_PROGRAM_NAME, arguments);
        debugPrintProcessCommand(m_processDumpJson);
    }
}

void StreamObjectDownloader::runAsyncFlatList()
{
    if (m_processFlatList->state() == QProcess::NotRunning) {
        auto arguments = QStringList()
                << QLatin1String("--dump-json")
                << QLatin1String("--flat-playlist")
                << QLatin1String("--yes-playlist")
                << QLatin1String("--no-colors")
                << QLatin1String("--no-check-certificate")
                << QLatin1String("--ignore-config")
                << QLatin1String("--ignore-errors")
                << m_url;
        if (!s_youtubedl_user_agent.isEmpty()) {
            // --user-agent option requires non-empty argument
            arguments << QLatin1String("--user-agent") << s_youtubedl_user_agent;
        }
        m_processFlatList->setWorkingDirectory(qApp->applicationDirPath());
        m_processFlatList->start(C_PROGRAM_NAME, arguments);
        debugPrintProcessCommand(m_processFlatList);
    }
}

void StreamObjectDownloader::stop()
{
    if (m_processDumpJson->state() != QProcess::NotRunning) {
        m_processDumpJson->kill();
    }
    if (m_processFlatList->state() != QProcess::NotRunning) {
        m_processFlatList->kill();
    }
    m_dumpMap.clear();
    m_flatList.clear();
    m_cancelled = true;
}

bool StreamObjectDownloader::isRunning() const
{
    return !( m_processDumpJson->state() == QProcess::NotRunning &&
              m_processFlatList->state() == QProcess::NotRunning);
}

void StreamObjectDownloader::onStarted()
{
}

void StreamObjectDownloader::onError(QProcess::ProcessError error)
{
    debug(sender(), error);
    m_dumpMap.clear();
    m_flatList.clear();
    /// \todo verify race condition
}

void StreamObjectDownloader::onFinishedDumpJson(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {

        /*
         * With --ignore-errors, the attributes of unavailable videos
         * in a playlist are communicated through the StandardError
         * whilst available streams are through the StandardOutput.
         */
        const QByteArray stdoutBytes = m_processDumpJson->readAllStandardOutput();
        const QByteArray stderrBytes = m_processDumpJson->readAllStandardError();

        /*
         * If StandardOutput or StandardError contains bytes,
         * but YDL doesn't return SUCCESS code,
         * it might mean that the problem comes from the server,
         * i.e. some videos in the playlist are not available.
         *
         * We parse the standard streams
         * and retry only if *ALL* the videos are missing.
         */
        m_dumpMap = parseDumpMap(stdoutBytes, stderrBytes);

        if (exitCode == C_EXIT_SUCCESS) {
            /*
             * Doing nothing here
             */
        } else {
            /*
             * We only retry if the first-try data is not a playlist.
             * Indeed, playlists might contain errors for some child streams,
             * so the flow comes here, but we skip this retry, because
             * metadata of long playlists takes time to be downloaded. And we
             * don't want to take this time twice.
             */
            const bool isPlaylist = m_dumpMap.count() > 1;
            if (!m_streamCleanCache->isCleaned() && !isPlaylist) {
                stop();
                m_streamCleanCache->runAsync(); // Clean cache and retry
                return;
            }
        }
        if (!m_dumpMap.isEmpty()) {
            onFinished();
        } else {
            emit error(tr("Couldn't parse JSON file."));
        }
    } else {
        emit error(tr("The process crashed."));
    }
}

void StreamObjectDownloader::onFinishedFlatList(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        if (exitCode == C_EXIT_SUCCESS) {

            /// \todo show playlist ASAP, right after --flat-playlist finishes instead of waiting for --dump-json finishes too?

            const QByteArray stdoutBytes = m_processFlatList->readAllStandardOutput();
            const QByteArray stderrBytes = m_processFlatList->readAllStandardError();
            m_flatList = parseFlatList(stdoutBytes, stderrBytes);
            if (!m_flatList.isEmpty()) {
                onFinished();
            } else {
                emit error(tr("Couldn't parse playlist (no data received)."));
            }
        } else {
            emit error(tr("Couldn't parse playlist (ill-formed JSON file)."));
        }
    } else {
        emit error(tr("The process crashed."));
    }
}

StreamDumpMap StreamObjectDownloader::parseDumpMap(const QByteArray &stdoutBytes,
                                                   const QByteArray &stderrBytes)
{
    StreamDumpMap map;
    QList<QByteArray> stdoutLines = stdoutBytes.split(QChar('\n').toLatin1());
    foreach (auto stdoutLine, stdoutLines) {
        if (!stdoutLine.isEmpty()) {
            StreamObject streamObject = parseDumpItemStdOut(stdoutLine);
            map.insert(streamObject.id, streamObject);

        }
    }
    QList<QByteArray> stderrLines = stderrBytes.split(QChar('\n').toLatin1());
    foreach (auto stderrLine, stderrLines) {
        if (!stderrLine.isEmpty()) {
            StreamObject streamObject = parseDumpItemStdErr(stderrLine);
            map.insert(streamObject.id, streamObject);
        }
    }
    return map;
}

StreamObject StreamObjectDownloader::parseDumpItemStdOut(const QByteArray &data)
{
    StreamObject obj;
    QJsonParseError ok{};
    QJsonDocument loadDoc(QJsonDocument::fromJson(data, &ok));
    if (ok.error != QJsonParseError::NoError) {
        obj.setError(StreamObject::ErrorJsonFormat);
        return obj;
    }
    QJsonObject json = loadDoc.object();
    obj.id                 = json[QLatin1String("id")].toString();
    obj._filename          = json[QLatin1String("_filename")].toString();
    obj.webpage_url        = json[QLatin1String("webpage_url")].toString();
    obj.fulltitle          = json[QLatin1String("fulltitle")].toString();
    obj.defaultTitle       = json[QLatin1String("title")].toString();
    obj.defaultSuffix      = json[QLatin1String("ext")].toString();
    obj.description        = json[QLatin1String("description")].toString();
    obj.thumbnail          = json[QLatin1String("thumbnail")].toString();
    obj.extractor          = json[QLatin1String("extractor")].toString();
    obj.extractor_key      = json[QLatin1String("extractor_key")].toString();
    obj.defaultFormatId    = StreamFormatId(json[QLatin1String("format_id")].toString());
    QJsonArray fmts        = json[QLatin1String("formats")].toArray();
    foreach (auto fmt, fmts) {
        QJsonObject fmtObject = fmt.toObject();
        StreamFormat format;
        format.formatId     = StreamFormatId(fmtObject[QLatin1String("format_id")].toString());
        format.ext          = fmtObject[QLatin1String("ext")].toString();
        format.format_note  = fmtObject[QLatin1String("format_note")].toString();
        format.filesize     = fmtObject[QLatin1String("filesize")].toInt();
        format.acodec       = fmtObject[QLatin1String("acodec")].toString();
        format.abr          = fmtObject[QLatin1String("abr")].toInt();
        format.asr          = fmtObject[QLatin1String("asr")].toInt();
        format.vcodec       = fmtObject[QLatin1String("vcodec")].toString();
        format.width        = fmtObject[QLatin1String("width")].toInt();
        format.height       = fmtObject[QLatin1String("height")].toInt();
        format.fps          = fmtObject[QLatin1String("fps")].toInt();
        format.tbr          = fmtObject[QLatin1String("tbr")].toInt();
        obj.formats << format;
    }
    obj.playlist           = json[QLatin1String("playlist")].toString();
    obj.playlist_index     = json[QLatin1String("playlist_index")].toString();
    obj.setError(StreamObject::NoError);
    return obj;
}

static StreamObjectId findId(const QByteArray &data)
{
    /*
     * "ERROR: 0123456789a: YouTube said: Unable to extract video data"
     * should return "0123456789a"
     */
    QString str = QString::fromLatin1(data);
    QStringList values = str.split(QLatin1String(":"), QString::SkipEmptyParts);
    return values.count() > 1 ? values.at(1).trimmed() : StreamObjectId();
}

StreamObject StreamObjectDownloader::parseDumpItemStdErr(const QByteArray &data)
{
    StreamObject ret;
    ret.id = findId(data);
    ret.setError(StreamObject::ErrorUnavailable);
    return ret;
}

StreamFlatList StreamObjectDownloader::parseFlatList(const QByteArray &stdoutBytes,
                                                     const QByteArray &stderrBytes)
{
    QList<StreamFlatListItem> list;
    QList<QByteArray> stdoutLines = stdoutBytes.split(QChar('\n').toLatin1());
    foreach (auto stdoutLine, stdoutLines) {
        if (!stdoutLine.isEmpty()) {
            StreamFlatListItem item = parseFlatItem(stdoutLine);
            if (!item.id.isEmpty()) {
                list << item;
            }
        }
    }
    QList<QByteArray> stderrLines = stderrBytes.split(QChar('\n').toLatin1());
    foreach (auto stderrLine, stderrLines) {
        if (!stderrLine.isEmpty()) {
            qWarning("Stream error: '%s'.", stderrLine.data());
        }
    }
    return list;
}

StreamFlatListItem StreamObjectDownloader::parseFlatItem(const QByteArray &data)
{
    StreamFlatListItem item;
    QJsonParseError ok{};
    QJsonDocument loadDoc(QJsonDocument::fromJson(data, &ok));
    if (ok.error == QJsonParseError::NoError) {
        QJsonObject json = loadDoc.object();
        item._type      = json[QLatin1String("_type")].toString();
        item.id         = json[QLatin1String("id")].toString();
        item.ie_key     = json[QLatin1String("ie_key")].toString();
        item.title      = json[QLatin1String("title")].toString();
        item.url        = json[QLatin1String("url")].toString();
    }
    return item;
}

void StreamObjectDownloader::onFinished()
{
    if (m_cancelled) {
        emit error(tr("Cancelled."));
        return;
    }
    if (!m_dumpMap.isEmpty() && !m_flatList.isEmpty()) {
        QList<StreamObject> streamObjects;
        int playlist_index = 0;
        foreach (auto flatItem, m_flatList) {
            playlist_index++;
            StreamObject si = createStreamObject(flatItem);
            si.playlist_index = QString::number(playlist_index);
            streamObjects << si;
        }
        // Some videos might have errors or not available, but it's ok.
        emit collected(streamObjects);
    }
}

StreamObject StreamObjectDownloader::createStreamObject(const StreamFlatListItem &flatItem) const
{
    StreamObject si;
    if (!flatItem.id.isEmpty() && m_dumpMap.contains(flatItem.id)) {
        si = m_dumpMap.value(flatItem.id);
    } else {
        /// \todo replace with ErrorGeoRestriction instead?
        si.setError(StreamObject::ErrorUnavailable);
    }
    if (si.defaultTitle.isEmpty()) {
        si.defaultTitle = flatItem.title;
    }
    if (si.webpage_url.isEmpty()) {
        si.webpage_url = flatItem.url; /// \todo fix incomplete URL
    }
    return si;
}

void StreamObjectDownloader::onCacheCleaned()
{
    runAsync(m_url); // retry
}

/******************************************************************************
 ******************************************************************************/
StreamUpgrader::StreamUpgrader(QObject *parent) : QObject(parent)
  , m_process(new QProcess(this))
{
    connect(m_process, SIGNAL(started()), this, SLOT(onStarted()));
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
#endif
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(onStandardOutputReady()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(onStandardErrorReady()));
}

StreamUpgrader::~StreamUpgrader()
{
    m_process->kill();
    m_process->deleteLater();
}

void StreamUpgrader::runAsync()
{
    if (m_process->state() == QProcess::NotRunning) {
        auto arguments = QStringList()
                << QLatin1String("--no-colors")
                << QLatin1String("--update");
        m_process->setWorkingDirectory(qApp->applicationDirPath());
        m_process->start(C_PROGRAM_NAME, arguments);
        debugPrintProcessCommand(m_process);
    }
}

void StreamUpgrader::onStarted()
{
}

void StreamUpgrader::onError(QProcess::ProcessError error)
{
    debug(sender(), error);
}

void StreamUpgrader::onStandardOutputReady()
{
    qDebug() << standardToString(m_process->readAllStandardOutput());
}

void StreamUpgrader::onStandardErrorReady()
{
    qDebug() << "Error:" << standardToString(m_process->readAllStandardError());
}

void StreamUpgrader::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        if (exitCode == C_EXIT_SUCCESS) {
            qInfo("Upgraded YT-DLP.");
        } else {
            qWarning("Can't upgrade YT-DLP.");
        }
    } else {
        qWarning("The process (YT-DLP) has crashed.");
    }
    emit done();
}

/******************************************************************************
 ******************************************************************************/
StreamExtractorListCollector::StreamExtractorListCollector(QObject *parent) : QObject(parent)
  , m_processExtractors(new QProcess(this))
  , m_processDescriptions(new QProcess(this))
{
    connect(m_processExtractors, SIGNAL(started()), this, SLOT(onStarted()));
    connect(m_processDescriptions, SIGNAL(started()), this, SLOT(onStarted()));
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    connect(m_processExtractors, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
    connect(m_processDescriptions, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onError(QProcess::ProcessError)));
#endif
    connect(m_processExtractors, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onFinishedExtractors(int, QProcess::ExitStatus)));
    connect(m_processDescriptions, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onFinishedDescriptions(int, QProcess::ExitStatus)));
}

StreamExtractorListCollector::~StreamExtractorListCollector()
{
    m_processExtractors->kill();
    m_processExtractors->deleteLater();
    m_processDescriptions->kill();
    m_processDescriptions->deleteLater();
}

void StreamExtractorListCollector::runAsync()
{
    if (m_processExtractors->state() == QProcess::NotRunning) {
        auto arguments = QStringList()
                << QLatin1String("--no-colors")
                << QLatin1String("--list-extractors");
        m_processExtractors->setWorkingDirectory(qApp->applicationDirPath());
        m_processExtractors->start(C_PROGRAM_NAME,arguments);
        debugPrintProcessCommand(m_processExtractors);
    }
    if (m_processDescriptions->state() == QProcess::NotRunning) {
        auto arguments = QStringList()
                << QLatin1String("--no-colors")
                << QLatin1String("--extractor-descriptions");
        m_processDescriptions->setWorkingDirectory(qApp->applicationDirPath());
        m_processDescriptions->start(C_PROGRAM_NAME,arguments);
        debugPrintProcessCommand(m_processDescriptions);
    }
}

void StreamExtractorListCollector::onStarted()
{
}

void StreamExtractorListCollector::onError(QProcess::ProcessError error)
{
    debug(sender(), error);
    m_extractors.clear();
    m_descriptions.clear();
    /// \todo verify race condition
}

void StreamExtractorListCollector::onFinishedExtractors(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        if (exitCode == C_EXIT_SUCCESS) {
            QString data = QString::fromLatin1(m_processExtractors->readAllStandardOutput());
            m_extractors = data.split(QChar('\n'), QString::KeepEmptyParts);
            onFinished();
        } else {
            auto msg = standardToString(m_processExtractors->readAllStandardError());
            emit error(msg);
            emit finished();
        }
    } else {
        emit error(tr("The process crashed."));
        emit finished();
    }
}

void StreamExtractorListCollector::onFinishedDescriptions(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        if (exitCode == C_EXIT_SUCCESS) {
            QString data = QString::fromLatin1(m_processDescriptions->readAllStandardOutput());
            m_descriptions = data.split("\n", QString::KeepEmptyParts);
            onFinished();
        } else {
            auto msg = standardToString(m_processDescriptions->readAllStandardError());
            emit error(msg);
            emit finished();
        }
    } else {
        emit error(tr("The process crashed."));
        emit finished();
    }
}

void StreamExtractorListCollector::onFinished()
{
    if (!m_extractors.isEmpty() && !m_descriptions.isEmpty()) {
        emit collected(m_extractors, m_descriptions);
        emit finished();
    }
}

/******************************************************************************
 ******************************************************************************/
StreamFormatId::StreamFormatId(const QString &format_id)
{
    fromString(format_id);
}

QString StreamFormatId::toString() const
{
    return m_identifiers.join(QChar('+'));
}

void StreamFormatId::fromString(const QString &format_id)
{
    auto split = format_id.split(QChar('+'), QString::SkipEmptyParts);
    /// \todo std::sort(split.begin(), split.end()); ?
    m_identifiers = split;
}

QList<StreamFormatId> StreamFormatId::compoundIds() const
{
    QList<StreamFormatId> ret;
    foreach (auto identifier, m_identifiers) {
        ret << StreamFormatId(identifier);
    }
    return ret;
}

bool StreamFormatId::isEmpty() const
{
    return m_identifiers.isEmpty();
}

bool StreamFormatId::operator==(const StreamFormatId &other) const
{
    return toString() == other.toString();
}

bool StreamFormatId::operator!=(const StreamFormatId &other) const
{
    return !(*this == other);
}

bool StreamFormatId::operator<(const StreamFormatId &other) const
{
    return toString() < other.toString();
}

/******************************************************************************
 ******************************************************************************/
StreamFormat::StreamFormat(const QString &format_id,
                           const QString &ext,
                           const QString &format_note,
                           int filesize,
                           const QString &acodec,
                           int abr,
                           int asr,
                           const QString &vcodec,
                           int width,
                           int height,
                           int fps,
                           int tbr)
{
    this->formatId     = StreamFormatId(format_id);
    this->ext          = ext;
    this->format_note  = format_note;
    this->filesize     = filesize;
    this->acodec       = acodec;
    this->abr          = abr;
    this->asr          = asr;
    this->vcodec       = vcodec;
    this->width        = width;
    this->height       = height;
    this->fps          = fps;
    this->tbr          = tbr;
}

bool StreamFormat::operator==(const StreamFormat &other) const
{
    return formatId         == other.formatId
            && ext          == other.ext
            && format_note  == other.format_note
            && filesize     == other.filesize
            && acodec       == other.acodec
            && abr          == other.abr
            && asr          == other.asr
            && vcodec       == other.vcodec
            && width        == other.width
            && height       == other.height
            && fps          == other.fps
            && tbr          == other.tbr;
}

bool StreamFormat::operator!=(const StreamFormat &other) const
{
    return !(*this == other);
}

bool StreamFormat::hasVideo() const {
    return vcodec != C_NONE;
}
bool StreamFormat::hasMusic() const {
    return acodec != C_NONE;
}

QString StreamFormat::toString() const
{
    if (hasVideo() && hasMusic()) {
        return QObject::tr("Video %0 x %1%2%3").arg(
                    width <= 0 ? QLatin1String("?") : QString::number(width),
                    height <= 0 ? QLatin1String("?") : QString::number(height),
                    format_note.isEmpty() ? QString() : QString(" (%0)").arg(format_note),
                    filesize <= 0 ? QString() : QString(", size: %0").arg(Format::fileSizeToString(filesize)));
    }
    if (hasVideo()) {
        return QObject::tr("[%0] %1 x %2 (%3 fps) @ %4 KBit/s, codec: %5")
                .arg(ext.toUpper())
                .arg(width)
                .arg(height)
                .arg(fps)
                .arg(tbr)
                .arg(vcodec);
    }
    if (hasMusic()) {
        return QObject::tr("[%0] %1 Hz @ %2 KBit/s, codec: %3")
                .arg(ext.toUpper())
                .arg(asr)
                .arg(abr)
                .arg(acodec);
    }
    return QString();
}

QString StreamFormat::debug_description() const
{
    return QString("StreamFormat '%0' (%1, %2, %3)").arg(
                formatId.toString(),
                QString("%0, %1, %2").arg(
                    ext,
                    format_note,
                    QString::number(filesize)),
                QString("%0, %1, %2").arg(
                    acodec,
                    QString::number(abr),
                    QString::number(asr)),
                QString("%0, %1, %2, %3, %4").arg(
                    vcodec,
                    QString::number(width),
                    QString::number(height),
                    QString::number(fps),
                    QString::number(tbr)));
}

/******************************************************************************
 ******************************************************************************/
bool StreamObject::operator==(const StreamObject &other) const
{
    return id                   == other.id
            && _filename        == other._filename
            && webpage_url      == other.webpage_url
            && fulltitle        == other.fulltitle
            && defaultTitle     == other.defaultTitle
            && defaultSuffix    == other.defaultSuffix
            && description      == other.description
            && thumbnail        == other.thumbnail
            && extractor        == other.extractor
            && extractor_key    == other.extractor_key
            && defaultFormatId  == other.defaultFormatId
            && formats          == other.formats
            && playlist         == other.playlist
            && playlist_index   == other.playlist_index
            && m_error          == other.m_error
            && m_userTitle      == other.m_userTitle
            && m_userSuffix     == other.m_userSuffix
            && m_userFormatId   == other.m_userFormatId
            && m_userConfig     == other.m_userConfig;
}

bool StreamObject::operator!=(const StreamObject &other) const
{
    return !(*this == other);
}

qint64 StreamObject::guestimateFullSize() const
{
    return guestimateFullSize(formatId());
}

qint64 StreamObject::guestimateFullSize(const StreamFormatId &formatId) const
{
    if (formatId.isEmpty()) {
        return -1;
    }
    QMap<StreamFormatId, qint64> sizes;
    for (auto format : formats) {
        sizes.insert(format.formatId, format.filesize);
    }
    qint64 estimatedSize = 0;
    for (auto id : formatId.compoundIds()) {
        estimatedSize += sizes.value(id, 0);
    }
    return estimatedSize;
}

QString StreamObject::title() const
{
    return m_userTitle.isEmpty() ? defaultTitle : m_userTitle;
}

void StreamObject::setTitle(const QString &title)
{
    m_userTitle = (title == defaultTitle) ? QString() : title;
}

StreamObjectConfig StreamObject::config() const
{
    return m_userConfig;
}

void StreamObject::setConfig(const StreamObjectConfig &config)
{
    m_userConfig = config;
}

static QString cleanFileName(const QString &fileName)
{
    QString ret = fileName.simplified();
    QString::iterator it;
    for (it = ret.begin(); it != ret.end(); ++it){
        const QChar c = (*it).unicode();
        if (c.isLetterOrNumber() || C_LEGAL_CHARS.contains(c)) {
            continue;
        }
        if (c == QChar('"')) {
            *it = QChar('\'');
        } else {
            *it = QChar('_');
        }
    }
    ret = ret.replace(QRegularExpression("_+"), QLatin1String("_"));
    return ret.simplified();
}


QString StreamObject::fullFileName() const
{
    return suffix().isEmpty()
            ? fileBaseName()
            : QString("%0.%1").arg(fileBaseName(), suffix());
}

QString StreamObject::fileBaseName() const
{
    return cleanFileName(title());
}

QString StreamObject::suffix() const
{
    return m_userSuffix.isEmpty() ? suffix(formatId()) : m_userSuffix;
}

QString StreamObject::suffix(const StreamFormatId &formatId) const
{
    if (defaultFormatId.isEmpty()) {
        return QLatin1String("???");
    }
    if (defaultFormatId == formatId) {
        return defaultSuffix;
    }
    auto suffix = defaultSuffix;
    foreach (auto id, formatId.compoundIds()) {
        foreach (auto format, formats) {
            if (id == format.formatId) {
                if (format.hasVideo()) {
                    return format.ext;
                }
                suffix = format.ext;
            }
        }
    }
    return suffix;
}

void StreamObject::setSuffix(const QString &suffix)
{
    m_userSuffix = (suffix == defaultSuffix) ? QString() : suffix;
}

StreamFormatId StreamObject::formatId() const
{
    return m_userFormatId.isEmpty() ? defaultFormatId : m_userFormatId;
}

void StreamObject::setFormatId(const StreamFormatId &formatId)
{
    m_userSuffix = QString();
    m_userFormatId = (formatId == defaultFormatId) ? StreamFormatId() : formatId;
}

QString StreamObject::formatToString() const
{
    QString ret;
    foreach (auto id, formatId().compoundIds()) {
        for (auto format : formats) {
            if (id == format.formatId) {
                if (!ret.isEmpty()) {
                    ret += QLatin1String(" ");
                }
                ret += format.toString();
            }
        }
    }
    return ret;
}

QList<StreamFormat> StreamObject::defaultFormats() const
{
    // Map avoids duplicate entries
    QMap<QString, StreamFormat> map;
    foreach (auto format, formats) {
        if (format.hasVideo() && format.hasMusic()) {

            // The output list should be sorted in ascending order of
            // video resolution, then in ascending order of codec name
            auto unique_sort_identifier = QString("%0 %1 %2")
                    .arg(format.width, 16, 10, QChar('0'))
                    .arg(format.height, 16, 10, QChar('0'))
                    .arg(format.toString());

            map.insert(unique_sort_identifier, format);
        }
    }
    return map.values();
}

QList<StreamFormat> StreamObject::audioFormats() const
{
    QList<StreamFormat> list;
    foreach (auto format, formats) {
        if (!format.hasVideo() && format.hasMusic()) {
            list.append(format);
        }
    }
    return list;
}

QList<StreamFormat> StreamObject::videoFormats() const
{
    QList<StreamFormat> list;
    foreach (auto format, formats) {
        if (format.hasVideo() && !format.hasMusic()) {
            list.append(format);
        }
    }
    return list;
}

bool StreamObject::isAvailable() const
{
    return m_error == NoError;
}

QString StreamObject::debug_description() const
{
    QString descr;
    descr.append(QString("StreamObject '%0' [%1] (%2, %3)").arg(
                     _filename,
                     webpage_url,
                     QString("%0, %1, %2, %3, %4").arg(
                         fulltitle,
                         defaultTitle,
                         defaultSuffix,
                         description,
                         thumbnail),
                     QString("%0, %1, %2, %3, %4").arg(
                         extractor,
                         extractor_key,
                         defaultFormatId.toString(),
                         playlist,
                         playlist_index)));
    foreach (auto format, formats) {
        descr.append("\n");
        descr.append(format.debug_description());
    }
    return descr;
}

StreamObject::Error StreamObject::error() const
{
    return m_error;
}

void StreamObject::setError(Error error)
{
    m_error = error;
}

/******************************************************************************
 ******************************************************************************/
static void debug(QObject *sender, QProcess::ProcessError error)
{
    auto process = qobject_cast<QProcess*>(sender);
    if (process) {
        qDebug().noquote() << toString(process);
        qDebug().noquote() << generateErrorMessage(error);
    } else {
        qDebug().noquote() << generateErrorMessage(error);
    }
}

static QString generateErrorMessage(QProcess::ProcessError error)
{
    switch(error) {
    case QProcess::FailedToStart: return QLatin1String("The process failed to start.");
    case QProcess::Crashed:       return QLatin1String("The process crashed while attempting to run.");
    case QProcess::Timedout:      return QLatin1String("The process has timed out.");
    case QProcess::WriteError:    return QLatin1String("The process has encountered a write error.");
    case QProcess::ReadError:     return QLatin1String("The process has encountered a read error.");
    case QProcess::UnknownError:  return QLatin1String("The process has encountered an unknown error.");
    default:
        Q_UNREACHABLE();
    }
    return QString();
}

static QString toString(QProcess *process)
{
    if (!process) {
        return QLatin1String("ERROR: invalid process");
    }
    return QString("[pid:%0] pwd='%1' cmd='%2 %3'").arg(
                QString::number(process->processId()),
                process->workingDirectory(),
                process->program(),
                process->arguments().join(" "));
}

/******************************************************************************
 ******************************************************************************/
#ifdef QT_TESTLIB_LIB
/// This function is used by QCOMPARE() to output verbose information in case of a test failure.
char *toString(const StreamFormat &streamFormat)
{
    // bring QTest::toString overloads into scope:
    using QTest::toString;

    // delegate char* handling to QTest::toString(QByteArray):
    return toString(streamFormat.debug_description());
}

char *toString(const StreamObject &streamObject)
{
    // bring QTest::toString overloads into scope:
    using QTest::toString;

    // delegate char* handling to QTest::toString(QByteArray):
    return toString(streamObject.debug_description());
}
#endif

#ifdef QT_DEBUG
/// Custom Types to a Stream
QDebug operator<<(QDebug dbg, const StreamFormat &streamFormat)
{
    dbg.nospace() << streamFormat.debug_description();
    return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const StreamFormat *streamFormat)
{
    if (streamFormat)
        dbg.nospace() << *streamFormat;
    else
        dbg.nospace() << QLatin1String("StreamFormat(): nullptr");
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const StreamObject &streamObject)
{
    dbg.nospace() << streamObject.debug_description();
    return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const StreamObject *streamObject)
{
    if (streamObject)
        // Here, noquote() is called,
        // because it doesn't escape non-text characters contrary to nospace()
        // Each StreamObject should appear in a separated line
        dbg.noquote() << *streamObject;
    else
        dbg.nospace() << QLatin1String("StreamObject(): nullptr");
    return dbg.space();
}
#endif

void debugPrintProcessCommand(QProcess *process)
{
    QString text = "";
    text +=  process->program();
    text +=  " ";
    foreach (auto arg, process->arguments()) {
        text +=  arg;
        text +=  " ";
    }
    qDebug() << text;
}
