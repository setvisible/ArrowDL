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

#include <Core/Stream>

#include "../../utils/biginteger.h"
#include "../../utils/dummystreamfactory.h"

#include <QtCore/QDebug>
#include <QtTest/QSignalSpy>
#include <QtTest/QtTest>

class tst_Stream : public QObject
{
    Q_OBJECT

    static inline void VERIFY_PROGRESS_SIGNAL(
            const QSignalSpy &spy, int index,
            qint64 expectedBytesReceived, qint64 expectedBytesTotal)
    {
        QList<QVariant> arguments = spy.at(index);
        auto actualBytesReceived = arguments.at(0).toInt();
        auto actualBytesTotal = arguments.at(1).toInt();
        QCOMPARE(actualBytesReceived, expectedBytesReceived);
        QCOMPARE(actualBytesTotal, expectedBytesTotal);
    }

private slots:
    void readStandardOutput();
    void readStandardOutputWithTwoStreams();

    void readStandardError();

    void fileBaseName_data();
    void fileBaseName();

    void guestimateFullSize_data();
    void guestimateFullSize();

    void fileExtension_data();
    void fileExtension();

};

class FriendlyStream : public Stream
{
    friend class tst_Stream;
public:
    explicit FriendlyStream(QObject *parent) : Stream(parent) {}
};

/******************************************************************************
******************************************************************************/
void tst_Stream::readStandardOutput()
{
    // Given
    QSharedPointer<FriendlyStream> target(new FriendlyStream(this));
    QSignalSpy spyProgress(target.data(), SIGNAL(downloadProgress(qint64, qint64)));

    // When
    target->parseStandardOutput(" .\\youtube-dl.exe https://www.youtube.com/watch?v=jDQv2jTNL04");
    target->parseStandardOutput("[youtube] jDQv2jTNL04: Downloading webpage");
    target->parseStandardOutput("[youtube] jDQv2jTNL04: Downloading video info webpage");
    target->parseStandardOutput("WARNING: Requested formats are incompatible for merge and will be merged into mkv.");
    target->parseStandardOutput("[download] Destination: C’EST QUOI CE TRUC.mp4");
    target->parseStandardOutput("[download]   0.0% of 167.85MiB at  4.13MiB/s ETA 00:00");
    target->parseStandardOutput("[download]   8.2% of 167.85MiB at  4.13MiB/s ETA 00:03");
    target->parseStandardOutput("[download]  25.6% of 167.85MiB at  4.13MiB/s ETA 00:13");
    target->parseStandardOutput("[download]  78.9% of 167.85MiB at  4.13MiB/s ETA 00:33");
    target->parseStandardOutput("[download]  98.2% of 167.85MiB at  4.13MiB/s ETA 00:39");
    target->parseStandardOutput("[download] 100% of 167.85MiB in 00:41");

    // Then
    // 167.85 MiB = 176003482 bytes
    QCOMPARE(spyProgress.count(), 7);
    VERIFY_PROGRESS_SIGNAL(spyProgress, 0,         0,         0); // -idle-
    VERIFY_PROGRESS_SIGNAL(spyProgress, 1,         0, 176003482); //   0.0%
    VERIFY_PROGRESS_SIGNAL(spyProgress, 2,  14432286, 176003482); //   8.2%
    VERIFY_PROGRESS_SIGNAL(spyProgress, 3,  45056892, 176003482); //  25.6%
    VERIFY_PROGRESS_SIGNAL(spyProgress, 4, 138866748, 176003482); //  78.9%
    VERIFY_PROGRESS_SIGNAL(spyProgress, 5, 172835420, 176003482); //  98.2%
    VERIFY_PROGRESS_SIGNAL(spyProgress, 6, 176003482, 176003482); // 100.0%
}

/******************************************************************************
******************************************************************************/
void tst_Stream::readStandardOutputWithTwoStreams()
{
    // Given
    QSharedPointer<FriendlyStream> target(new FriendlyStream(this));
    QSignalSpy spyProgress(target.data(), SIGNAL(downloadProgress(qint64, qint64)));

    // When
    target->parseStandardOutput(" .\\youtube-dl.exe https://www.youtube.com/watch?v=jDQv2jTNL04");
    target->parseStandardOutput("[youtube] jDQv2jTNL04: Downloading webpage");
    target->parseStandardOutput("[youtube] jDQv2jTNL04: Downloading video info webpage");
    target->parseStandardOutput("WARNING: Requested formats are incompatible for merge and will be merged into mkv.");
    target->parseStandardOutput("[download] Destination: C’EST QUOI CE TRUC  (Je me balade sur Twitter)-jDQv2jTNL04.f299.mp4");
    target->parseStandardOutput("[download]   8.2% of 167.85MiB at  4.13MiB/s ETA 00:03");
    target->parseStandardOutput("[download]  25.6% of 167.85MiB at  4.13MiB/s ETA 00:13");
    target->parseStandardOutput("[download]  78.9% of 167.85MiB at  4.13MiB/s ETA 00:33");
    target->parseStandardOutput("[download]  98.2% of 167.85MiB at  4.13MiB/s ETA 00:39");
    target->parseStandardOutput("[download] 100% of 167.85MiB in 00:41");
    target->parseStandardOutput("[download] Destination: C’EST QUOI CE TRUC  (Je me balade sur Twitter)-jDQv2jTNL04.f251.webm");
    target->parseStandardOutput("[download] 100% of 8.75MiB in 00:02");
    target->parseStandardOutput("[ffmpeg] Merging formats into \"C’EST QUOI CE TRUC  (Je me balade sur Twitter)-jDQv2jTNL04.mkv\"");
    target->parseStandardOutput("Deleting original file C’EST QUOI CE TRUC  (Je me balade sur Twitter)-jDQv2jTNL04.f299.mp4 (pass -k to keep)");
    target->parseStandardOutput("Deleting original file C’EST QUOI CE TRUC  (Je me balade sur Twitter)-jDQv2jTNL04.f251.webm (pass -k to keep)");


    // Then
    //    QCOMPARE(spyProgress.count(), 6);

    //    VERIFY_PROGRESS_SIGNAL(spyProgress, 1, 1510, 1670045);
}

