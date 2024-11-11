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

#include <Core/Mask>

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_Mask : public QObject
{
    Q_OBJECT

private slots:
    void defaultMask();

    void fromUserInput_data();
    void fromUserInput();

    void customFileName_data();
    void customFileName();

    void interpret_data();
    void interpret();

    void interpretEscaped_data();
    void interpretEscaped();

    void interpretForbidden_data();
    void interpretForbidden();
};


/******************************************************************************
******************************************************************************/
void tst_Mask::defaultMask()
{
    const QString url = "https://www.myweb.com/images/01/myimage.tar.gz?id=1345&lang=eng";
    const QString mask = QString();
    const QString expected = "www.myweb.com/images/01/myimage.tar.gz";

    const QString actual = Mask::interpret(url, QString(), QString());

    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/
void tst_Mask::fromUserInput_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QUrl>("expected");

    QTest::newRow("empty") << "" << QUrl();
    QTest::newRow("trimmed") << "  \n  \t  \r \f  " << QUrl();
    QTest::newRow("trimmed hex encoding") << "%0A %0D %0A%0D %20%20" << QUrl();

    QTest::newRow("batch") << "https://www.example.org/IMG_[01-03].jpg"
                           << QUrl("https://www.example.org/IMG_[01-03].jpg");

    /* End of line */
    QUrl expectedEOL("https://www.example.org/image.jpg");
    QTest::newRow("EOL N") << "https://www.example.org/image.jpg\n" << expectedEOL;
    QTest::newRow("EOL R") << "https://www.example.org/image.jpg\r" << expectedEOL;
    QTest::newRow("EOL RN") << "https://www.example.org/image.jpg\r\n" << expectedEOL;
    QTest::newRow("EOL %") << "https://www.example.org/image.jpg%0A%0D%0D" << expectedEOL;
}

void tst_Mask::fromUserInput()
{
    QFETCH(QString, input);
    QFETCH(QUrl, expected);

    const QUrl actual = Mask::fromUserInput(input);

    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/
void tst_Mask::customFileName_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QString>("customFileName");
    QTest::addColumn<QString>("mask");
    QTest::addColumn<QString>("expected");

    const QString url = "https://www.myweb.com/images/01/myimage.tar.gz?id=1345&lang=eng";
    const QString mask = "*url*/*subdirs*/*name*.*ext*";

    QTest::newRow("empty") << url << QString() << mask << "www.myweb.com/images/01/myimage.tar.gz";
    QTest::newRow("simple") << url << "new_Name" << mask << "www.myweb.com/images/01/new_Name.gz";

    QTest::newRow("no *name*") << url << "new_Name" << "*url*/*subdirs*/*ext*" << "www.myweb.com/images/01/gz";

}

void tst_Mask::customFileName()
{
    QFETCH(QString, url);
    QFETCH(QString, customFileName);
    QFETCH(QString, mask);
    QFETCH(QString, expected);

    const QString actual = Mask::interpret(url, customFileName, mask);

    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/
void tst_Mask::interpret_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QString>("mask");
    QTest::addColumn<QString>("expected");

    const QString url = "https://www.myweb.com/images/01/myimage.tar.gz?id=1345&lang=eng";
    const QString mask = "*url*/*subdirs*/*name*.*ext*";

    /* Simples */
    QTest::newRow("simple ext") << url << "*ext*" << "gz";
    QTest::newRow("simple name") << url << "*name*" << "myimage.tar";
    QTest::newRow("simple url") << url << "*url*" << "www.myweb.com";
    QTest::newRow("simple curl") << url << "*curl*" << "www.myweb.com/images/01/myimage.tar.gz";
    QTest::newRow("simple flaturl") << url << "*flaturl*" << "www.myweb.com-images-01-myimage.tar.gz";
    QTest::newRow("simple subdirs") << url << "*subdirs*" << "images/01";
    QTest::newRow("simple flatsubdirs") << url << "*flatsubdirs*" << "images-01";
    QTest::newRow("simple qstring") << url << "*qstring*" << "id=1345&lang=eng";

    /* Composites */
    QTest::newRow("composite") << url << "*name*.*ext*" << "myimage.tar.gz";
    QTest::newRow("file separator slash") << url << "*url*/*subdirs*/*name*.*ext*" << "www.myweb.com/images/01/myimage.tar.gz";
    QTest::newRow("file separator backslash") << url << "*url*\\*subdirs*\\*name*.*ext*" << "www.myweb.com/images/01/myimage.tar.gz";

    /* Limit cases */
    QTest::newRow("no file")   << "https://www.myweb.com/images/" << mask << "www.myweb.com/images";
    QTest::newRow("no suffix") << "https://www.myweb.com/images/myimage" << mask << "www.myweb.com/images/myimage";
    QTest::newRow("no subdir") << "https://www.myweb.com/myimage.png" << mask << "www.myweb.com/myimage.png";
    QTest::newRow("no host")   << "https://myimage.png" << mask << "myimage.png";
    QTest::newRow("no prefix") << "myimage.png" << mask << "myimage.png";
    QTest::newRow("no host/suffix") << "image" << mask << "image";
    QTest::newRow("no basename") << "https://www.myweb.com/.image" << mask << "www.myweb.com/.image";

    /* Bad masks */
    QTest::newRow("name with trailing . and slash") << url << "///*name*..//./." << "myimage.tar";
    QTest::newRow("extension with trailing . and slash") << url << "///*ext*..//./." << "gz";
    QTest::newRow("name with duplicate slash") << url << "*url*/////*name*" << "www.myweb.com/myimage.tar";
    QTest::newRow("extension with duplicate slash") << url << "*url*/////*ext*" << "www.myweb.com/gz";
}

