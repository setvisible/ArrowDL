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

    auto operator<=>(const StreamFormatId&) const = default; /* required by QHash, QMap */

    bool operator<(const StreamFormatId &other) const
    {
        return toString() < other.toString();
    }

    /*!
     * \remark The first format must contain the video.
     * If the video is 299 and the audio is 251,
     * then pass "299+251", not "251+299".
     */
    QString toString() const;
    void fromString(const QString &format_id);

    QList<StreamFormatId> compoundIds() const;

    bool isEmpty() const;

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
Q_DECLARE_METATYPE(StreamFormatId)

inline size_t qHash(const StreamFormatId &key, size_t seed)
{
    return qHashMulti(seed, key.toString());
}

/*!
 * \typedef StreamObjectId
 * \brief Represents a 11-alphanumeric characters Unique Id (ex: "aBcDEfg1234")
 */
using StreamObjectId = QString;


/*!
 * \class StreamObject
 * \brief The StreamObject class stores
 * the stream attributes provided by the stream server,
 * and the options (selected by the user) for downloading the asset(s)
 * (audio, video, thumbnail, subtitle, etc.).
 */
class StreamObject
{
public:
    /*!
     * \brief The Data class stores the metadata provided by the server.
     */
    class Data
    {
    public:
        /*!
         * \brief The Format class stores the properties of an encoded stream.
         */
        class Format
        {
        public:
            Format() = default;
            ~Format() = default;
            Format(const Format &) = default;
            Format &operator=(const Format &) = default;

            Format(const QString &format_id,
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
                   int tbr);

            auto operator<=>(const Format&) const = default;

            bool hasVideo() const;
            bool hasMusic() const;
            QString toString() const;

            QString debug_description() const;

            StreamFormatId formatId;    // (string): Format code specified by --format
            QString url;
            QString ext;                // (string): Video filename extension
            QString format;
            QString formatNote;         // (string): Additional info about the format
            qsizetype filesize{};       // (numeric): The number of bytes, if known in advance
            QString acodec;             // (string): Name of the audio codec in use
            qreal abr;                  // (numeric): Average audio bitrate in KBit/s
            int asr{};                  // (numeric): Audio sampling rate in Hertz
            int vbr{};
            QString vcodec;             // (string): Name of the video codec in use
            int width{};                // (numeric): Width of the video
            int height{};               // (numeric): Height of the video
            QString resolution;
            QString dynamicRange;
            int fps{};                  // (numeric): Frame rate
            qreal tbr;                  // (numeric): Average bitrate of audio and video in KBit/s
        };

        class Subtitle
        {
        public:
            auto operator<=>(const Subtitle&) const = default;

            QString languageCode;
            QString ext;
            QString url;
            QString data;
            QString languageName;
            bool isAutomatic{false};  // auto-generated (automatic caption)
        };

        auto operator<=>(const Data&) const = default;

        QList<Format> defaultFormats() const;
        QList<Format> audioFormats() const;
        QList<Format> videoFormats() const;

        QList<Subtitle> subtitleLanguages() const;
        QList<QString> subtitleExtensions() const;

        QString debug_description() const;

        StreamObjectId id;              // (string): Video identifier
        QString originalFilename;

        QList<Subtitle> subtitles;

        QString webpage_url;            // (string): URL to the video webpage
        QString title;                  // (string): Video title
        QString defaultSuffix;          // (string): Video filename suffix (complete extension)
        QString description;            // (string): Video description
        QString artist;                 // (string): Artist(s) of the track
        QString album;                  // (string): Title of the album the track belongs to
        QString release_year;           // (numeric): Year (YYYY) when the album was released
        QString thumbnail;              // (string): thumbnail URL
        QString extractor;              // (string): Name of the extractor
        QString extractor_key;          // (string): Key name of the extractor
        StreamFormatId defaultFormatId; // (string): Format code specified by --format
        QList<Format> formats;          // List of available formats, ordered from worst to best quality
        QString playlist;               // (string): Name or id of the playlist that contains the video
        QString playlist_index;         // (numeric): Index of the video in the playlist padded with leading zeros according to the total length of the playlist
    };

    /*!
     * \brief The Config class stores the options of downloaded asset(s)
     */
    class Config
    {
    public:
        struct Overview
        {
            auto operator<=>(const Overview&) const = default;

            bool skipVideo{false};
            bool markWatched{false};
        };
        struct Subtitle
        {
            auto operator<=>(const Subtitle&) const = default;

            QString extensions;
            QString languages;
            QString convert;
            bool writeSubtitle{false};
            bool isAutoGenerated{false};
        };
        struct Chapter
        {
            auto operator<=>(const Chapter&) const = default;

            bool writeChapters{false};
        };
        struct Thumbnail
        {
            auto operator<=>(const Thumbnail&) const = default;

            bool writeDefaultThumbnail{false};
        };
        struct Comment
        {
            auto operator<=>(const Comment&) const = default;

            bool writeComment{false};
        };
        struct Metadata
        {
            auto operator<=>(const Metadata&) const = default;

