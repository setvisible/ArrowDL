/* - ArrowDL - Copyright (C) 2019-present Sebastien Vavassori
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

#include <Core/AbstractJob>
#include <Io/TextHandler>

#include "../utils/dummyscheduler.h"

#include <QtCore/QDebug>
#include <QtTest/QtTest>


class tst_TextHandler : public QObject
{
    Q_OBJECT

private slots:
    void write();
    void writeUTF8();
    void read();
    void readUTF8();

private:
    inline QByteArray simplify(QByteArray &str);
    inline QString toString(AbstractJob *item) const;
};


/*!
 * Remove all \r\n to make comparison ok on Windows and Linux
 */
inline QByteArray tst_TextHandler::simplify(QByteArray &str)
{
    QString source = QString::fromUtf8(str);
    QString result = source.remove('\r');
    return result.toUtf8();
}

inline QString tst_TextHandler::toString(AbstractJob *item) const {
    return item->sourceUrl().toString();
}

/******************************************************************************
******************************************************************************/
void tst_TextHandler::write()
{
    // Given
    DummyScheduler scheduler;
    scheduler.appendFakeJob(QUrl("https://www.example.com/2019/10/DSC_8045.jpg"));
    scheduler.appendFakeJob(QUrl("https://www.example.com/2019/10/DSC_8046.jpg"));
    scheduler.appendFakeJob(QUrl("https://www.example.com/2019/10/DSC_8047.jpg"));
    scheduler.appendFakeJob(QUrl("https://www.example.com/2019/10/DSC_8048.jpg"));
    scheduler.appendFakeJob(QUrl("https://www.example.com/favicon.ico"));

    /* QIODevice::Text forces convert to \r\n on Windows */
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);

    QByteArray expected =
            "https://www.example.com/2019/10/DSC_8045.jpg\n"
            "https://www.example.com/2019/10/DSC_8046.jpg\n"
            "https://www.example.com/2019/10/DSC_8047.jpg\n"
            "https://www.example.com/2019/10/DSC_8048.jpg\n"
            "https://www.example.com/favicon.ico\n";

    TextHandler target;
    target.setDevice(&buffer);

    // When
    bool opened = target.write(scheduler);
    QByteArray actual = simplify(byteArray);

    // Then
    QVERIFY(opened);
    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/
void tst_TextHandler::writeUTF8()
{
    // Given
    DummyScheduler scheduler;
    scheduler.appendFakeJob(QUrl("http://www.exemple.fr/Capture-d’écran-2017-06-20-à-10.22.38.png"));
    scheduler.appendFakeJob(QUrl("http://www.exemple.fr/Capture-d%E2%80%99e%CC%81cran-2017-06-20-a%CC%80-10.22.38.png"));

    /* QIODevice::Text forces convert to \r\n on Windows */
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly | QIODevice::Text);

    QByteArray expected =
            "http://www.exemple.fr/Capture-d’écran-2017-06-20-à-10.22.38.png\n"
            "http://www.exemple.fr/Capture-d’écran-2017-06-20-à-10.22.38.png\n";

    TextHandler target;
    target.setDevice(&buffer);

    // When
    bool opened = target.write(scheduler);
    QByteArray actual = simplify(byteArray);

    // Then
    QVERIFY(opened);
    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/
void tst_TextHandler::read()
{
    // Given
    DummyScheduler scheduler;

    QByteArray byteArray =
            "https://www.example.com/2019/10/DSC_8045.jpg\n"
            "     https://www.example.com/2019/10/DSC_8046.jpg       \n"
            "https://www.example.com/2019/10/DSC_8047.jpg\r\n" /* Windows \r\n here */
            "\thttps://www.example.com/2019/10/DSC_8048.jpg\t\t\r\n"
            "\r\n"
            "\n"             /* Empty lines */
            "    \n"
            "\t\t\r\n"
            "https://www.example.com/favicon.ico"; /* No endline here */

    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);

    TextHandler target;
    target.setDevice(&buffer);

    // When
    bool opened = target.read(&scheduler);

    // Then
    QVERIFY(opened);
    QCOMPARE(scheduler.jobs().count(), 5);
    QVERIFY(toString(scheduler.jobs().at(0)) == "https://www.example.com/2019/10/DSC_8045.jpg");
    QVERIFY(toString(scheduler.jobs().at(1)) == "https://www.example.com/2019/10/DSC_8046.jpg");
    QVERIFY(toString(scheduler.jobs().at(2)) == "https://www.example.com/2019/10/DSC_8047.jpg");
    QVERIFY(toString(scheduler.jobs().at(3)) == "https://www.example.com/2019/10/DSC_8048.jpg");
    QVERIFY(toString(scheduler.jobs().at(4)) == "https://www.example.com/favicon.ico");
}

/******************************************************************************
******************************************************************************/
void tst_TextHandler::readUTF8()
{
    // Given
    DummyScheduler scheduler;

    QByteArray byteArray =
            "http://www.exemple.fr/Capture-d’écran-2017-06-20-à-10.22.38.png\n";

    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);

    TextHandler target;
    target.setDevice(&buffer);

    // When
    bool opened = target.read(&scheduler);

    // Then
    QVERIFY(opened);
    QCOMPARE(scheduler.jobs().count(), 1);
    QVERIFY(toString(scheduler.jobs().at(0)) == "http://www.exemple.fr/Capture-d’écran-2017-06-20-à-10.22.38.png");
}

/******************************************************************************
******************************************************************************/

QTEST_APPLESS_MAIN(tst_TextHandler)

#include "tst_texthandler.moc"
