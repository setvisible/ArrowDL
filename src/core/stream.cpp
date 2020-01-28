/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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
#include <QtCore/QChar>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonParseError>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QMap>
#include <QtCore/QtMath>

static QString generateErrorMessage(QProcess::ProcessError error);
static QString toString(QProcess *process);

static const QString C_PROGRAM_NAME  = QLatin1String("youtube-dl.exe");
static const QString C_LEGAL_CHARS   = QLatin1String("' @()[]{}°#,.&");
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
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(onErrorOccurred(QProcess::ProcessError)));
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
void Stream::clear()
{
    m_url.clear();
    m_outputPath.clear();
    m_selectedFormatId.clear();
    m_fileSizeInBytes = 0;
    m_currentSectionBytes = 0;
    m_totalBytes = 0;

    m_fileBaseName.clear();
    m_fileExtension.clear();
}

bool Stream::isEmpty()
{
    return m_selectedFormatId.isEmpty();
}

/******************************************************************************
 ******************************************************************************/
void Stream::initializeWithStreamInfos(const StreamInfos &streamInfos)
{
    m_selectedFormatId = streamInfos.format_id;
    m_currentSectionBytes = 0;
    m_totalBytes = 0;
    m_fileSizeInBytes = streamInfos.guestimateFullSize(m_selectedFormatId);

    m_fileBaseName = streamInfos.fileBaseName();
    m_fileExtension = streamInfos.fileExtension();
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
    return m_fileSizeInBytes;
}

void Stream::setFileSizeInBytes(qint64 fileSizeInBytes)
{
    m_fileSizeInBytes = fileSizeInBytes;
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
        arguments << "--output" << m_outputPath
                  << "--no-check-certificate"
                  << "--no-overwrites"      // If option in settings !
                  << "--no-continue"
                  << "--ignore-config"
                  << "--format" << m_selectedFormatId
                  << m_url;
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

void Stream::onErrorOccurred(QProcess::ProcessError error)
{
    // Issue with configuration, or argument,
    // but not due to user input like invalid URL.
    // Also check that the path to "youtube-dl" program is valid.
    qDebug() << Q_FUNC_INFO << generateErrorMessage(error);
}

void Stream::onFinished(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
    emit downloadProgress(m_fileSizeInBytes, m_fileSizeInBytes);
    emit downloadFinished();
}

void Stream::onStandardOutputReady()
{
    QString data = m_process->readAllStandardOutput();
    data = data.simplified();
    parseStandardOutput(data);
}

void Stream::onStandardErrorReady()
{
    QString data = m_process->readAllStandardError();
    data = data.simplified();
    parseStandardError(data);
}

/******************************************************************************
 ******************************************************************************/
void Stream::parseStandardOutput(const QString &data)
{
    qDebug() << Q_FUNC_INFO << data;

    auto tokens = data.split(QChar::Space, QString::SkipEmptyParts);
    if (tokens.isEmpty()) {
        return;
    }
    if (tokens.at(0).toLower() != C_DOWNLOAD_msg_header) {
        return;
    }
    if ( tokens.count() > 2 &&
         tokens.at(1) == C_DOWNLOAD_next_section) {
        m_totalBytes = m_totalBytes + m_currentSectionBytes;
        emit downloadProgress(m_totalBytes, m_fileSizeInBytes);
        return;
    }

    if ( tokens.count() > 3 &&
         tokens.at(1).contains('%') &&
         tokens.at(2) == QLatin1String("of")) {

        const QString percentToken = tokens.at(1);
        const QString sizeToken= tokens.at(3);

        double percent = Format::parsePercentDecimal(percentToken);
        if (percent < 0) {
            qDebug() << Q_FUNC_INFO << "ERROR: Can't parse" << percentToken;
            return;
        }

        qint64 sizeInByte = Format::parseBytes(sizeToken);
        if (sizeInByte < 0) {
            qDebug() << Q_FUNC_INFO << "ERROR: Can't parse" << sizeToken;
            return;
        }

        if (m_fileSizeInBytes <= 0) {
            m_fileSizeInBytes = sizeInByte;
        }

        m_currentSectionBytes = qCeil((percent * sizeInByte) / 100.0);
    }
    auto bytes = m_totalBytes + m_currentSectionBytes;
    emit downloadProgress(bytes, m_fileSizeInBytes);
}

void Stream::parseStandardError(const QString &data)
{
    qDebug() << Q_FUNC_INFO << data;

    if ( data.startsWith(C_ERROR_msg_header_01, Qt::CaseInsensitive) ||
         data.startsWith(C_ERROR_msg_header_02, Qt::CaseInsensitive)) {

        emit downloadError(data);

    } else if ( data.startsWith(C_WARNING_msg_header_01, Qt::CaseInsensitive) ||
                data.startsWith(C_WARNING_msg_header_02, Qt::CaseInsensitive)) {

        if (data.contains(C_WARNING_merge_output_format, Qt::CaseInsensitive)) {
            m_fileExtension = "mkv";   // TODO change extension
            emit downloadMetadataChanged();
        }
    }
}

/******************************************************************************
 ******************************************************************************/
StreamInfoDownloader::StreamInfoDownloader(QObject *parent) : QObject(parent)
  , m_process(new QProcess(this))
{
    connect(m_process, SIGNAL(started()),
            this, SLOT(onStarted()));
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(onErrorOccurred(QProcess::ProcessError)));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onFinished(int, QProcess::ExitStatus)));
}

