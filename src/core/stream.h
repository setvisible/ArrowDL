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

class StreamFormat : public QObject
{
    Q_OBJECT
public:
    explicit StreamFormat(QObject *parent = nullptr) : QObject(parent) {}
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

    bool hasVideo() const;
    bool hasMusic() const;
    QString toString() const;

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
    explicit StreamInfos(QObject *parent = nullptr) : QObject(parent) {}
    ~StreamInfos() Q_DECL_OVERRIDE {}

    qint64 guestimateFullSize(const QString &format_id) const;

    static qint64 guestimateFullSize(const QString &format_id,
                                     const QMap<QString, qint64> &sizes);

    QString safeTitle() const;
    QString fileBaseName() const;
    QString fileExtension() const;
    QString formatId() const;

    QList<StreamFormat*> defaultFormats() const;
    QList<StreamFormat*> audioFormats() const;
    QList<StreamFormat*> videoFormats() const;

    QMap<QString, qint64> formatSizes() const;

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

    qint64 m_fileSizeInBytes;
    qint64 m_currentSectionBytes;
    qint64 m_totalBytes;

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
    void collected(StreamInfos* infos = nullptr);

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
    void collected(QStringList extractors);

private slots:
    void onStarted();
    void onErrorOccurred(QProcess::ProcessError error);
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_process;
};

#endif // CORE_STREAM_H
