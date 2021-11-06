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
#include <QtCore/QMap>

QT_BEGIN_NAMESPACE
class QDebug;
class QUrl;
QT_END_NAMESPACE

/*!
 * \brief The StreamFormatId class represents a format identifier
 *
 * Ex: "mp4_h264_opus", "19", "214+299"
 *
 * It identifies an Audio stream, a Video stream, or a compound of
 * an Audio and Video streams.
 */
class StreamFormatId
{
public:
    StreamFormatId() = default;
    ~StreamFormatId() = default;
    StreamFormatId(const StreamFormatId &) = default;
    StreamFormatId &operator=(const StreamFormatId &) = default;

    StreamFormatId(const QString &format_id);

    /*!
     * \remark The first format must contain the video.
     * If the video is 299 and the audio is 251,
     * then pass "299+251", not "251+299".
     */
    QString toString() const;
    void fromString(const QString &format_id);

    QList<StreamFormatId> compoundIds() const;

    bool isEmpty() const;
    bool operator==(const StreamFormatId &other) const; /* required by QHash */
    bool operator!=(const StreamFormatId &other) const;

    bool operator<(const StreamFormatId &other) const; /* required by QMap */

    friend StreamFormatId operator+(StreamFormatId lhs, const StreamFormatId& rhs)
    {
        // Operator +:
        // ===========
        // - "friend X"     : Friend functions have no "this"
        // - "X lhs"        : Passing lhs by value helps optimize chained a+b+c
        // - "const X& rhs" : otherwise, both parameters may be const references
        return StreamFormatId(lhs.toString() + QChar('+') + rhs.toString());
    }
private:
    QStringList m_identifiers;
};

/* Enable the type to be used with QVariant. */
Q_DECLARE_METATYPE(StreamFormatId);

/* Note: qHash() must be declared inside the object's namespace */
inline uint qHash(const StreamFormatId &key, uint seed) {
    return qHash(key.toString(), seed);
}

/*!
 * \brief The StreamFormat class stores the properties of an encoded stream.
 */
class StreamFormat
{
public:
    StreamFormat() = default;
    ~StreamFormat() = default;
    StreamFormat(const StreamFormat &) = default;
    StreamFormat &operator=(const StreamFormat &) = default;

    StreamFormat(const QString &format_id,
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
                 int tbr);

    bool operator==(const StreamFormat &other) const;
    bool operator!=(const StreamFormat &other) const;

    bool hasVideo() const;
    bool hasMusic() const;
    QString toString() const;

    QString debug_description() const;

    StreamFormatId formatId;    // (string): Format code specified by --format
    QString ext;                // (string): Video filename extension
    QString format_note;        // (string): Additional info about the format
    int filesize{};             // (numeric): The number of bytes, if known in advance
    QString acodec;             // (string): Name of the audio codec in use
    int abr{};                  // (numeric): Average audio bitrate in KBit/s
    int asr{};                  // (numeric): Audio sampling rate in Hertz
    QString vcodec;             // (string): Name of the video codec in use
    int width{};                // (numeric): Width of the video
    int height{};               // (numeric): Height of the video
    int fps{};                  // (numeric): Frame rate
    int tbr{};                  // (numeric): Average bitrate of audio and video in KBit/s
};

/*!
 * \typedef StreamObjectId
 * \brief Represents a 11-alphanumeric characters Unique Id (ex: "aBcDEfg1234")
 */
using StreamObjectId = QString;

struct StreamFlatListItem
{
    QString _type;
    StreamObjectId id;
    QString ie_key;
    QString title;
    QString url;
};
using StreamFlatList = QList<StreamFlatListItem>;


/*!
 * \class StreamObject
 * \brief The StreamObject class represents the stream properties and options
 * provided by the stream server.
 */
class StreamObject
{
public:
    StreamObject() = default;
    ~StreamObject() = default;
    StreamObject(const StreamObject &) = default;
    StreamObject &operator=(const StreamObject &) = default;

    bool operator==(const StreamObject &other) const;
    bool operator!=(const StreamObject &other) const;

    enum Error{
        NoError = 0,
        ErrorJsonFormat,
        ErrorUnavailable
    };
    Error error() const;
    void setError(Error error);

    qint64 guestimateFullSize() const;
    qint64 guestimateFullSize(const StreamFormatId &formatId) const;

    QString title() const;
    void setTitle(const QString &title);

    QString fullFileName() const;
    QString fileBaseName() const;

    QString suffix() const;
    QString suffix(const StreamFormatId &formatId) const;
    void setSuffix(const QString &suffix);

    StreamFormatId formatId() const;
    void setFormatId(const StreamFormatId &formatId);
    QString formatToString() const;

    QList<StreamFormat> defaultFormats() const;
    QList<StreamFormat> audioFormats() const;
    QList<StreamFormat> videoFormats() const;

    bool isAvailable() const;

    QString debug_description() const;