void tst_Mask::interpret()
{
    QFETCH(QString, url);
    QFETCH(QString, mask);
    QFETCH(QString, expected);

    const QString actual = Mask::interpret(url, QString(), mask);

    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/
void tst_Mask::interpretEscaped_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QString>("customFileName");
    QTest::addColumn<QString>("mask");
    QTest::addColumn<QString>("expected");

    const QString mask = "*name*.*ext*";

    QTest::newRow("escaped bracket")
            << "http://www.example.org/%28brackets%29.txt"
            << QString() << mask << "(brackets).txt";

}

void tst_Mask::interpretEscaped()
{
    QFETCH(QString, url);
    QFETCH(QString, customFileName);
    QFETCH(QString, mask);
    QFETCH(QString, expected);

    const QString actual = Mask::interpret(url, customFileName, mask);

    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/
void tst_Mask::interpretForbidden_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QString>("customFileName");
    QTest::addColumn<QString>("mask");
    QTest::addColumn<QString>("expected");

    const QString mask = "*url*/*subdirs*/*name*.*ext*";
    const QString nameMask = "*name*.*ext*";

    QTest::newRow("question mark (%3F) encoded")
            << "http://www.example.org/%3F/question_mark_%3F.jpg"
            << QString() << mask << "www.example.org/_/question_mark__.jpg";

    QTest::newRow("'?' is a query here 1")
            << "http://www.example.org/archive.tar.gz?id=1345&lang=eng"
            << QString() << mask << "www.example.org/archive.tar.gz";

    QTest::newRow("'?' is a query here 2")
            << "http://www.example.org/faq.html?#fragment"
            << QString() << mask << "www.example.org/faq.html";

    QTest::newRow("'?' is a query here 3")
            << "http://www.example.org/faq.html?"
            << QString() << mask << "www.example.org/faq.html";

    QTest::newRow("'?' is not a query here, all the segments must appear")
            << "http://www.example.org/?/data_(?).jpg"
            << QString() << mask << "www.example.org/_/data_(_).jpg";

    QTest::newRow("'#' is not a query fragment here, all the segments must appear")
            << "http://www.example.org/data#/file(?).jpg"
            << QString() << mask << "www.example.org/data_/file(_).jpg";

    QTest::newRow("reserved chars")
            << "http://www.example.org/data #5?/file:George_Hendrik_Breitner_(?).jpg"
            << QString() << mask << "www.example.org/data _5_/file_George_Hendrik_Breitner_(_).jpg";


    /* Windows reserved names */
    QTest::newRow("reserved <") << "file://<.jpg" << QString() << nameMask << "_.jpg";
    QTest::newRow("reserved >") << "file://>.jpg" << QString() << nameMask << "_.jpg";
    QTest::newRow("reserved :") << "file://:.jpg" << QString() << nameMask << "_.jpg";
    QTest::newRow("reserved \"") << "file://\".jpg" << QString() << nameMask << "_.jpg";
    QTest::newRow("reserved |") << "file://|.jpg" << QString() << nameMask << "_.jpg";
    QTest::newRow("reserved ?") << "file://?.jpg" << QString() << nameMask << "_.jpg";
    QTest::newRow("reserved *") << "file://*.jpg" << QString() << nameMask << "_.jpg";
    QTest::newRow("reserved #") << "file://#.jpg" << QString() << nameMask << "_.jpg";
}

void tst_Mask::interpretForbidden()
{
    QFETCH(QString, url);
    QFETCH(QString, customFileName);
    QFETCH(QString, mask);
    QFETCH(QString, expected);

    const QString actual = Mask::interpret(url, customFileName, mask);

    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/
QTEST_APPLESS_MAIN(tst_Mask)

#include "tst_mask.moc"