StreamInfoDownloader::~StreamInfoDownloader()
{
    m_process->kill();
    m_process->deleteLater();
}

void StreamInfoDownloader::runAsync(const QString &url)
{
    if (m_process->state() == QProcess::NotRunning) {
        m_process->start(C_PROGRAM_NAME, QStringList() << "--dump-json" << url);
        qDebug() << Q_FUNC_INFO << toString(m_process);
    }
}

void StreamInfoDownloader::onStarted()
{
    qDebug() << Q_FUNC_INFO;
}

void StreamInfoDownloader::onErrorOccurred(QProcess::ProcessError error)
{
    qDebug() << Q_FUNC_INFO << generateErrorMessage(error);
}

void StreamInfoDownloader::onFinished(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    //qDebug() << Q_FUNC_INFO << exitCode << exitStatus;
    if (exitCode != 0) {
        auto message = generateErrorMessage(m_process->error());
        emit error(message);
    } else {
        QByteArray data = m_process->readAllStandardOutput();
        StreamInfos *infos = new StreamInfos(this);
        if (parseJSON(data, infos)) {
            emit collected(infos);
        } else {
            emit error(tr("Couldn't parse JSON file."));
        }
    }
}

bool StreamInfoDownloader::parseJSON(const QByteArray &data, StreamInfos *infos)
{
    QJsonParseError ok;
    QJsonDocument loadDoc(QJsonDocument::fromJson(data, &ok));
    if (ok.error != QJsonParseError::NoError) {
        return false;
    }
    QJsonObject json = loadDoc.object();

    infos->_filename         = json[QLatin1String("_filename")].toString();
    infos->fulltitle         = json[QLatin1String("fulltitle")].toString();
    infos->title             = json[QLatin1String("title")].toString();
    infos->ext               = json[QLatin1String("ext")].toString();
    infos->description       = json[QLatin1String("description")].toString();
    infos->thumbnail         = json[QLatin1String("thumbnail")].toString();
    infos->extractor         = json[QLatin1String("extractor")].toString();
    infos->extractor_key     = json[QLatin1String("extractor_key")].toString();
    infos->format_id         = json[QLatin1String("format_id")].toString();
    QJsonArray jobsArray    = json[QLatin1String("formats")].toArray();
    for (int i = 0; i < jobsArray.size(); ++i) {
        StreamFormat *format = new StreamFormat(infos);
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
        infos->formats << format;
    }
    infos->playlist          = json[QLatin1String("playlist")].toString();
    infos->playlist_index    = json[QLatin1String("playlist_index")].toString();

    return true;
}

