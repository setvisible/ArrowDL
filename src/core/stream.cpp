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
#ifdef QT_TESTLIB_LIB
#  include <QtTest/QTest>
#endif

static QString generateErrorMessage(QProcess::ProcessError error);
static QString toString(QProcess *process);

static QString s_youtubedl_version = QString();
static QString s_youtubedl_user_agent = QString();

#if defined Q_OS_WIN
static const QString C_PROGRAM_NAME  = QLatin1String("youtube-dl.exe");
#else
static const QString C_PROGRAM_NAME  = QLatin1String("./youtube-dl");
#endif

static const QString C_WEBSITE_URL   = QLatin1String("http://ytdl-org.github.io/youtube-dl/");

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


Stream::Stream(QObject *parent) : QObject(parent)
  , m_process(new QProcess(this))
{
    connect(m_process, SIGNAL(started()), this, SLOT(onStarted()));
#if QT_VERSION >= 0x050600
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
        QProcess process;
        process.start(
                    C_PROGRAM_NAME, QStringList()
                    << QLatin1String("--version"));
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

void Stream::setUserAgent(const QString &userAgent)
{
    s_youtubedl_user_agent = userAgent;
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
    static QRegExp delimiters("[.|:]");

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
    m_selectedFormatId.clear();
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
void Stream::initializeWithStreamInfo(const StreamInfo &streamInfo)
{
    m_selectedFormatId = streamInfo.format_id;
    m_bytesReceived = 0;
    m_bytesReceivedCurrentSection = 0;
    m_bytesTotal = 0;
    m_bytesTotalCurrentSection = streamInfo.guestimateFullSize(m_selectedFormatId);
    m_fileBaseName = streamInfo.fileBaseName();
    m_fileExtension = streamInfo.fileExtension(m_selectedFormatId);
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
QString Stream::selectedFormatId() const
{
    return m_selectedFormatId;
}

void Stream::setSelectedFormatId(const QString &formatId)
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
QString Stream::fileName() const
{
    if (m_fileExtension.isEmpty()) {
        return m_fileBaseName;
    }
    return QString("%0.%1").arg(m_fileBaseName, m_fileExtension);
}

/******************************************************************************
 ******************************************************************************/
void Stream::start()
{
    if (!isEmpty() && m_process->state() == QProcess::NotRunning) {
        // Usage: youtube-dl.exe [OPTIONS] URL [URL...]
        QStringList arguments;
        arguments << QLatin1String("--output") << m_outputPath
                  << QLatin1String("--no-check-certificate")
                  << QLatin1String("--no-overwrites")  // If option in settings!
                  << QLatin1String("--no-continue")
                  << QLatin1String("--ignore-config")
                  << QLatin1String("--format") << m_selectedFormatId
                  << m_url;
        if (!s_youtubedl_user_agent.isNull()) {
            arguments << QLatin1String("--user-agent") << s_youtubedl_user_agent;
        }
        if (!m_referringPage.isEmpty()) {
            arguments << QLatin1String("--referer") << m_referringPage;
        }
        m_process->start(C_PROGRAM_NAME, arguments);
        qDebug() << Q_FUNC_INFO << toString(m_process);
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
    qDebug() << Q_FUNC_INFO;
}

void Stream::onError(QProcess::ProcessError error)
{
    // Issue with configuration, or argument,
    // but not due to user input like invalid URL.
    // Also check that the path to "youtube-dl" program is valid.
    qDebug() << Q_FUNC_INFO << generateErrorMessage(error);
}

void Stream::onFinished(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
    emit downloadProgress(_q_bytesTotal(), _q_bytesTotal());
    emit downloadFinished();
}

void Stream::onStandardOutputReady()
{
    QString data = QString::fromLatin1(m_process->readAllStandardOutput());
    data = data.simplified();
    parseStandardOutput(data);
}

void Stream::onStandardErrorReady()
{
    QString data = QString::fromLatin1(m_process->readAllStandardError());
    data = data.simplified();
    parseStandardError(data);
}

/******************************************************************************
 ******************************************************************************/
void Stream::parseStandardOutput(const QString &data)
{
    // qDebug() << Q_FUNC_INFO << data;

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

        const QString percentToken = tokens.at(1);
        const QString sizeToken= tokens.at(3);

        double percent = Format::parsePercentDecimal(percentToken);
        if (percent < 0) {
            qDebug() << Q_FUNC_INFO << "ERROR: Can't parse" << percentToken;
            return;
        }

        m_bytesTotalCurrentSection = Format::parseBytes(sizeToken);
        if (m_bytesTotalCurrentSection < 0) {
            qDebug() << Q_FUNC_INFO << "ERROR: Can't parse" << sizeToken;
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
            m_fileExtension = QLatin1String("mkv");   // TODO change extension
            emit downloadMetadataChanged();
        }
    }
}

qint64 Stream::_q_bytesTotal() const
{
    return m_bytesTotal > 0 ? m_bytesTotal : m_bytesTotalCurrentSection;
}

/******************************************************************************
 ******************************************************************************/
StreamCleanCache::StreamCleanCache(QObject *parent) : QObject(parent)
  , m_process(new QProcess(this))
  , m_isCleaned(false)
{
    connect(m_process, SIGNAL(started()), this, SLOT(onStarted()));
#if QT_VERSION >= 0x050600
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(onError(QProcess::ProcessError)));
#endif
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onFinished(int, QProcess::ExitStatus)));
}

