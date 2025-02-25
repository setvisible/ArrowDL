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
#include <Io/JsonHandler>

#include "../utils/dummyscheduler.h"

#include <QtCore/QDebug>
#include <QtTest/QtTest>


class tst_JsonHandler : public QObject
{
    Q_OBJECT

private slots:
    void write();
    void read();

private:
    inline QByteArray simplify(QByteArray &str);
    inline QString toString(AbstractJob *item) const;
};


/*!
 * Remove all \n, \r and spaces to make comparison ok on Windows and Linux
 */
inline QByteArray tst_JsonHandler::simplify(QByteArray &str)
{
    QString source = QString::fromUtf8(str);
    QString result = source.simplified();
    result.remove(' ');
    return result.toUtf8();
}

inline QString tst_JsonHandler::toString(AbstractJob *item) const {
    return item->sourceUrl().toString();
}

/******************************************************************************
******************************************************************************/
void tst_JsonHandler::write()
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
            "{"
            "\"links\": ["
            "{\"url\": \"https://www.example.com/2019/10/DSC_8045.jpg\"},"
            "{\"url\": \"https://www.example.com/2019/10/DSC_8046.jpg\"},"
            "{\"url\": \"https://www.example.com/2019/10/DSC_8047.jpg\"},"
            "{\"url\": \"https://www.example.com/2019/10/DSC_8048.jpg\"},"
            "{\"url\": \"https://www.example.com/favicon.ico\"}"
            "]"
            "}";
    expected = simplify(expected);

    JsonHandler target;
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
void tst_JsonHandler::read()
{
    // Given
    DummyScheduler scheduler;

    QByteArray byteArray =
            "  \t  \n"
            "  \t {     \n"
            "   \"links\": [  \t   \r\n" /* Windows \r\n here */
            "     {\"url\": \"https://www.example.com/2019/10/DSC_8045.jpg\"},     \n"
            "     {\"url\": \"https://www.example.com/2019/10/DSC_8046.jpg\"},     \n"
            "     {\"url\": \"https://www.example.com/2019/10/DSC_8047.jpg\"   \t }, \n"
            "\r\n"
            "\n"             /* Empty lines */
            "    \n"
            " \t {\"url\": \"https://www.example.com/2019/10/DSC_8048.jpg\"},     \n"
            "     {\"url\": \"https://www.example.com/favicon.ico\"}"
            "   ]     \n"
            "\t  }"; /* No endline here */


    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::ReadOnly | QIODevice::Text);

    JsonHandler target;
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

QTEST_APPLESS_MAIN(tst_JsonHandler)

#include "tst_jsonhandler.moc"