/******************************************************************************
 ******************************************************************************/
StreamExtractorListCollector::StreamExtractorListCollector(QObject *parent) : QObject(parent)
  , m_process(new QProcess(this))
{
    connect(m_process, SIGNAL(started()),
            this, SLOT(onStarted()));
    connect(m_process, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(onErrorOccurred(QProcess::ProcessError)));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onFinished(int, QProcess::ExitStatus)));
}

StreamExtractorListCollector::~StreamExtractorListCollector()
{
    m_process->kill();
    m_process->deleteLater();
}

void StreamExtractorListCollector::runAsync()
{
    if (m_process->state() == QProcess::NotRunning) {
        m_process->start(C_PROGRAM_NAME, QStringList() << "--list-extractors");
        qDebug() << Q_FUNC_INFO << toString(m_process);
    }
}

void StreamExtractorListCollector::onStarted()
{
    qDebug() << Q_FUNC_INFO;
}

void StreamExtractorListCollector::onErrorOccurred(QProcess::ProcessError error)
{
    qDebug() << Q_FUNC_INFO << generateErrorMessage(error);
}

void StreamExtractorListCollector::onFinished(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    if (exitCode != 0) {
        auto message = generateErrorMessage(m_process->error());
        emit error(message);
    } else {
        QString data = m_process->readAllStandardOutput();
        auto extractors = data.split("\n");
        emit collected(extractors);
    }
}

/******************************************************************************
 ******************************************************************************/
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

bool StreamFormat::hasVideo() const {
    return vcodec != C_NONE;
}
bool StreamFormat::hasMusic() const {
    return acodec != C_NONE;
}

QString StreamFormat::toString() const
{
    if (hasVideo() && hasMusic()) {
        return QObject::tr("Video %0 x %1 (%2), size: %3")
                .arg(width)
                .arg(height)
                .arg(format_note)
                .arg(Format::fileSizeToString(filesize));
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

/******************************************************************************
 ******************************************************************************/
qint64 StreamInfos::guestimateFullSize(const QString &format_id) const
{
    auto sizes = formatSizes();
    return guestimateFullSize(format_id, sizes);
}

qint64 StreamInfos::guestimateFullSize(
        const QString &format_id, const QMap<QString, qint64> &sizes)
{
    if (format_id.isEmpty()) {
        return -1;
    }
    qint64 estimedSize = 0;
    QStringList ids = format_id.split("+");
    for (auto id : ids) {
        estimedSize += sizes.value(id, 0);
    }
    return estimedSize;
}

QString StreamInfos::safeTitle() const
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
    ret = ret.replace(QRegExp("_+"), "_");
    return ret.simplified();
}

QString StreamInfos::fileBaseName() const
{
    return cleanFileName(this->title);
}

QString StreamInfos::fileExtension() const
{
    return this->ext;
}

QString StreamInfos::formatId() const
{
    return this->format_id;
}

QList<StreamFormat*> StreamInfos::defaultFormats() const
{
    QList<StreamFormat*> list;
    for (auto format : formats) {
        if (format->hasVideo() && format->hasMusic()) {
            list.append(format);
        }
    }
    return list;
}

QList<StreamFormat*> StreamInfos::audioFormats() const
{
    QList<StreamFormat*> list;
    for (auto format : formats) {
        if (!format->hasVideo() && format->hasMusic()) {
            list.append(format);
        }
    }
    return list;
}

QList<StreamFormat*> StreamInfos::videoFormats() const
{
    QList<StreamFormat*> list;
    for (auto format : formats) {
        if (format->hasVideo() && !format->hasMusic()) {
            list.append(format);
        }
    }
    return list;
}

QMap<QString, qint64> StreamInfos::formatSizes() const
{
    QMap<QString, qint64> map;
    for (auto format : formats) {
        map.insert(format->format_id, format->filesize);
    }
    return map;
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
        message = QObject::tr("The process has encountered a write error");
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