StreamCleanCache::~StreamCleanCache()
{
    m_process->kill();
    m_process->deleteLater();
}

void StreamCleanCache::runAsync()
{
    if (m_process->state() == QProcess::NotRunning) {
        m_process->start(
                    C_PROGRAM_NAME, QStringList()
                    << QLatin1String("--rm-cache-dir"));
        qDebug() << Q_FUNC_INFO << toString(m_process);
    }
}

QString StreamCleanCache::cacheDir()
{
    // Try to get the .cache from $XDG_CACHE_HOME, if it's not set,
    // it has to be in ~/.cache as per XDG standard
    QString dir = QString::fromUtf8(getenv("XDG_CACHE_HOME"));
    if (dir.isEmpty()) {
        dir = QDir::cleanPath(QDir::homePath() + QLatin1String("/.cache"));
    }
    return QDir::toNativeSeparators(dir);
}

bool StreamCleanCache::isCleaned() const
{
    return m_isCleaned;
}

void StreamCleanCache::onStarted()
{
    qDebug() << Q_FUNC_INFO;
}

void StreamCleanCache::onError(QProcess::ProcessError error)
{
    qDebug() << Q_FUNC_INFO << generateErrorMessage(error);
}

void StreamCleanCache::onFinished(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
    m_isCleaned = true;
    emit done();
}

/******************************************************************************
 ******************************************************************************/
StreamInfoDownloader::StreamInfoDownloader(QObject *parent) : QObject(parent)
  , m_process(new QProcess(this))
  , m_streamCleanCache(new StreamCleanCache(this))
  , m_url(QString())
  , m_cancelled(false)
{
    connect(m_process, SIGNAL(started()), this, SLOT(onStarted()));
#if QT_VERSION >= 0x050600
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(onError(QProcess::ProcessError)));
#endif
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onFinished(int, QProcess::ExitStatus)));
    connect(m_streamCleanCache, SIGNAL(done()), this, SLOT(onCacheCleaned()));
}

StreamInfoDownloader::~StreamInfoDownloader()
{
    m_process->kill();
    m_process->deleteLater();
}

void StreamInfoDownloader::runAsync(const QString &url)
{
    m_url = url;
    m_cancelled = false;
    if (m_process->state() == QProcess::NotRunning) {
        QStringList arguments;
        arguments << QLatin1String("--dump-json")
                  << m_url;
        if (!s_youtubedl_user_agent.isNull()) {
            arguments << QLatin1String("--user-agent") << s_youtubedl_user_agent;
        }
        m_process->start(C_PROGRAM_NAME, arguments);
        qDebug() << Q_FUNC_INFO << toString(m_process);
    }
}

void StreamInfoDownloader::stop()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_cancelled = true;
        m_process->kill();
    }
}

