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

#ifndef CORE_STREAM_H
#define CORE_STREAM_H

#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QSharedPointer>
#include <QtCore/QMetaType>
#include <QtCore/QThread>

QT_BEGIN_NAMESPACE
class QDebug;
QT_END_NAMESPACE


class StreamFormat
{
public:
    explicit StreamFormat();
    explicit StreamFormat(QString format_id,
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
                          int tbr);
    ~StreamFormat();

    bool operator==(const StreamFormat &other) const;
    bool operator!=(const StreamFormat &other) const;

    bool hasVideo() const;
    bool hasMusic() const;
    QString toString() const;

    QString debug_description() const;

    QString format_id;   // (string): Format code specified by --format
    QString ext;         // (string): Video filename extension
    QString format_note; // (string): Additional info about the format
    int filesize;        // (numeric): The number of bytes, if known in advance
    QString acodec;      // (string): Name of the audio codec in use
    int abr;             // (numeric): Average audio bitrate in KBit/s
    int asr;             // (numeric): Audio sampling rate in Hertz
    QString vcodec;      // (string): Name of the video codec in use
    int width;           // (numeric): Width of the video
    int height;          // (numeric): Height of the video
    int fps;             // (numeric): Frame rate
    int tbr;             // (numeric): Average bitrate of audio and video in KBit/s
};

class StreamInfo
{
public:
    enum Error{
        NoError = 0,
        ErrorJsonFormat
    };
    explicit StreamInfo();
    ~StreamInfo();

    qint64 guestimateFullSize() const;
    qint64 guestimateFullSize(const QString &defaultFormatId) const;

    QString title() const;

    QString fullFileName() const;
    QString fileBaseName() const;
    QString fileExtension() const;
    QString fileExtension(const QString &formatId) const;

    QString formatId() const;

    QList<StreamFormat> defaultFormats() const;
    QList<StreamFormat> audioFormats() const;
    QList<StreamFormat> videoFormats() const;

    QString debug_description() const;

    QString _filename;
    QString fulltitle;      // (string): Video title
    QString defaultTitle;   // (string): Video title
    QString defaultSuffix;  // (string): Video filename suffix (complete extension)
    QString description;    // (string): Video description
    QString thumbnail;      // (string): thumbnail URL
    QString extractor;      // (string): Name of the extractor
    QString extractor_key;  // (string): Key name of the extractor
    QString defaultFormatId;// (string): Format code specified by --format
    QList<StreamFormat> formats;
    QString playlist;       // (string): Name or id of the playlist that contains the video
    QString playlist_index; // (numeric): Index of the video in the playlist padded with leading zeros according to the total length of the playlist

    /* User data */
    QString userTitle;
    QString userSuffix;
    QString userFormatId;

    Error error;
};

class Stream : public QObject
{
    Q_OBJECT

public:
    explicit Stream(QObject *parent);
    ~Stream() Q_DECL_OVERRIDE;

    static QString version();
    static QString website();
    static void setUserAgent(const QString &userAgent);

    static bool matchesHost(const QString &host, const QStringList &regexHosts);

    void clear();
    bool isEmpty();

    QString url() const;
    void setUrl(const QString &url);

    QString localFullOutputPath() const;
    void setLocalFullOutputPath(const QString &outputPath);

    QString referringPage() const;
    void setReferringPage(const QString &referringPage);

    QString selectedFormatId() const;
    void setSelectedFormatId(const QString &formatId);

    QString fileName() const;

    qint64 fileSizeInBytes() const;
    void setFileSizeInBytes(qint64 fileSizeInBytes);

    void initializeWithStreamInfo(const StreamInfo &streamInfo);

public slots:
    void start();
    void abort();

signals:
    void downloadMetadataChanged();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
    void downloadError(QString message);

protected:
    /* For test purpose */
    void parseStandardError(const QString &data);
    void parseStandardOutput(const QString &data);

private slots:
    void onStarted();
    void onError(QProcess::ProcessError error);
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onStandardOutputReady();
    void onStandardErrorReady();

private:
    QProcess *m_process;

