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

#include <Io/TextHandler>

#include "../../utils/fakedownloaditem.h"
#include "../../utils/fakedownloadmanager.h"

#include <QtCore/QDebug>
#include <QtTest/QtTest>


class tst_TextHandler : public QObject
{
    Q_OBJECT

private slots:
    void write();
    void read();

private:
    inline QByteArray simplify(QByteArray &str);
    inline QString toString(IDownloadItem *item) const;
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

inline QString tst_TextHandler::toString(IDownloadItem *item) const {
    return item->sourceUrl().toString();
}

/******************************************************************************
******************************************************************************/
void tst_TextHandler::write()
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
            "https://www.example.com/2019/10/DSC_8045.jpg\n"
            "https://www.example.com/2019/10/DSC_8046.jpg\n"
            "https://www.example.com/2019/10/DSC_8047.jpg\n"
            "https://www.example.com/2019/10/DSC_8048.jpg\n"
            "https://www.example.com/favicon.ico\n";

    TextHandler target;
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
void tst_TextHandler::read()
{
    // Given
    FakeDownloadManager manager;

    QByteArray byteArray=
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
    bool opened = target.read(&manager);

    // Then
    QVERIFY(opened);
    QCOMPARE(manager.downloadItems().count(), 5);
    QVERIFY(toString(manager.downloadItems().at(0)) == "https://www.example.com/2019/10/DSC_8045.jpg");
    QVERIFY(toString(manager.downloadItems().at(1)) == "https://www.example.com/2019/10/DSC_8046.jpg");
    QVERIFY(toString(manager.downloadItems().at(2)) == "https://www.example.com/2019/10/DSC_8047.jpg");
    QVERIFY(toString(manager.downloadItems().at(3)) == "https://www.example.com/2019/10/DSC_8048.jpg");
    QVERIFY(toString(manager.downloadItems().at(4)) == "https://www.example.com/favicon.ico");
}

/******************************************************************************
******************************************************************************/

QTEST_APPLESS_MAIN(tst_TextHandler)

#include "tst_texthandler.moc"