/******************************************************************************
******************************************************************************/
void tst_Stream::readStandardError()
{

}

/******************************************************************************
******************************************************************************/
void tst_Stream::fileBaseName_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("null") << QString() << QString();
    QTest::newRow("empty") << "" << QString();

    QTest::newRow("separator") << "Live 10/02/2018" << "Live 10_02_2018";
    QTest::newRow("separator") << "here\\we\\come" << "here_we_come";

    QTest::newRow("tab") << "\t here\twe\tcome \t" << "here we come";

    QTest::newRow("brackets") << "Windsor ['98]" << "Windsor ['98]";
    QTest::newRow("brackets") << "Windsor ('98)" << "Windsor ('98)";
    QTest::newRow("@")        << "Live @ Windsor" << "Live @ Windsor";
    QTest::newRow("extra _") << "_*_Cambridge_*_" << "_Cambridge_";

    QTest::newRow("double quote") << "\"Hello\" by Adele" << "'Hello' by Adele";
    QTest::newRow("simple quote") << "Live in Paris '79" << "Live in Paris '79";

    QTest::newRow("")
            << "\"Bohemian Rhapsody\" Steve Vai & Malmsteen & Zakk Wylde & Nuno@Atlantic City (11/30/18)"
            << "'Bohemian Rhapsody' Steve Vai & Malmsteen & Zakk Wylde & Nuno@Atlantic City (11_30_18)" ;
}

void tst_Stream::fileBaseName()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);

    StreamInfos target(this);
    target.title = input;
    target.fulltitle = input;

    auto actual = target.fileBaseName();

    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/
void tst_Stream::guestimateFullSize_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<BigInteger>("expected");

    QTest::newRow("null") << QString() << BigInteger(-1);
    QTest::newRow("empty") << "" << BigInteger(-1);
    QTest::newRow("default") << "244+140" << BigInteger(294311 + 280597);

    QTest::newRow("audio") << "18" << BigInteger(552999);
    QTest::newRow("audio") << "43" << BigInteger(287596);
    QTest::newRow("audio") << "140" << BigInteger(280597);

    QTest::newRow("audio+video") << "160+140" << BigInteger(63901 + 280597);
    QTest::newRow("audio+video") << "278+140" << BigInteger(53464 + 280597);

    /*
     * Invalid formats
     * In Youtube-DL, video ID must be the first ID.
     * But we accept them as valid here.
     */
    QTest::newRow("audio+video") << "140+160" << BigInteger(280597 + 63901);
    QTest::newRow("audio+video") << "140+278" << BigInteger(280597 + 53464);
}

void tst_Stream::guestimateFullSize()
{
    QFETCH(QString, input);
    QFETCH(BigInteger, expected);

    auto target = DummyStreamFactory::createDummyStreamInfos();
    qint64 actual = target->guestimateFullSize(input);

    QCOMPARE(actual, expected.value);
}

/******************************************************************************
******************************************************************************/
void tst_Stream::fileExtension_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("null") << QString() << "webm";
    QTest::newRow("empty") << "" << "webm";
    QTest::newRow("default") << "244+140" << "webm";

    QTest::newRow("audio") << "18" << "mp4";
    QTest::newRow("audio") << "43" << "webm";
    QTest::newRow("audio") << "140" << "m4a";

    QTest::newRow("audio+video") << "160+140" << "mp4";
    QTest::newRow("audio+video") << "278+140" << "webm";

    /*
     * Invalid formats
     * In Youtube-DL, video ID must be the first ID.
     * But we accept them as valid here.
     */
    QTest::newRow("audio+video") << "140+160" << "mp4";
    QTest::newRow("audio+video") << "140+278" << "webm";
}

void tst_Stream::fileExtension()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);

    auto target = DummyStreamFactory::createDummyStreamInfos();
    auto actual = target->fileExtension(input);

    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/
QTEST_APPLESS_MAIN(tst_Stream)

#include "tst_stream.moc"