bool StreamInfoDownloader::isRunning() const
{
    return m_process->state() != QProcess::NotRunning;
}

void StreamInfoDownloader::onStarted()
{
    qDebug() << Q_FUNC_INFO;
}

void StreamInfoDownloader::onError(QProcess::ProcessError error)
{
    qDebug() << Q_FUNC_INFO << generateErrorMessage(error);
}

void StreamInfoDownloader::onFinished(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    //qDebug() << Q_FUNC_INFO << exitCode << exitStatus;
    if (exitCode != 0) {
        if (m_cancelled) {
            emit error(tr("Cancelled."));
            return;
        }
        if (!m_streamCleanCache->isCleaned()) {
            m_streamCleanCache->runAsync(); // clean cache and retry
            return;
        }
        auto message = generateErrorMessage(m_process->error());
        emit error(message);
    } else {
        QByteArray data = m_process->readAllStandardOutput();
        StreamInfoPtr info(new StreamInfo());
        if (parseJSON(data, info.data())) {
            emit collected(info);
        } else {
            emit error(tr("Couldn't parse JSON file."));
        }
    }
}

void StreamInfoDownloader::onCacheCleaned()
{
    runAsync(m_url); // retry
}

bool StreamInfoDownloader::parseJSON(const QByteArray &data, StreamInfo *info)
{
    QJsonParseError ok;
    QJsonDocument loadDoc(QJsonDocument::fromJson(data, &ok));
    if (ok.error != QJsonParseError::NoError) {
        return false;
    }
    QJsonObject json = loadDoc.object();

    info->_filename         = json[QLatin1String("_filename")].toString();
    info->fulltitle         = json[QLatin1String("fulltitle")].toString();
    info->title             = json[QLatin1String("title")].toString();
    info->ext               = json[QLatin1String("ext")].toString();
    info->description       = json[QLatin1String("description")].toString();
    info->thumbnail         = json[QLatin1String("thumbnail")].toString();
    info->extractor         = json[QLatin1String("extractor")].toString();
    info->extractor_key     = json[QLatin1String("extractor_key")].toString();
    info->format_id         = json[QLatin1String("format_id")].toString();
    QJsonArray jobsArray    = json[QLatin1String("formats")].toArray();
    for (int i = 0; i < jobsArray.size(); ++i) {
        StreamFormat *format = new StreamFormat(info);
        QJsonObject jobObject = jobsArray[i].toObject();
        format->format_id    = jobObject[QLatin1String("format_id")].toString();
        format->ext          = jobObject[QLatin1String("ext")].toString();
        format->format_note  = jobObject[QLatin1String("format_note")].toString();
        format->filesize     = jobObject[QLatin1String("filesize")].toInt();
        format->acodec       = jobObject[QLatin1String("acodec")].toString();
        format->abr          = jobObject[QLatin1String("abr")].toInt();
        format->asr          = jobObject[QLatin1String("asr")].toInt();
        format->vcodec       = jobObject[QLatin1String("vcodec")].toString();
        format->width        = jobObject[QLatin1String("width")].toInt();
        format->height       = jobObject[QLatin1String("height")].toInt();
        format->fps          = jobObject[QLatin1String("fps")].toInt();
        format->tbr          = jobObject[QLatin1String("tbr")].toInt();
        info->formats << format;
    }
    info->playlist          = json[QLatin1String("playlist")].toString();
    info->playlist_index    = json[QLatin1String("playlist_index")].toString();
    return true;
}

/******************************************************************************
 ******************************************************************************/
StreamUpgrader::StreamUpgrader(QObject *parent) : QObject(parent)
  , m_process(new QProcess(this))
{
    connect(m_process, SIGNAL(started()),
            this, SLOT(onStarted()));
#if QT_VERSION >= 0x050600
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(onError(QProcess::ProcessError)));
#endif
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onFinished(int, QProcess::ExitStatus)));
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
        m_process->start(
                    C_PROGRAM_NAME, QStringList()
                    << QLatin1String("--update"));
        qDebug() << Q_FUNC_INFO << toString(m_process);
    }
}