    QString m_url;
    QString m_outputPath;
    QString m_referringPage;
    QString m_selectedFormatId;

    qint64 m_bytesReceived;
    qint64 m_bytesReceivedCurrentSection;
    qint64 m_bytesTotal;
    qint64 m_bytesTotalCurrentSection;

    QString m_fileBaseName;
    QString m_fileExtension;

    qint64 _q_bytesTotal() const;
};

class StreamCleanCache : public QObject
{
    Q_OBJECT
public:
    explicit StreamCleanCache(QObject *parent);
    ~StreamCleanCache() Q_DECL_OVERRIDE;

    static QString cacheDir();

    void runAsync();
    bool isCleaned() const;

signals:
    void done();

private slots:
    void onStarted();
    void onError(QProcess::ProcessError error);
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_process;
    bool m_isCleaned;
};

class StreamInfoDownloader : public QObject
{
    Q_OBJECT
public:
    explicit StreamInfoDownloader(QObject *parent);
    ~StreamInfoDownloader() Q_DECL_OVERRIDE;

    void runAsync(const QString &url);
    void stop();

    bool isRunning() const;

    static QList<StreamInfo> parse(const QByteArray &data);

signals:
    void error(QString errorMessage);
    void collected(QList<StreamInfo> streamInfoList);

private slots:
    void onStarted();
    void onError(QProcess::ProcessError error);
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onCacheCleaned();

private:
    QProcess *m_process;
    StreamCleanCache *m_streamCleanCache;

    QString m_url;
    bool m_cancelled;

    static StreamInfo parseJSON(const QByteArray &data);
};

class AskStreamVersionThread : public QThread
{
    Q_OBJECT
public:
    AskStreamVersionThread(QObject *parent = nullptr): QThread(parent) {}

    void run() Q_DECL_OVERRIDE;
    void stop();

signals:
    void resultReady(const QString &s);

private:
    bool stopped = false;
};

class StreamUpgrader : public QObject
{
    Q_OBJECT
public:
    explicit StreamUpgrader(QObject *parent);
    ~StreamUpgrader() Q_DECL_OVERRIDE;

    void runAsync();

signals:
    void done();

private slots:
    void onStarted();
    void onError(QProcess::ProcessError error);
    void onStandardOutputReady();
    void onStandardErrorReady();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_process;
};

class StreamExtractorListCollector : public QObject
{
    Q_OBJECT
public:
    explicit StreamExtractorListCollector(QObject *parent);
    ~StreamExtractorListCollector() Q_DECL_OVERRIDE;

    void runAsync();

signals:
    void error(QString errorMessage);
    void collected(QStringList extractors, QStringList descriptions);
    void finished();

private slots:
    void onStarted();
    void onError(QProcess::ProcessError error);

    void onFinishedExtractors(int exitCode, QProcess::ExitStatus exitStatus);
    void onFinishedDescriptions(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_processExtractors;
    QProcess *m_processDescriptions;
    QStringList m_extractors;
    QStringList m_descriptions;

    void onFinished();
};


Q_DECLARE_TYPEINFO(StreamFormat, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(StreamInfo, Q_PRIMITIVE_TYPE);

#ifdef QT_TESTLIB_LIB
char *toString(const StreamFormat &streamFormat);
char *toString(const StreamInfo &streamInfo);
#endif

Q_DECLARE_METATYPE(StreamFormat);
Q_DECLARE_METATYPE(StreamInfo);

#ifdef QT_DEBUG
QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug dbg, const StreamFormat &streamFormat);
QDebug operator<<(QDebug dbg, const StreamFormat *streamFormat);
QDebug operator<<(QDebug dbg, const StreamInfo &streamInfo);
QDebug operator<<(QDebug dbg, const StreamInfo *streamInfo);
QT_END_NAMESPACE
#endif

#endif // CORE_STREAM_H
