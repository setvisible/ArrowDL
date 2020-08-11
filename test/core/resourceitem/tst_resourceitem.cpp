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

#include <Core/ResourceItem>

#include <QtCore/QUrl>

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_ResourceItem : public QObject
{
    Q_OBJECT

private slots:
    void localFileUrl_data();
    void localFileUrl();
};

/******************************************************************************
******************************************************************************/
void tst_ResourceItem::localFileUrl_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QString>("destination");
    QTest::addColumn<QString>("mask");
    QTest::addColumn<QString>("customFileName");
    QTest::addColumn<QUrl>("expected");

    QTest::newRow("linux file system")
            << "https://www.myweb.com/images/01/myimage.tar.gz"
            << "/home/me/documents/"
            << "*name*.*ext*"
            << ""
            << QUrl("file:///home/me/documents/myimage.tar.gz");

    QTest::newRow("windows file system")
            << "https://www.myweb.com/images/01/myimage.tar.gz"
            << "C:/Temp"
            << "*name*.*ext*"
            << ""
            << QUrl("file:///C:/Temp/myimage.tar.gz");

    QTest::newRow("windows file system")
            << "https://www.myweb.com/images/01/myimage.tar.gz"
            << "C:/Temp"
            << "*url*\\*subdirs*\\*name*.*ext*"
            << ""
            << QUrl("file:///C:/Temp/www.myweb.com/images/01/myimage.tar.gz");

    QTest::newRow("windows custom filename")
            << "https://www.myweb.com/images/01/myimage.tar.gz"
            << "C:/Temp"
            << "*name*.*ext*"
            << "A NEW FILE NAME"
            << QUrl("file:///C:/Temp/A NEW FILE NAME.gz");

    QTest::newRow("windows custom filename")
            << "https://www.myweb.com/images/01/myimage.tar.gz"
            << "C:/Temp"
            << "*url*\\*subdirs*\\*name*.*ext*"
            << "A NEW FILE NAME"
            << QUrl("file:///C:/Temp/www.myweb.com/images/01/A NEW FILE NAME.gz");

    /// \todo Unsupported Url Protocol

    // Magnet URI protocol
    // https://en.wikipedia.org/wiki/Magnet_URI_scheme
    QTest::newRow("magnet simple")
            << "magnet:?&dn=Some.Files"
            << "/home/me/documents/"
            << "*name*.*ext*"
            << ""
            << QUrl("file:///home/me/documents/Some.Files.torrent");

    QTest::newRow("magnet with info-hash and trackers")
            << "magnet:?"
               "&xt=urn:btih:0123456789abcdef0123456789abcdef"
               "&dn=My.File-ISO"
               "&tr=udp%3A%2F%2Ftracker.servertorrent.com%3A80"
               "&tr=udp%3A%2F%2Ftracker.torrentserver.com%3A80"
               "&tr=udp%3A%2F%2Ftracker.anothertorrentserver.com%3A80"
            << "/home/me/documents/"
            << "*name*.*ext*"
            << ""
            << QUrl("file:///home/me/documents/My.File-ISO.torrent");

    QTest::newRow("magnet escaped")
            << "magnet:?&dn=Some+Files+-+Hello.world%28Hi%29.1"
            << "/home/me/documents/"
            << "*name*.*ext*"
            << ""
            << QUrl("file:///home/me/documents/Some Files - Hello.world(Hi).1.torrent");

    QTest::newRow("magnet no display name")
            << "magnet:?&xt=urn:btih:0123456789abcdef0123456789abcdef"
            << "/home/me/documents/"
            << "*name*.*ext*"
            << ""
            << QUrl("file:///home/me/documents/[Wait... Downloading metadata...].torrent");

}

void tst_ResourceItem::localFileUrl()
{
    /* Source */
    QFETCH(QString, url);
    QFETCH(QString, destination);
    QFETCH(QString, mask);
    QFETCH(QString, customFileName);
    QFETCH(QUrl, expected);

    ResourceItem item;
    item.setUrl(url);
    item.setDestination(destination);
    item.setMask(mask);
    item.setCustomFileName(customFileName);
    QUrl actual = item.localFileUrl();

    QCOMPARE(actual, expected);
}


/******************************************************************************
******************************************************************************/

QTEST_APPLESS_MAIN(tst_ResourceItem)

#include "tst_resourceitem.moc"