void StreamUpgrader::onStarted()
{
    qDebug() << Q_FUNC_INFO;
}

void StreamUpgrader::onError(QProcess::ProcessError error)
{
    qDebug() << Q_FUNC_INFO << generateErrorMessage(error);
}

void StreamUpgrader::onStandardOutputReady()
{
    QString data = QString::fromLatin1(m_process->readAllStandardOutput());
    data = data.simplified();
    qDebug() << data;
}

void StreamUpgrader::onStandardErrorReady()
{
    QString data = QString::fromLatin1(m_process->readAllStandardError());
    data = data.simplified();
    qDebug() << "Error:" << data;
}

void StreamUpgrader::onFinished(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    if (exitCode != 0) {
        auto message = generateErrorMessage(m_process->error());
        qDebug() << "Error:" << message;
    }
    emit done();
}

/******************************************************************************
 ******************************************************************************/
StreamExtractorListCollector::StreamExtractorListCollector(QObject *parent) : QObject(parent)
  , m_processExtractors(new QProcess(this))
  , m_processDescriptions(new QProcess(this))
{
    connect(m_processExtractors, SIGNAL(started()),
            this, SLOT(onStarted()));
#if QT_VERSION >= 0x050600
    connect(m_processExtractors, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(onError(QProcess::ProcessError)));
#endif
    connect(m_processExtractors, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onFinishedExtractors(int, QProcess::ExitStatus)));

    connect(m_processDescriptions, SIGNAL(started()),
            this, SLOT(onStarted()));
#if QT_VERSION >= 0x050600
    connect(m_processDescriptions, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(onError(QProcess::ProcessError)));
#endif
    connect(m_processDescriptions, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onFinishedDescriptions(int, QProcess::ExitStatus)));
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
        m_processExtractors->start(
                    C_PROGRAM_NAME, QStringList()
                    << QLatin1String("--list-extractors"));
        qDebug() << Q_FUNC_INFO << toString(m_processExtractors);
    }
    if (m_processDescriptions->state() == QProcess::NotRunning) {
        m_processDescriptions->start(
                    C_PROGRAM_NAME, QStringList()
                    << QLatin1String("--extractor-descriptions"));
        qDebug() << Q_FUNC_INFO << toString(m_processDescriptions);
    }
}

void StreamExtractorListCollector::onStarted()
{
    qDebug() << Q_FUNC_INFO;
}

void StreamExtractorListCollector::onError(QProcess::ProcessError error)
{
    qDebug() << Q_FUNC_INFO << generateErrorMessage(error);
    m_extractors.clear();
    m_descriptions.clear();
}

void StreamExtractorListCollector::onFinishedExtractors(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    if (exitCode != 0) {
        auto message = generateErrorMessage(m_processExtractors->error());
        emit error(message);
        emit finished();
    } else {
        QString data = QString::fromLatin1(m_processExtractors->readAllStandardOutput());
        m_extractors = data.split(QChar('\n'), QString::KeepEmptyParts);
        onFinished();
    }
}

