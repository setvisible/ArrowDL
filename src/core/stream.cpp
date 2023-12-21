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

#include <Core/FileUtils>
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

using namespace Qt::Literals::StringLiterals;

#if defined Q_OS_WIN
static const QString C_PROGRAM_NAME  = QLatin1String("yt-dlp.exe");
#else
static const QString C_PROGRAM_NAME  = QLatin1String("yt-dlp");
#endif

static const QString C_WEBSITE_URL   = QLatin1String("https://github.com/yt-dlp/yt-dlp");
static const int     C_EXIT_SUCCESS  = 0;

static const QString C_NONE          = QLatin1String("none");

static const QString C_WARNING_msg_header_01 = QLatin1String("WARNING:");
static const QString C_WARNING_msg_header_02 = QLatin1String("\\033[0;33mWARNING:\\033[0m");
static const QString C_ERROR_msg_header_01 = QLatin1String("ERROR:");
static const QString C_ERROR_msg_header_02 = QLatin1String("\\033[0;31mERROR:\\033[0m");

static const QString C_WARNING_merge_output_format = QLatin1String(
            "Requested formats are incompatible for merge and will be merged into mkv.");

static const QString C_DOWNLOAD_msg_header = QLatin1String("[download]");
static const QString C_DOWNLOAD_next_section = QLatin1String("Destination:");
static const QString C_MERGER_msg_header = QLatin1String("[Merger]");


static QString s_youtubedl_version = QString();
static int s_youtubedl_concurrent_fragments = 0;
static bool s_youtubedl_last_modified_time_enabled = true;
static QString s_youtubedl_user_agent = QString();
static int s_youtubedl_socket_type = 0;
static int s_youtubedl_socket_timeout = 0;

static bool areEqual(const QString &s1, const QString &s2)
{
    return s1.compare(s2, Qt::CaseInsensitive) == 0;
}

static void debug(QObject *sender, QProcess::ProcessError error);
static QString generateErrorMessage(QProcess::ProcessError error);
static QString toString(QProcess *process);

static void debugPrintProcessCommand(QProcess *process);

