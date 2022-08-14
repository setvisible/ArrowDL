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

            Format(
                    const QString &format_id,
                    const QString &ext,
                    const QString &formatNote,
                    int filesize,
                    const QString &acodec,
                    int abr,
                    int asr,
                    const QString &vcodec,
                    int width,
                    int height,
                    int fps,
                    int tbr);

            bool operator!=(const Format &other) const;
            bool operator==(const Format &other) const
            {
                return format   == other.format
                        && formatId     == other.formatId
                        && url          == other.url
                        && ext          == other.ext
                        && formatNote   == other.formatNote
                        && filesize     == other.filesize
                        && acodec       == other.acodec
                        && abr          == other.abr
                        && asr          == other.asr
                        && vbr          == other.vbr
                        && vcodec       == other.vcodec
                        && width        == other.width
                        && height       == other.height
                        && resolution   == other.resolution
                        && dynamicRange == other.dynamicRange
                        && fps          == other.fps
                        && tbr          == other.tbr;
            }

            bool hasVideo() const;
            bool hasMusic() const;
            QString toString() const;

            QString debug_description() const;

            StreamFormatId formatId;    // (string): Format code specified by --format
            QString url;
            QString ext;                // (string): Video filename extension
            QString format;
            QString formatNote;         // (string): Additional info about the format
            int filesize{};             // (numeric): The number of bytes, if known in advance
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
            bool operator!=(const Subtitle &other) const;
            bool operator==(const Subtitle &other) const
            {
                return languageCode == other.languageCode
                        && ext == other.ext
                        && url == other.url
                        && data == other.data
                        && languageName == other.languageName
                        && isAutomatic == other.isAutomatic;
            }

            QString languageCode;
            QString ext;
            QString url;
            QString data;
            QString languageName;
            bool isAutomatic{false};  // auto-generated (automatic caption)
        };

        bool operator!=(const Data &other) const;
        bool operator==(const Data &other) const
        {
            return id                   == other.id
                    && originalFilename == other.originalFilename
                    && subtitles        == other.subtitles
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
                    ;
        }

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
        QString fulltitle;              // (string): Video title
        QString defaultTitle;           // (string): Video title
        QString defaultSuffix;          // (string): Video filename suffix (complete extension)
        QString description;            // (string): Video description
        QString thumbnail;              // (string): thumbnail URL
        QString extractor;              // (string): Name of the extractor
        QString extractor_key;          // (string): Key name of the extractor
        StreamFormatId defaultFormatId; // (string): Format code specified by --format
        QList<Format> formats;      // List of available formats, ordered from worst to best quality
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
            /// \todo since C++20: auto operator<=>(const Overview&) const = default;
            bool operator!=(const Overview &other) const;
            bool operator==(const Overview &other) const
            {
                return skipVideo == other.skipVideo
                        && markWatched == other.markWatched;
            }
            bool skipVideo{false};
            bool markWatched{false};
        };
        struct Subtitle
        {
            bool operator!=(const Subtitle &other) const;
            bool operator==(const Subtitle &other) const
            {
                return writeSubtitle == other.writeSubtitle
                        && isAutoGenerated == other.isAutoGenerated
                        && extensions == other.extensions
                        && languages == other.languages
                        && convert == other.convert;
            }
            QString extensions;
            QString languages;
            QString convert;
            bool writeSubtitle{false};
            bool isAutoGenerated{false};
        };
        struct Chapter
        {
            bool operator!=(const Chapter &other) const;
            bool operator==(const Chapter &other) const
            {
                return writeChapters == other.writeChapters;
            }
            bool writeChapters{false};
        };
        struct Thumbnail
        {
            bool operator!=(const Thumbnail &other) const;
            bool operator==(const Thumbnail &other) const
            {
                return writeDefaultThumbnail == other.writeDefaultThumbnail;
            }
            bool writeDefaultThumbnail{false};
        };
        struct Comment
        {
            bool operator!=(const Comment &other) const;
            bool operator==(const Comment &other) const
            {
                return writeComment == other.writeComment;
            }
            bool writeComment{false};
        };
        struct Metadata
        {
            bool operator!=(const Metadata &other) const;
            bool operator==(const Metadata &other) const
            {
                return writeDescription == other.writeDescription
                        && writeMetadata == other.writeMetadata
                        && writeInternetShortcut == other.writeInternetShortcut;
            }
            bool writeDescription{false};
            bool writeMetadata{false};
            bool writeInternetShortcut{false};
        };
        struct Processing
        {
            bool operator!=(const Processing &other) const;
            bool operator==(const Processing &other) const
            {
                Q_UNUSED(other)
                return true;
            }
        };
        struct SponsorBlock
        {
            bool operator!=(const SponsorBlock &other) const;
            bool operator==(const SponsorBlock &other) const
            {
                Q_UNUSED(other)
                return true;
            }
        };

        bool operator!=(const Config &other) const;
        bool operator==(const Config &other) const
        {
            return overview == other.overview
                    && subtitle == other.subtitle
                    && chapter == other.chapter
                    && thumbnail == other.thumbnail
                    && comment == other.comment
                    && metadata == other.metadata
                    && processing == other.processing
                    && sponsorBlock == other.sponsorBlock;
        }

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

    bool operator!=(const StreamObject &other) const;
    bool operator==(const StreamObject &other) const
    {
        return m_data == other.m_data
                && m_config == other.m_config
                && m_error          == other.m_error
                && m_userTitle      == other.m_userTitle
                && m_userSuffix     == other.m_userSuffix
                && m_userFormatId   == other.m_userFormatId;
    }

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

    StreamObjectId id() const { return m_data.id; }

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
    ~Stream() Q_DECL_OVERRIDE;

    static QString version();
    static QString website();
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

    qint64 fileSizeInBytes() const;
    void setFileSizeInBytes(qint64 fileSizeInBytes);

    StreamObject::Config config() const;
    void setConfig(const StreamObject::Config &config);

    void initialize(const StreamObject &streamObject);

    QString command(int indent = 4) const;

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
    void parseStandardError(const QString &msg);
    void parseStandardOutput(const QString &msg);

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

    StreamObject::Config m_config;

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

class StreamAssetDownloader : public QObject
{
    Q_OBJECT
public:
    struct StreamFlatListItem
    {
        QString _type;
        StreamObjectId id;
        QString ie_key;
        QString title;
        QString url;
    };
    using StreamFlatList = QList<StreamFlatListItem>;
    using StreamDumpMap = QMap<StreamObjectId, StreamObject>;

    explicit StreamAssetDownloader(QObject *parent);
    ~StreamAssetDownloader() Q_DECL_OVERRIDE;

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