void StreamExtractorListCollector::onFinishedDescriptions(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    if (exitCode != 0) {
        auto message = generateErrorMessage(m_processDescriptions->error());
        emit error(message);
        emit finished();
    } else {
        QString data = QString::fromLatin1(m_processDescriptions->readAllStandardOutput());
        m_descriptions = data.split("\n", QString::KeepEmptyParts);
        onFinished();
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
StreamFormat::StreamFormat(QObject *parent) : QObject(parent)
{
}

StreamFormat::StreamFormat(const StreamFormat &other) : QObject(other.parent())
{
    this->format_id    = other.format_id   ;
    this->ext          = other.ext         ;
    this->format_note  = other.format_note ;
    this->filesize     = other.filesize    ;
    this->acodec       = other.acodec      ;
    this->abr          = other.abr         ;
    this->asr          = other.asr         ;
    this->vcodec       = other.vcodec      ;
    this->width        = other.width       ;
    this->height       = other.height      ;
    this->fps          = other.fps         ;
    this->tbr          = other.tbr         ;
}

StreamFormat::StreamFormat(
        QString format_id,
        QString ext,
        QString format_note,
        int filesize,
        QString acodec,
        int abr,
        int asr,
        QString vcodec,
        int width,
        int height,
        int fps,
        int tbr,
        QObject *parent) : QObject(parent)
{
    this->format_id    = format_id   ;
    this->ext          = ext         ;
    this->format_note  = format_note ;
    this->filesize     = filesize    ;
    this->acodec       = acodec      ;
    this->abr          = abr         ;
    this->asr          = asr         ;
    this->vcodec       = vcodec      ;
    this->width        = width       ;
    this->height       = height      ;
    this->fps          = fps         ;
    this->tbr          = tbr         ;
}

StreamFormat::~StreamFormat()
{
}

bool StreamFormat::operator==(const StreamFormat &other) const
{
    return (format_id    == other.format_id   &&
            ext          == other.ext         &&
            format_note  == other.format_note &&
            filesize     == other.filesize    &&
            acodec       == other.acodec      &&
            abr          == other.abr         &&
            asr          == other.asr         &&
            vcodec       == other.vcodec      &&
            width        == other.width       &&
            height       == other.height      &&
            fps          == other.fps         &&
            tbr          == other.tbr         );
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
        return QObject::tr("Video %0 x %1%2%3")
                .arg(width <= 0 ? QLatin1String("?") : QString::number(width))
                .arg(height <= 0 ? QLatin1String("?") : QString::number(height))
                .arg(format_note.isEmpty() ? QString() : QString(" (%0)").arg(format_note))
                .arg(filesize <= 0 ? QString() : QString(", size: %0").arg(Format::fileSizeToString(filesize)));
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
    return QString("StreamFormat '%0' (%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11)")
            .arg(format_id)
            .arg(ext)
            .arg(format_note)
            .arg(filesize)
            .arg(acodec)
            .arg(abr)
            .arg(asr)
            .arg(vcodec)
            .arg(width)
            .arg(height)
            .arg(fps)
            .arg(tbr);
}

/******************************************************************************
 ******************************************************************************/
StreamInfo::StreamInfo(QObject *parent) : QObject(parent)
{
}

StreamInfo::StreamInfo(const StreamInfo &other): QObject(other.parent())
{
    this->_filename        = other._filename      ;
    this->fulltitle        = other.fulltitle      ;
    this->title            = other.title          ;
    this->ext              = other.ext            ;
    this->description      = other.description    ;
    this->thumbnail        = other.thumbnail      ;
    this->extractor        = other.extractor      ;
    this->extractor_key    = other.extractor_key  ;
    this->format_id        = other.format_id      ;
    this->formats          = other.formats        ;
    this->playlist         = other.playlist       ;
    this->playlist_index   = other.playlist_index ;
}

StreamInfo::~StreamInfo()
{
}

qint64 StreamInfo::guestimateFullSize(const QString &format_id) const
{
    if (format_id.isEmpty()) {
        return -1;
    }
    QMap<QString, qint64> sizes;
    for (auto format : formats) {
        sizes.insert(format->format_id, format->filesize);
    }
    qint64 estimedSize = 0;
    QStringList ids = format_id.split(QChar('+'));
    for (auto id : ids) {
        estimedSize += sizes.value(id, 0);
    }
    return estimedSize;
}

QString StreamInfo::safeTitle() const
{
    return this->title;
}

static QString cleanFileName(const QString &fileName)
{
    QString ret = fileName.simplified();
    QString::iterator it;
    for (it = ret.begin(); it != ret.end(); ++it){
        const QChar c = (*it).unicode();
        if (c.isLetterOrNumber() || C_LEGAL_CHARS.contains(c)) {
            continue;
        } else if (c == QChar('"')) {
            *it = QChar('\'');
        } else {
            *it = QChar('_');
        }
    }
    ret = ret.replace(QRegExp("_+"), QLatin1String("_"));
    return ret.simplified();
}

QString StreamInfo::fileBaseName() const
{
    return cleanFileName(this->title);
}

QString StreamInfo::fileExtension(const QString &formatId) const
{
    if (format_id.isEmpty()) {
        return QLatin1String("???");
    }
    if (this->format_id == formatId) {
        return this->ext;
    }
    QString estimedExt = this->ext;
    QStringList ids = formatId.split(QChar('+'));
    for (auto id : ids) {
        for (auto format : formats) {
            if (id == format->format_id) {
                if (format->hasVideo()) {
                    return format->ext;
                }
                estimedExt = format->ext;
            }
        }
    }
    return estimedExt;
}

QString StreamInfo::formatId() const
{
    return this->format_id;
}

QList<StreamFormat*> StreamInfo::defaultFormats() const
{
    // Map avoids duplicate entries
    QMap<QString, StreamFormat*> map;
    for (auto format : formats) {
        if (format->hasVideo() && format->hasMusic()) {

            // The output list should be sorted in ascending order of
            // video resolution, then in ascending order of codec name
            auto unique_sort_identifier = QString("%0 %1 %2")
                    .arg(format->width, 16, 10, QChar('0'))
                    .arg(format->height, 16, 10, QChar('0'))
                    .arg(format->toString());

            map.insert(unique_sort_identifier, format);
        }
    }
    return map.values();
}

QList<StreamFormat*> StreamInfo::audioFormats() const
{
    QList<StreamFormat*> list;
    for (auto format : formats) {
        if (!format->hasVideo() && format->hasMusic()) {
            list.append(format);
        }
    }
    return list;
}

QList<StreamFormat*> StreamInfo::videoFormats() const
{
    QList<StreamFormat*> list;
    for (auto format : formats) {
        if (format->hasVideo() && !format->hasMusic()) {
            list.append(format);
        }
    }
    return list;
}

QString StreamInfo::debug_description() const
{
    QString descr;
    descr.append(QString("StreamInfo '%0' (%1, %2, %3, %4, %5, %6, %7, %8, %9, %10)")
                 .arg(_filename)
                 .arg(fulltitle)
                 .arg(title)
                 .arg(ext)
                 .arg(description)
                 .arg(thumbnail)
                 .arg(extractor)
                 .arg(extractor_key)
                 .arg(format_id)
                 .arg(playlist)
                 .arg(playlist_index));
    foreach (auto format, formats) {
        descr.append("\n");
        descr.append(format->debug_description());
    }
    return descr;
}

/******************************************************************************
 ******************************************************************************/
static QString generateErrorMessage(QProcess::ProcessError error)
{
    QString message;
    switch(error) {
    case QProcess::FailedToStart:
        message = QObject::tr("The process failed to start.");
        break;
    case QProcess::Crashed:
        message = QObject::tr("The process crashed while attempting to run.");
        break;
    case QProcess::Timedout:
        message = QObject::tr("The process has timed out.");
        break;
    case QProcess::WriteError:
        message = QObject::tr("The process has encountered a write error.");
        break;
    case QProcess::ReadError:
        message = QObject::tr("The process has encountered a read error.");
        break;
    case QProcess::UnknownError:
        message = QObject::tr("The process has encountered an unknown error.");
        break;
    }
    return message;
}

static QString toString(QProcess *process)
{
    if (!process) {
        return QString("ERROR: invalid process");
    }
    return QString("[pid:%0] %1 %2 %3")
            .arg(process->processId())
            .arg(process->workingDirectory())
            .arg(process->program())
            .arg(process->arguments().join(" "));
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

char *toString(const StreamInfo &streamInfo)
{
    // bring QTest::toString overloads into scope:
    using QTest::toString;

    // delegate char* handling to QTest::toString(QByteArray):
    return toString(streamInfo.debug_description());
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

QDebug operator<<(QDebug dbg, const StreamInfo &streamInfo)
{
    dbg.nospace() << streamInfo.debug_description();
    return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const StreamInfo *streamInfo)
{
    if (streamInfo)
        // Here, noquote() is called,
        // because it doesn't escape non-text characters contrary to nospace()
        // Each streamInfo should appear in a separated line
        dbg.noquote() << *streamInfo;
    else
        dbg.nospace() << QLatin1String("StreamInfo(): nullptr");
    return dbg.space();
}
#endif