            bool writeDescription{false};
            bool writeMetadata{false};
            bool writeInternetShortcut{false};
        };
        struct Processing
        {
            auto operator<=>(const Processing&) const = default;
        };
        struct SponsorBlock
        {
            auto operator<=>(const SponsorBlock&) const = default;
        };

        auto operator<=>(const Config&) const = default;

        Overview overview;
        Subtitle subtitle;
        Chapter chapter;
        Thumbnail thumbnail;
        Comment comment;
        Metadata metadata;
        Processing processing;
        SponsorBlock sponsorBlock;
    };

    StreamObject() = default;
    ~StreamObject() = default;
    StreamObject(const StreamObject &) = default;
    StreamObject &operator=(const StreamObject &) = default;

    auto operator<=>(const StreamObject&) const = default;

    enum Error{
        NoError = 0,
        ErrorJsonFormat,
        ErrorUnavailable
    };
    Error error() const;
    void setError(Error error);

    Data data() const;
    void setData(const Data &data);

    Config config() const;
    void setConfig(const Config &config);

    StreamObjectId id() const;

    qsizetype guestimateFullSize() const;
    qsizetype guestimateFullSize(const StreamFormatId &formatId) const;

    QString defaultTitle() const;

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

    bool isAvailable() const;

private:
    Data m_data;
    Config m_config;

    /* Error */
    Error m_error{NoError};

    /* Mutable data, modifiable by the user */
    QString m_userTitle;
    QString m_userSuffix;
    StreamFormatId m_userFormatId;
};

using StreamFormat = StreamObject::Data::Format;
using StreamSubtitle = StreamObject::Data::Subtitle;


/*!
 * \brief The Stream class is the main class to download a stream.
 */
class Stream : public QObject
{
    Q_OBJECT

public:
    explicit Stream(QObject *parent);
    ~Stream() override;

    static QString version();
    static QString website();
    static void setConcurrentFragments(int fragments);
    static void setLastModifiedTimeEnabled(bool enabled);
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

    qsizetype fileSizeInBytes() const;
    void setFileSizeInBytes(qsizetype fileSizeInBytes);

    StreamObject::Config config() const;
    void setConfig(const StreamObject::Config &config);

    void initialize(const StreamObject &streamObject);

    QString command(int indent = 4) const;

public slots:
    void start();
    void abort();

signals:
    void downloadMetadataChanged();
    void downloadProgress(qsizetype bytesReceived, qsizetype bytesTotal);
    void downloadFinished();
    void downloadError(QString message);

protected:
    /* For test purpose */
    void parseStandardError(const QString &msg);
    void parseStandardOutput(const QString &msg);
    QStringList splitMultiThreadMessages(const QString &raw) const;

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

    qsizetype m_bytesReceived;
    qsizetype m_bytesReceivedCurrentSection;
    qsizetype m_bytesTotal;
    qsizetype m_bytesTotalCurrentSection;

    QString m_fileBaseName;
    QString m_fileExtension;

    StreamObject::Config m_config;

    qsizetype _q_bytesTotal() const;
    bool isMergeFormat(const QString &suffix) const;
    QStringList arguments() const;

    void parseSingleStandardOutput(const QString &msg);
};

/******************************************************************************
 * UTILS CLASSES
 ******************************************************************************/

class StreamCleanCache : public QObject
{
    Q_OBJECT
public:
    explicit StreamCleanCache(QObject *parent);
    ~StreamCleanCache() override;

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
    QProcess *m_process = nullptr;
    bool m_isCleaned = false;
};

class StreamAssetDownloader : public QObject
{
    Q_OBJECT
public:
    struct StreamFlatListItem
    {
        auto operator<=>(const StreamFlatListItem&) const = default;

        QString _type = {};
        StreamObjectId id;
        QString ie_key;
        QString title;
        QString url;
    };

    using StreamFlatList = QList<StreamFlatListItem>;
    using StreamDumpMap = QMap<StreamObjectId, StreamObject>;

    explicit StreamAssetDownloader(QObject *parent);
    ~StreamAssetDownloader() override;

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

    static StreamObject parseDumpItemStdOut(const QByteArray &bytes);
    static StreamObject parseDumpItemStdErr(const QByteArray &bytes);

    static StreamFlatListItem parseFlatItem(const QByteArray &bytes);
    StreamObject createStreamObject(const StreamFlatListItem &flatItem) const;
};

class StreamVersion : public QThread
{
    Q_OBJECT
public:
    StreamVersion(QObject *parent = nullptr): QThread(parent) {}

    void run() override;
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
    ~StreamUpgrader() override;

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
    ~StreamExtractorListCollector() override;

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
Q_DECLARE_METATYPE(StreamFormat)

Q_DECLARE_METATYPE(StreamObject)


#ifdef QT_DEBUG
QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug dbg, const StreamFormat &streamFormat);
QDebug operator<<(QDebug dbg, const StreamFormat *streamFormat);
QDebug operator<<(QDebug dbg, const StreamObject &streamObject);
QDebug operator<<(QDebug dbg, const StreamObject *streamObject);
QT_END_NAMESPACE
#endif

#endif // CORE_STREAM_H
