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

#ifndef CORE_STREAM_H
#define CORE_STREAM_H

#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QSharedPointer>
#include <QtCore/QMetaType>

QT_BEGIN_NAMESPACE
class QDebug;
QT_END_NAMESPACE

class StreamInfos;
typedef QSharedPointer<StreamInfos> StreamInfosPtr;

class StreamFormat : public QObject
{
    Q_OBJECT
public:
    explicit StreamFormat(QObject *parent = nullptr);
    explicit StreamFormat(const StreamFormat &other);
    explicit StreamFormat(
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
            QObject *parent = nullptr);
    ~StreamFormat() Q_DECL_OVERRIDE;

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

class StreamInfos : public QObject
{
    Q_OBJECT
public:
    explicit StreamInfos(QObject *parent = nullptr);
    explicit StreamInfos(const StreamInfos &other);
    ~StreamInfos() Q_DECL_OVERRIDE;

    qint64 guestimateFullSize(const QString &format_id) const;

    QString safeTitle() const;
    QString fileBaseName() const;
    QString fileExtension(const QString &formatId) const;
    QString formatId() const;

    QList<StreamFormat*> defaultFormats() const;
    QList<StreamFormat*> audioFormats() const;
    QList<StreamFormat*> videoFormats() const;

    QString debug_description() const;

    QString _filename;
    QString fulltitle;      // (string): Video title
    QString title;          // (string): Video title
    QString ext;            // (string): Video filename extension
    QString description;    // (string): Video description
    QString thumbnail;      // (string): thumbnail URL
    QString extractor;      // (string): Name of the extractor
    QString extractor_key;  // (string): Key name of the extractor
    QString format_id;      //(string): Format code specified by --format
    QList<StreamFormat*> formats;
    QString playlist;       // (string): Name or id of the playlist that contains the video
    QString playlist_index; // (numeric): Index of the video in the playlist padded with leading zeros according to the total length of the playlist
};

class Stream : public QObject
{
    Q_OBJECT

public:
    explicit Stream(QObject *parent);
    ~Stream() Q_DECL_OVERRIDE;

    static QString version();
    static QString website();

    void clear();
    bool isEmpty();

    QString url() const;
    void setUrl(const QString &url);

    QString localFullOutputPath() const;
    void setLocalFullOutputPath(const QString &outputPath);

    QString selectedFormatId() const;
    void setSelectedFormatId(const QString &formatId);

    QString fileName() const;

    qint64 fileSizeInBytes() const;
    void setFileSizeInBytes(qint64 fileSizeInBytes);

    void initializeWithStreamInfos(const StreamInfos &streamInfos);

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
    void onErrorOccurred(QProcess::ProcessError error);
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onStandardOutputReady();
    void onStandardErrorReady();

private:
    QProcess *m_process;

    QString m_url;
    QString m_outputPath;
    QString m_selectedFormatId;

    qint64 m_bytesReceived;
    qint64 m_bytesReceivedCurrentSection;
    qint64 m_bytesTotal;
    qint64 m_bytesTotalCurrentSection;

    QString m_fileBaseName;
    QString m_fileExtension;
};

class StreamInfoDownloader : public QObject
{
    Q_OBJECT
public:
    explicit StreamInfoDownloader(QObject *parent);
    ~StreamInfoDownloader() Q_DECL_OVERRIDE;

    void runAsync(const QString &url);

signals:
    void error(QString errorMessage);
    void collected(StreamInfosPtr infos);

private slots:
    void onStarted();
    void onErrorOccurred(QProcess::ProcessError error);
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_process;
    bool parseJSON(const QByteArray &data, StreamInfos *infos);
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

private slots:
    void onStarted();
    void onErrorOccurred(QProcess::ProcessError error);

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
Q_DECLARE_TYPEINFO(StreamInfos, Q_PRIMITIVE_TYPE);

#ifdef QT_TESTLIB_LIB
char *toString(const StreamFormat &streamFormat);
char *toString(const StreamInfos &streamInfos);
#endif

Q_DECLARE_METATYPE(StreamFormat);
Q_DECLARE_METATYPE(StreamInfos);

#ifdef QT_DEBUG
QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug dbg, const StreamFormat &streamFormat);
QDebug operator<<(QDebug dbg, const StreamFormat *streamFormat);
QDebug operator<<(QDebug dbg, const StreamInfos &streamInfos);
QDebug operator<<(QDebug dbg, const StreamInfos *streamInfos);
QT_END_NAMESPACE
#endif

#endif // CORE_STREAM_H