    /* Immutable data, not modifiable by the user */
    StreamObjectId id;              // (string): Video identifier
    QString _filename;
    QString webpage_url;            // (string): URL to the video webpage
    QString fulltitle;              // (string): Video title
    QString defaultTitle;           // (string): Video title
    QString defaultSuffix;          // (string): Video filename suffix (complete extension)
    QString description;            // (string): Video description
    QString thumbnail;              // (string): thumbnail URL
    QString extractor;              // (string): Name of the extractor
    QString extractor_key;          // (string): Key name of the extractor
    StreamFormatId defaultFormatId; // (string): Format code specified by --format
    QList<StreamFormat> formats;
    QString playlist;               // (string): Name or id of the playlist that contains the video
    QString playlist_index;         // (numeric): Index of the video in the playlist padded with leading zeros according to the total length of the playlist


private:
    /* Error */
    Error m_error{NoError};

    /* User data, modifiable */
    QString m_userTitle;
    QString m_userSuffix;
    StreamFormatId m_userFormatId;
};

using StreamDumpMap = QMap<StreamObjectId, StreamObject>;

/*!
 * \brief The Stream class is the main class to download a stream.
 */
class Stream : public QObject
{
    Q_OBJECT

public:
    explicit Stream(QObject *parent);
    ~Stream() Q_DECL_OVERRIDE;

    static QString version();
    static QString website();
    static void setUserAgent(const QString &userAgent);
    static void setConnectionProtocol(int index);
    static void setConnectionTimeout(int secs);

    static bool matchesHost(const QString &host, const QStringList &regexHosts);

    void clear();
    bool isEmpty();

    QString url() const;
    void setUrl(const QString &url);

    QString localFullOutputPath() const;
    void setLocalFullOutputPath(const QString &outputPath);

    QString referringPage() const;
    void setReferringPage(const QString &referringPage);

    StreamFormatId selectedFormatId() const;
    void setSelectedFormatId(const StreamFormatId &formatId);

    QString fileName() const;

    qint64 fileSizeInBytes() const;
    void setFileSizeInBytes(qint64 fileSizeInBytes);

    void initialize(const StreamObject &streamObject);

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
    StreamFormatId m_selectedFormatId;

    qint64 m_bytesReceived;
    qint64 m_bytesReceivedCurrentSection;
    qint64 m_bytesTotal;
    qint64 m_bytesTotalCurrentSection;

    QString m_fileBaseName;
    QString m_fileExtension;

    qint64 _q_bytesTotal() const;
    bool isMergeFormat(const QString &suffix) const;
    QStringList arguments() const;
};

/******************************************************************************
 * UTILS CLASSES
 ******************************************************************************/

class StreamCleanCache : public QObject
{
    Q_OBJECT
public:
    explicit StreamCleanCache(QObject *parent);
    ~StreamCleanCache() Q_DECL_OVERRIDE;

    static QUrl cacheDir();

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

class StreamObjectDownloader : public QObject
{
    Q_OBJECT
public:
    explicit StreamObjectDownloader(QObject *parent);
    ~StreamObjectDownloader() Q_DECL_OVERRIDE;

    void runAsync(const QString &url);
    void stop();

    bool isRunning() const;

    static StreamDumpMap parseDumpMap(const QByteArray &stdoutBytes, const QByteArray &stderrBytes);
    static StreamFlatList parseFlatList(const QByteArray &stdoutBytes, const QByteArray &stderrBytes);

signals:
    void error(QString errorMessage);
    void collected(QList<StreamObject> streamObjects);

private slots:
    void onStarted();
    void onError(QProcess::ProcessError error);
    void onCacheCleaned();

    void onFinishedDumpJson(int exitCode, QProcess::ExitStatus exitStatus);
    void onFinishedFlatList(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_processDumpJson;
    QProcess *m_processFlatList;
    StreamCleanCache *m_streamCleanCache;
    QString m_url;
    bool m_cancelled;

    StreamDumpMap m_dumpMap;
    StreamFlatList m_flatList;

    void runAsyncDumpJson();
    void runAsyncFlatList();
    void onFinished();

    static StreamObject parseDumpItemStdOut(const QByteArray &data);
    static StreamObject parseDumpItemStdErr(const QByteArray &data);

    static StreamFlatListItem parseFlatItem(const QByteArray &data);
    StreamObject createStreamObject(const StreamFlatListItem &flatItem) const;
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
Q_DECLARE_TYPEINFO(StreamObject, Q_PRIMITIVE_TYPE);

#ifdef QT_TESTLIB_LIB
char *toString(const StreamFormat &streamFormat);
char *toString(const StreamObject &streamObject);
#endif

/* Enable the type to be used with QVariant. */
Q_DECLARE_METATYPE(StreamFormat);
Q_DECLARE_METATYPE(StreamObject);

#ifdef QT_DEBUG
QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug dbg, const StreamFormat &streamFormat);
QDebug operator<<(QDebug dbg, const StreamFormat *streamFormat);
QDebug operator<<(QDebug dbg, const StreamObject &streamObject);
QDebug operator<<(QDebug dbg, const StreamObject *streamObject);
QT_END_NAMESPACE
#endif

#endif // CORE_STREAM_H