static QString standardToString(const QByteArray &bytes)
{
    return QString::fromLatin1(bytes).simplified();
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
                << "--no-colors"_L1
                << "--version"_L1;
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

void Stream::setConcurrentFragments(int fragments)
{
    s_youtubedl_concurrent_fragments = fragments > 0 ? fragments : 0;
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
    s_youtubedl_socket_timeout = secs > 0 ? secs : 0;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Get the version of the downloader asynchronously.
 */
void StreamVersion::run()
{
    // Stream::version() is blocking and time expensive
    QString result = Stream::version();
    if (!stopped) {
        emit resultReady(result);
    }
}

void StreamVersion::stop()
{
    stopped = true;
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

    auto domains = host.split('.', Qt::SkipEmptyParts);
    auto mandatoryDomains = regexHost.split(delimiters, Qt::SkipEmptyParts);

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
qsizetype Stream::fileSizeInBytes() const
{
    return _q_bytesTotal();
}

void Stream::setFileSizeInBytes(qsizetype fileSizeInBytes)
{
    m_bytesTotal = fileSizeInBytes;
}

/******************************************************************************
 ******************************************************************************/
StreamObject::Config Stream::config() const
{
    return m_config;
}

void Stream::setConfig(const StreamObject::Config &config)
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
    if (m_config.subtitle.writeSubtitle) {
        if (m_config.subtitle.isAutoGenerated) {
            arguments << QLatin1String("--write-auto-subs");
        } else {
            arguments << QLatin1String("--write-subs");
        }
        if (!m_config.subtitle.languages.isEmpty()) {
            arguments << QLatin1String("--sub-langs") << m_config.subtitle.languages;
        }
        if (!m_config.subtitle.extensions.isEmpty()) {
            arguments << QLatin1String("--sub-format") << m_config.subtitle.extensions;
        }
        if (!m_config.subtitle.convert.isEmpty()) {
            arguments << QLatin1String("--convert-subs") << m_config.subtitle.convert;
        }
    }
    if (m_config.chapter.writeChapters) {
        /// \todo implement chapters writing
    }
    if (m_config.thumbnail.writeDefaultThumbnail) {
        arguments << "--write-thumbnail"_L1;
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
    if (s_youtubedl_concurrent_fragments > 1) {
        arguments << QLatin1String("--concurrent-fragments")
                  << QString::number(s_youtubedl_concurrent_fragments);
    }
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
        arguments << "--referer"_L1 << m_referringPage;
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
/*!
 * \brief Try to clean properly the given multi-threaded raw that come in a single line.
 *
 * It takes a single-line message like:
 *    "[download] 11.2% ... [download] 12.3% ... [download] Destination: path\to\file.f599.m4a ..."
 *
 * and returns it as separate messages:
 *    "[download] 11.2% ..."
 *    "[download] 12.3% ..."
 *    "[download] Destination: path\to\file.f599.m4a ..."
 */
QStringList Stream::splitMultiThreadMessages(const QString &raw) const
{
    QStringList messages;
    QRegularExpression re(R"(\[download\]|\[Merger\])", QRegularExpression::CaseInsensitiveOption);
    QString raw2 = raw;
    qsizetype pos = raw2.lastIndexOf(re);
    if (0 <= pos && pos <= raw2.size()) {
        while (pos != -1) {
            QString message = raw2.last(raw2.size() - pos);
            messages.prepend(message);
            raw2.truncate(pos);
            pos = raw2.lastIndexOf(re);
        }
    } else {
        messages << raw2;
    }
    return messages;
}

void Stream::parseStandardOutput(const QString &msg)
{
    QStringList messages = splitMultiThreadMessages(msg);
    foreach (auto message, messages) {
        parseSingleStandardOutput(message);
    }
}

void Stream::parseSingleStandardOutput(const QString &msg)
{
    auto tokens = msg.split(QChar::Space, Qt::SkipEmptyParts);
    if (tokens.isEmpty()) {
        return;
    }
    if (tokens.count() == 0) {
        return;
    }
    if (areEqual(tokens.at(0), C_MERGER_msg_header)) {
        // During merger, the progress is arbitrarily at 99%, not 100%.
        qsizetype bytesTotal = _q_bytesTotal();
        qsizetype almostFinished = static_cast<qsizetype>(0.99 * qreal(bytesTotal));
        emit downloadProgress(almostFinished, bytesTotal);
        return;
    }
    if (areEqual(tokens.at(0), C_DOWNLOAD_msg_header)) {

        if ( tokens.count() > 2 &&
             areEqual(tokens.at(1), C_DOWNLOAD_next_section)) {
            m_bytesReceived += m_bytesReceivedCurrentSection;
            emit downloadProgress(m_bytesReceived, _q_bytesTotal());
            return;
        }

        if ( tokens.count() > 3 &&
             tokens.at(1).contains(QChar('%')) &&
             areEqual(tokens.at(2), QLatin1String("of")) ) {

            auto percentToken = tokens.at(1);
            auto sizeToken = !areEqual(tokens.at(3), QLatin1String("~"))
                    ? tokens.at(3)
                    : tokens.at(4);

            qreal percent = Format::parsePercentDecimal(percentToken);
            if (percent < 0) {
                qWarning("Can't parse '%s'.", percentToken.toLatin1().data());
                return;
            }

            m_bytesTotalCurrentSection = Format::parseBytes(sizeToken);
            if (m_bytesTotalCurrentSection < 0) {
                qWarning("Can't parse '%s'.", sizeToken.toLatin1().data());
                return;
            }
            m_bytesReceivedCurrentSection = static_cast<qsizetype>(qreal(percent * m_bytesTotalCurrentSection) / 100);
        }

        qsizetype received = m_bytesReceived + m_bytesReceivedCurrentSection;
        emit downloadProgress(received, _q_bytesTotal());
    }
}

void Stream::parseStandardError(const QString &msg)
{
    // qDebug() << Q_FUNC_INFO << bytes;

    if ( msg.startsWith(C_ERROR_msg_header_01, Qt::CaseInsensitive) ||
         msg.startsWith(C_ERROR_msg_header_02, Qt::CaseInsensitive)) {

        emit downloadError(msg);

    } else if ( msg.startsWith(C_WARNING_msg_header_01, Qt::CaseInsensitive) ||
                msg.startsWith(C_WARNING_msg_header_02, Qt::CaseInsensitive)) {

        if (msg.contains(C_WARNING_merge_output_format, Qt::CaseInsensitive)) {

            /// \todo change extension ?
            /// \todo propose: mkv, mp4, ogg, webm, flv (= merge formats if merge is required)
            m_fileExtension = QLatin1String("mkv");
            emit downloadMetadataChanged();
        }
    }
}

qsizetype Stream::_q_bytesTotal() const
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
                << "--no-colors"_L1
                << "--rm-cache-dir"_L1;
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
        dir = QDir::cleanPath(QDir::homePath() + "/.cache"_L1);
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
StreamAssetDownloader::StreamAssetDownloader(QObject *parent) : QObject(parent)
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


StreamAssetDownloader::~StreamAssetDownloader()
{
    m_processDumpJson->kill();
    m_processDumpJson->deleteLater();
    m_processFlatList->kill();
    m_processFlatList->deleteLater();
}

void StreamAssetDownloader::runAsync(const QString &url)
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

void StreamAssetDownloader::runAsyncDumpJson()
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

void StreamAssetDownloader::runAsyncFlatList()
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

void StreamAssetDownloader::stop()
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

bool StreamAssetDownloader::isRunning() const
{
    return !( m_processDumpJson->state() == QProcess::NotRunning &&
              m_processFlatList->state() == QProcess::NotRunning);
}

void StreamAssetDownloader::onStarted()
{
}

void StreamAssetDownloader::onError(QProcess::ProcessError error)
{
    debug(sender(), error);
    m_dumpMap.clear();
    m_flatList.clear();
    /// \todo verify race condition
}

void StreamAssetDownloader::onFinishedDumpJson(int exitCode, QProcess::ExitStatus exitStatus)
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

void StreamAssetDownloader::onFinishedFlatList(int exitCode, QProcess::ExitStatus exitStatus)
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

StreamAssetDownloader::StreamDumpMap StreamAssetDownloader::parseDumpMap(
        const QByteArray &stdoutBytes,
        const QByteArray &stderrBytes)
{
    StreamDumpMap map;
    QList<QByteArray> stdoutLines = stdoutBytes.split(QChar('\n').toLatin1());
    foreach (auto stdoutLine, stdoutLines) {
        if (!stdoutLine.isEmpty()) {
            StreamObject streamObject = parseDumpItemStdOut(stdoutLine);
            map.insert(streamObject.id(), streamObject);

        }
    }
    QList<QByteArray> stderrLines = stderrBytes.split(QChar('\n').toLatin1());
    foreach (auto stderrLine, stderrLines) {
        if (!stderrLine.isEmpty()) {
            StreamObject streamObject = parseDumpItemStdErr(stderrLine);
            map.insert(streamObject.id(), streamObject);
        }
    }
    return map;
}

static void parseSubtitles(
        const QJsonObject &jsonSubtitles,
        const bool isAutomatic,
        StreamObject::Data *data = nullptr)
{
    Q_ASSERT(data);
    foreach (const QString &languageCode, jsonSubtitles.keys()) {
        QJsonValue jsonLanguage = jsonSubtitles.value(languageCode);
        QJsonArray jsonExtensions = jsonLanguage.toArray();
        foreach (auto jsonExtension, jsonExtensions) {
            QJsonObject jsonSub = jsonExtension.toObject();
            StreamSubtitle subtitle;
            subtitle.languageCode = languageCode;
            subtitle.ext  = jsonSub[QLatin1String("ext")].toString();
            subtitle.url  = jsonSub[QLatin1String("url")].toString();
            subtitle.data = jsonSub[QLatin1String("data")].toString();
            subtitle.languageName = jsonSub[QLatin1String("name")].toString();
            subtitle.isAutomatic = isAutomatic;
            data->subtitles << subtitle;
        }
    }
}

StreamObject StreamAssetDownloader::parseDumpItemStdOut(const QByteArray &bytes)
{
    // *****************************************************************
    // The attributes are listed by the "class InfoExtractor(object):"
    // in <ROOT_YT_DLP>/yt_dlp/extractor/common.py
    // *****************************************************************
    StreamObject obj;
    auto data = obj.data();

    QJsonParseError ok{};
    QJsonDocument loadDoc(QJsonDocument::fromJson(bytes, &ok));
    if (ok.error != QJsonParseError::NoError) {

        qDebug() << Q_FUNC_INFO;
        qDebug() << "! Error JSON:" << QJsonParseError::ParseError(ok.error);
        qDebug() << "   at position" << ok.offset;
        qDebug() << bytes.mid(ok.offset - 20, ok.offset + 20);
        qDebug() << QString("%0^").arg(QString().fill(' ', 20));

        obj.setError(StreamObject::ErrorJsonFormat);
        return obj;
    }
    QJsonObject json = loadDoc.object();
    data.id                 = json[QLatin1String("id")].toString();

    auto title              = json[QLatin1String("title")].toString();
    if (title.isEmpty()) {
        title               = json[QLatin1String("alt_title")].toString();
    }
    if (title.isEmpty()) {
        title               = json[QLatin1String("fulltitle")].toString();
    }
    if (title.isEmpty()) {
        title               = json[QLatin1String("track")].toString();
    }
    data.title = title;

    QJsonArray jsonFormats  = json[QLatin1String("formats")].toArray();
    foreach (auto jsonFormat, jsonFormats) {
        QJsonObject jsonFmt = jsonFormat.toObject();
        StreamFormat format;

        format.url          = jsonFmt[QLatin1String("url")].toString();
        format.ext          = jsonFmt[QLatin1String("ext")].toString();

        format.format       = jsonFmt[QLatin1String("format")].toString();
        format.formatId     = StreamFormatId(jsonFmt[QLatin1String("format_id")].toString());
        format.formatNote  = jsonFmt[QLatin1String("format_note")].toString();

        format.width        = jsonFmt[QLatin1String("width")].toInt();
        format.height       = jsonFmt[QLatin1String("height")].toInt();
        format.resolution   = jsonFmt[QLatin1String("resolution")].toString();
        format.dynamicRange = jsonFmt[QLatin1String("dynamic_range")].toString();

        format.tbr          = jsonFmt[QLatin1String("tbr")].toDouble();
        format.abr          = jsonFmt[QLatin1String("abr")].toDouble();
        format.acodec       = jsonFmt[QLatin1String("acodec")].toString();
        format.asr          = jsonFmt[QLatin1String("asr")].toInt();
        format.vbr          = jsonFmt[QLatin1String("vbr")].toInt();
        format.fps          = jsonFmt[QLatin1String("fps")].toInt();
        format.vcodec       = jsonFmt[QLatin1String("vcodec")].toString();

        format.filesize     = jsonFmt[QLatin1String("filesize")].toInteger();
        if (!(format.filesize > 0)) {
            format.filesize = jsonFmt[QLatin1String("filesize_approx")].toInteger();
        }
        data.formats << format;
    }

    data.defaultSuffix      = json[QLatin1String("ext")].toString();
    data.description        = json[QLatin1String("description")].toString();

    auto artist             = json[QLatin1String("artist")].toString();
    if (artist.isEmpty()) {
        artist              = json[QLatin1String("creator")].toString();
    }
    data.artist = artist;

    data.album              = json[QLatin1String("album")].toString();

    auto release_year       = json[QLatin1String("release_year")].toInt();
    if (release_year > 0) {
        data.release_year = QString("%0").arg(QString::number(release_year));
    } else {
        data.release_year   = json[QLatin1String("release_date")].toString();
    }

    data.thumbnail          = json[QLatin1String("thumbnail")].toString();

    // Subtitles and Automatic Captions have the same format
    {
        QJsonObject jsonSubtitles = json[QLatin1String("subtitles")].toObject();
        parseSubtitles(jsonSubtitles, false, &data);
    }
    {
        QJsonObject jsonCaptions = json[QLatin1String("automatic_captions")].toObject();
        parseSubtitles(jsonCaptions, true, &data);
    }

    data.webpage_url        = json[QLatin1String("webpage_url")].toString();

    //-----
    // Specific Extractors
    data.originalFilename   = json[QLatin1String("_filename")].toString();
    data.extractor          = json[QLatin1String("extractor")].toString();
    data.extractor_key      = json[QLatin1String("extractor_key")].toString();
    data.defaultFormatId    = StreamFormatId(json[QLatin1String("format_id")].toString());

    data.playlist           = json[QLatin1String("playlist")].toString();
    auto playlist_index     = json[QLatin1String("playlist_index")].toInt();
    auto playlist_autonumber= json[QLatin1String("playlist_autonumber")].toInt();
    auto playlist_count     = json[QLatin1String("playlist_count")].toInt();
    auto n_entries          = json[QLatin1String("n_entries")].toInt();

    if (!(playlist_index > 0)) {
        playlist_index = playlist_autonumber;
    }
    if (!(playlist_count > 0)) {
        playlist_count = n_entries;
    }
    if (playlist_index > 0) {
        auto digits = QString::number(playlist_count).count();
        data.playlist_index = QString("%0").arg(QString::number(playlist_index), digits, QLatin1Char('0'));
    }

    obj.setError(StreamObject::NoError);
    obj.setData(data);
    return obj;
}

static StreamObjectId findId(const QByteArray &bytes)
{
    /*
     * "ERROR: 0123456789a: YouTube said: Unable to extract video data"
     * should return "0123456789a"
     */
    QString str(bytes);
    QStringList values = str.split(QLatin1String(":"), Qt::SkipEmptyParts);
    return values.count() > 1 ? values.at(1).trimmed() : StreamObjectId();
}

StreamObject StreamAssetDownloader::parseDumpItemStdErr(const QByteArray &bytes)
{
    StreamObject ret;
    auto data = ret.data();
    data.id = findId(bytes);
    ret.setError(StreamObject::ErrorUnavailable);
    ret.setData(data);
    return ret;
}

StreamAssetDownloader::StreamFlatList StreamAssetDownloader::parseFlatList(
        const QByteArray &stdoutBytes,
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

StreamAssetDownloader::StreamFlatListItem StreamAssetDownloader::parseFlatItem(
        const QByteArray &bytes)
{
    StreamFlatListItem item;
    QJsonParseError ok{};
    QJsonDocument loadDoc(QJsonDocument::fromJson(bytes, &ok));
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

void StreamAssetDownloader::onFinished()
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
            si.data().playlist_index = QString::number(playlist_index);
            streamObjects << si;
        }
        // Some videos might have errors or not available, but it's ok.
        emit collected(streamObjects);
    }
}

StreamObject StreamAssetDownloader::createStreamObject(const StreamFlatListItem &flatItem) const
{
    StreamObject si;
    if (!flatItem.id.isEmpty() && m_dumpMap.contains(flatItem.id)) {
        si = m_dumpMap.value(flatItem.id);
    } else {
        /// \todo replace with ErrorGeoRestriction instead?
        si.setError(StreamObject::ErrorUnavailable);
    }
    if (si.data().title.isEmpty()) {
        si.data().title = flatItem.title;
    }
    if (si.data().webpage_url.isEmpty()) {
        si.data().webpage_url = flatItem.url; /// \todo fix incomplete URL
    }
    return si;
}

void StreamAssetDownloader::onCacheCleaned()
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
            auto bytes = m_processExtractors->readAllStandardOutput();
            QString str(bytes);
            m_extractors = str.split(QChar('\n'), Qt::KeepEmptyParts);
            onFinished();
        } else {
            auto errorMessage = standardToString(m_processExtractors->readAllStandardError());
            emit error(errorMessage);
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
            auto bytes = m_processDescriptions->readAllStandardOutput();
            QString str(bytes);
            m_descriptions = str.split("\n", Qt::KeepEmptyParts);
            onFinished();
        } else {
            auto errorMessage = standardToString(m_processDescriptions->readAllStandardError());
            emit error(errorMessage);
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
    auto split = format_id.split(QChar('+'), Qt::SkipEmptyParts);
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
StreamObject::Data::Format::Format(const QString &format_id,
        const QString &ext,
        const QString &formatNote,
        qsizetype filesize,
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
    this->formatNote   = formatNote;
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

bool StreamObject::Data::Format::operator!=(const StreamFormat &other) const
{
    return !(*this == other);
}

bool StreamObject::Data::Subtitle::operator!=(const Subtitle &other) const
{
    return !(*this == other);
}

bool StreamObject::Data::Format::hasVideo() const {
    return vcodec != C_NONE;
}

bool StreamObject::Data::Format::hasMusic() const {
    return acodec != C_NONE;
}

QString StreamObject::Data::Format::toString() const
{
    if (hasVideo() && hasMusic()) {
        return QObject::tr("Video %0 x %1%2%3").arg(
                    width <= 0 ? QLatin1String("?") : QString::number(width),
                    height <= 0 ? QLatin1String("?") : QString::number(height),
                    formatNote.isEmpty() ? QString() : QString(" (%0)").arg(formatNote),
                    filesize <= 0 ? QString() : QString(", size: %0").arg(::Format::fileSizeToString(filesize)));
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

QString StreamObject::Data::Format::debug_description() const
{
    return QString("StreamObject::Data::Format '%0' (%1, %2, %3)").arg(
                formatId.toString(),
                QString("%0, %1, %2").arg(
                    ext,
                    formatNote,
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
/// \todo C++11 relational operations must be explicit. Since C++14 they can be defaulted
bool StreamObject::Data::operator!=(const Data &other) const
{
    return !(*this == other);
}

bool StreamObject::Config::Overview::operator!=(const Overview &other) const
{
    return !(*this == other);
}

bool StreamObject::Config::Subtitle::operator!=(const Subtitle &other) const
{
    return !(*this == other);
}

bool StreamObject::Config::Chapter::operator!=(const Chapter &other) const
{
    return !(*this == other);
}

bool StreamObject::Config::Thumbnail::operator!=(const Thumbnail &other) const
{
    return !(*this == other);
}

bool StreamObject::Config::Comment::operator!=(const Comment &other) const
{
    return !(*this == other);
}

bool StreamObject::Config::Metadata::operator!=(const Metadata &other) const
{
    return !(*this == other);
}

bool StreamObject::Config::Processing::operator!=(const Processing &other) const
{
    return !(*this == other);
}

bool StreamObject::Config::SponsorBlock::operator!=(const SponsorBlock &other) const
{
    return !(*this == other);
}

bool StreamObject::Config::operator!=(const Config &other) const
{
    return !(*this == other);
}

bool StreamObject::operator!=(const StreamObject &other) const
{
    return !(*this == other);
}

/******************************************************************************
 ******************************************************************************/
StreamObject::Data StreamObject::data() const
{
    return m_data;
}

void StreamObject::setData(const Data &data)
{
    m_data = data;
}

/******************************************************************************
 ******************************************************************************/
StreamObject::Config StreamObject::config() const
{
    return m_config;
}

void StreamObject::setConfig(const Config &config)
{
    m_config = config;
}

/******************************************************************************
 ******************************************************************************/
qsizetype StreamObject::guestimateFullSize() const
{
    return guestimateFullSize(formatId());
}

qsizetype StreamObject::guestimateFullSize(const StreamFormatId &formatId) const
{
    if (formatId.isEmpty()) {
        return -1;
    }
    QMap<StreamFormatId, qsizetype> sizes;
    for (auto format : m_data.formats) {
        sizes.insert(format.formatId, format.filesize);
    }
    qsizetype estimatedSize = 0;
    for (auto id : formatId.compoundIds()) {
        estimatedSize += sizes.value(id, 0);
    }
    return estimatedSize;
}

QString StreamObject::defaultTitle() const
{
    auto title = m_data.title;
    if (!m_data.artist.isEmpty() && !title.contains(m_data.artist, Qt::CaseInsensitive)) {
        title.prepend(QString("%0 - ").arg(m_data.artist));
    }
    if (!m_data.release_year.isEmpty() && !title.contains(m_data.release_year, Qt::CaseInsensitive)) {
        title.append(QString(" (%0)").arg(m_data.release_year));
    }
    return title;
}

QString StreamObject::title() const
{
    return m_userTitle.isEmpty() ? defaultTitle() : m_userTitle;
}

void StreamObject::setTitle(const QString &title)
{
    m_userTitle = (title == defaultTitle()) ? QString() : title;
}

QString StreamObject::fullFileName() const
{
    return suffix().isEmpty()
            ? fileBaseName()
            : QString("%0.%1").arg(fileBaseName(), suffix());
}

QString StreamObject::fileBaseName() const
{
    return FileUtils::cleanFileName(title());
}

QString StreamObject::suffix() const
{
    return m_userSuffix.isEmpty() ? suffix(formatId()) : m_userSuffix;
}

QString StreamObject::suffix(const StreamFormatId &formatId) const
{
    if (m_data.defaultFormatId.isEmpty()) {
        return QLatin1String("???");
    }
    if (m_data.defaultFormatId == formatId) {
        return m_data.defaultSuffix;
    }
    auto suffix = m_data.defaultSuffix;
    foreach (auto id, formatId.compoundIds()) {
        foreach (auto format, m_data.formats) {
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
    m_userSuffix = (suffix == m_data.defaultSuffix) ? QString() : suffix;
}

StreamFormatId StreamObject::formatId() const
{
    return m_userFormatId.isEmpty() ? m_data.defaultFormatId : m_userFormatId;
}

void StreamObject::setFormatId(const StreamFormatId &formatId)
{
    m_userSuffix = QString();
    m_userFormatId = (formatId == m_data.defaultFormatId) ? StreamFormatId() : formatId;
}

QString StreamObject::formatToString() const
{
    QString ret;
    foreach (auto id, formatId().compoundIds()) {
        for (auto format : m_data.formats) {
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

QList<StreamObject::Data::Format> StreamObject::Data::defaultFormats() const
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

QList<StreamObject::Data::Format> StreamObject::Data::audioFormats() const
{
    QList<StreamFormat> list;
    foreach (auto format, formats) {
        if (!format.hasVideo() && format.hasMusic()) {
            list.append(format);
        }
    }
    return list;
}

QList<StreamObject::Data::Format> StreamObject::Data::videoFormats() const
{
    QList<StreamFormat> list;
    foreach (auto format, formats) {
        if (format.hasVideo() && !format.hasMusic()) {
            list.append(format);
        }
    }
    return list;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Get a copy of the subtitles list, without extension/url/data duplicate
 */
QList<StreamObject::Data::Subtitle> StreamObject::Data::subtitleLanguages() const
{
    QMap<QString, StreamObject::Data::Subtitle> map;
    foreach (auto subtitle, subtitles) {
        QString key =
                subtitle.languageCode + " " +
                subtitle.languageName + " " +
                (subtitle.isAutomatic == true ? QLatin1String("auto-generated")
                                              : QLatin1String("normal"));
        map.insert(key, subtitle);
    }
    return map.values();
}

QList<QString> StreamObject::Data::subtitleExtensions() const
{
    // QSet avoids duplicate entries
    QSet<QString> set;
    foreach (auto subtitle, subtitles) {
        set.insert(subtitle.ext);
    }
    return set.values();  // unsorted
}

/******************************************************************************
 ******************************************************************************/
bool StreamObject::isAvailable() const
{
    return m_error == NoError;
}

QString StreamObject::Data::debug_description() const
{
    QString descr;
    descr.append(QString("StreamObject::Data '%0' [%1] (%2, %3)").arg(
                     originalFilename,
                     webpage_url,
                     QString("%0, %1, %2, %3, %4, %5").arg(
                         title,
                         defaultSuffix,
                         description,
                         artist,
                         release_year,
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
    return toString(streamObject.data().debug_description());
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
    dbg.nospace() << streamObject.data().debug_description();
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
