/* - DownZemAll! - Copyright (C) 2019 Sebastien Vavassori
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

#include <Io/JsonHandler>

#include "../../utils/fakedownloaditem.h"
#include "../../utils/fakedownloadmanager.h"

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

/******************************************************************************
******************************************************************************/
void tst_JsonHandler::write()
{
    // Given
    FakeDownloadManager manager;
    manager.appendFakeJob(QUrl("https://www.example.com/2019/10/DSC_8045.jpg"));
    manager.appendFakeJob(QUrl("https://www.example.com/2019/10/DSC_8046.jpg"));
    manager.appendFakeJob(QUrl("https://www.example.com/2019/10/DSC_8047.jpg"));
    manager.appendFakeJob(QUrl("https://www.example.com/2019/10/DSC_8048.jpg"));
    manager.appendFakeJob(QUrl("https://www.example.com/favicon.ico"));

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
    bool opened = target.write(manager);
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
    FakeDownloadManager manager;

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
    bool opened = target.read(&manager);

    // Then
    QVERIFY(opened);
    QCOMPARE(manager.downloadItems().count(), 5);
}

/******************************************************************************
******************************************************************************/

QTEST_APPLESS_MAIN(tst_JsonHandler)

#include "tst_jsonhandler.moc"
