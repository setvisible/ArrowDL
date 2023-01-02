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

#include <Core/FileUtils>

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_FileUtils : public QObject
{
    Q_OBJECT

private slots:
    void validateFileName_data();
    void validateFileName();

    void cleanFileName_data();
    void cleanFileName();
};

/******************************************************************************
******************************************************************************/
void tst_FileUtils::validateFileName_data()
{
    QTest::addColumn<bool>("is_subdirectory_allowed");
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    // **************************************************
    // With allowed directories
    // **************************************************
    QTest::newRow("empty 2") << true << QString() << QString();
    QTest::newRow("empty 4") << true << "" << "";

    QTest::newRow("reserved dir+name 1") << true << "CON" << "file";
    QTest::newRow("reserved dir+name 2") << true << "/CON" << "/file";
    QTest::newRow("reserved dir+name 3") << true << "\\CON" << "\\file";
    QTest::newRow("reserved dir+name 4") << true << "/some/path/CON" << "/some/path/file";
    QTest::newRow("reserved dir+name 5") << true << "\\some/path\\CON" << "\\some/path\\file";

    QTest::newRow("reserved dir+name+ext 1") << true << "CON.txt" << "file.txt";
    QTest::newRow("reserved dir+name+ext 2") << true << "/CON.txt" << "/file.txt";
    QTest::newRow("reserved dir+name+ext 3") << true << "\\CON.txt" << "\\file.txt";
    QTest::newRow("reserved dir+name+ext 4") << true << "/some/path/CON.txt" << "/some/path/file.txt";
    QTest::newRow("reserved dir+name+ext 5") << true << "\\some/path\\CON.txt" << "\\some/path\\file.txt";


    // **************************************************
    // Without allowed directories
    // **************************************************
    QTest::newRow("empty 1") << false << QString() << QString();
    QTest::newRow("empty 3") << false << "" << "";

    QTest::newRow("forbidden bracket 1") << false << "my<file>.txt" << "my_file_.txt";
    QTest::newRow("forbidden bracket 2") << false << "my\"file\".txt" << "my_file_.txt";

    QTest::newRow("forbidden char 1") << false << "my:file.txt" << "my_file.txt";
    QTest::newRow("forbidden char 2") << false << "my?file.txt" << "my_file.txt";
    QTest::newRow("forbidden char 3") << false << "my*file.txt" << "my_file.txt";
    QTest::newRow("forbidden char 4") << false << "my|file.txt" << "my_file.txt";

    QTest::newRow("allowed bracket 1") << false << "my(file).txt" << "my(file).txt";
    QTest::newRow("allowed bracket 2") << false << "my{file}.txt" << "my{file}.txt";
    QTest::newRow("allowed bracket 3") << false << "my[file].txt" << "my[file].txt";
    QTest::newRow("allowed bracket 6") << false << "my'file'.txt" << "my'file'.txt";
    QTest::newRow("allowed bracket 7") << false << "my`file`.txt" << "my`file`.txt";

    QTest::newRow("allowed char 1") << false << "my,file.txt" << "my,file.txt";
    QTest::newRow("allowed char 2") << false << "my^file.txt" << "my^file.txt";
    QTest::newRow("allowed char 3") << false << "my@file.txt" << "my@file.txt";
    QTest::newRow("allowed char 4") << false << "my=file.txt" << "my=file.txt";
    QTest::newRow("allowed char 5") << false << "my~file.txt" << "my~file.txt";
    QTest::newRow("allowed char 6") << false << "my!file.txt" << "my!file.txt";
    QTest::newRow("allowed char 7") << false << "my;file.txt" << "my;file.txt";
    QTest::newRow("allowed char 8") << false << "my&file.txt" << "my&file.txt";
    QTest::newRow("allowed char 9") << false << "my#file.txt" << "my#file.txt";
    QTest::newRow("allowed char 10") << false << "my%file.txt" << "my%file.txt";
    QTest::newRow("allowed char 11") << false << "my$file.txt" << "my$file.txt";
    QTest::newRow("space") << false << "my   file.txt" << QString("my   file.txt");

    QTest::newRow("forbidden slash 1") << false << QString("/my/file.txt")
                                       << QString("_my_file.txt");
    QTest::newRow("forbidden slash 2") << false << QString("\\my\\file.txt")
                                       << QString("_my_file.txt");

    QTest::newRow("reserved dir+name 1") << false << "dir/file/" << "dir_file_";
    QTest::newRow("reserved dir+name 2") << false << "/dir/file/" << "_dir_file_";
    QTest::newRow("reserved dir+name 3") << false << "dir/file.txt" << "dir_file.txt";
    QTest::newRow("reserved dir+name 4") << false << "/dir/file.txt" << "_dir_file.txt";

    QTest::newRow("reserved dir+name 5") << false << "dir\\file\\" << "dir_file_";
    QTest::newRow("reserved dir+name 6") << false << "\\dir\\file\\" << "_dir_file_";
    QTest::newRow("reserved dir+name 7") << false << "dir\\file.txt" << "dir_file.txt";
    QTest::newRow("reserved dir+name 8") << false << "\\dir\\file.txt" << "_dir_file.txt";

    QTest::newRow("reserved name 1") << false << "CON" << "file";
    QTest::newRow("reserved name 2") << false << "PRN" << "file";
    QTest::newRow("reserved name 3") << false << "AUX" << "file";
    QTest::newRow("reserved name 4") << false << "NUL" << "file";
    QTest::newRow("reserved name 5") << false << "COM1" << "file";
    QTest::newRow("reserved name 6") << false << "COM2" << "file";
    QTest::newRow("reserved name 7") << false << "COM3" << "file";
    QTest::newRow("reserved name 8") << false << "COM4" << "file";
    QTest::newRow("reserved name 9") << false << "COM5" << "file";
    QTest::newRow("reserved name 10") << false << "COM6" << "file";
    QTest::newRow("reserved name 11") << false << "COM7" << "file";
    QTest::newRow("reserved name 12") << false << "COM8" << "file";
    QTest::newRow("reserved name 13") << false << "COM9" << "file";
    QTest::newRow("reserved name 14") << false << "LPT1" << "file";
    QTest::newRow("reserved name 15") << false << "LPT2" << "file";
    QTest::newRow("reserved name 16") << false << "LPT3" << "file";
    QTest::newRow("reserved name 17") << false << "LPT4" << "file";
    QTest::newRow("reserved name 18") << false << "LPT5" << "file";
    QTest::newRow("reserved name 19") << false << "LPT6" << "file";
    QTest::newRow("reserved name 20") << false << "LPT7" << "file";
    QTest::newRow("reserved name 21") << false << "LPT8" << "file";
    QTest::newRow("reserved name 22") << false << "LPT9" << "file";

    QTest::newRow("reserved name+ext 1") << false << "CON.txt" << "file.txt";
    QTest::newRow("reserved name+ext 2") << false << "PRN.txt" << "file.txt";
    QTest::newRow("reserved name+ext 3") << false << "AUX.txt" << "file.txt";
    QTest::newRow("reserved name+ext 4") << false << "NUL.txt" << "file.txt";
    QTest::newRow("reserved name+ext 5") << false << "COM1.txt" << "file.txt";
    QTest::newRow("reserved name+ext 6") << false << "COM2.txt" << "file.txt";
    QTest::newRow("reserved name+ext 7") << false << "COM3.txt" << "file.txt";
    QTest::newRow("reserved name+ext 8") << false << "COM4.txt" << "file.txt";
    QTest::newRow("reserved name+ext 9") << false << "COM5.txt" << "file.txt";
    QTest::newRow("reserved name+ext 10") << false << "COM6.txt" << "file.txt";
    QTest::newRow("reserved name+ext 11") << false << "COM7.txt" << "file.txt";
    QTest::newRow("reserved name+ext 12") << false << "COM8.txt" << "file.txt";
    QTest::newRow("reserved name+ext 13") << false << "COM9.txt" << "file.txt";
    QTest::newRow("reserved name+ext 14") << false << "LPT1.txt" << "file.txt";
    QTest::newRow("reserved name+ext 15") << false << "LPT2.txt" << "file.txt";
    QTest::newRow("reserved name+ext 16") << false << "LPT3.txt" << "file.txt";
    QTest::newRow("reserved name+ext 17") << false << "LPT4.txt" << "file.txt";
    QTest::newRow("reserved name+ext 18") << false << "LPT5.txt" << "file.txt";
    QTest::newRow("reserved name+ext 19") << false << "LPT6.txt" << "file.txt";
    QTest::newRow("reserved name+ext 20") << false << "LPT7.txt" << "file.txt";
    QTest::newRow("reserved name+ext 21") << false << "LPT8.txt" << "file.txt";
    QTest::newRow("reserved name+ext 22") << false << "LPT9.txt" << "file.txt";

    QTest::newRow("end char 1") << false << "my file." << "my file._";
    QTest::newRow("end char 2") << false << "my file " << "my file _";
    QTest::newRow("end char 3") << false << "my file ." << "my file ._";

    // Unicode UTF chars
    QTest::newRow("allowed utf char") << false << "لة الش.txt" << "لة الش.txt";
    QTest::newRow("allowed utf char") << false << "番剧.txt" << "番剧.txt";
}

void tst_FileUtils::validateFileName()
{
    QFETCH(bool, is_subdirectory_allowed);
    QFETCH(QString, input);
    QFETCH(QString, expected);
    auto actual = FileUtils::validateFileName(input, is_subdirectory_allowed);
    QCOMPARE(actual, expected);
}

/******************************************************************************
******************************************************************************/
void tst_FileUtils::cleanFileName_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("empty") << "" << "";
    QTest::newRow("simple") << "a" << "A";
    QTest::newRow("simple") << "A" << "A";

    QTest::newRow("") << "A - B" << "A - B";
    QTest::newRow("dot") << "A • B" << "A - B";
    QTest::newRow("") << "Rénet Schlüß" << "Rénet Schlüß";

    QTest::newRow("dash") << "Quikelol #lol" << "Quikelol #lol";

    QTest::newRow("stupid text") << "Live '01 (Official Video)" << "Live '01";
    QTest::newRow("stupid text") << "(Official Video) Live '01" << "Live '01";
    QTest::newRow("stupid text") << "Live (Official Video) '01" << "Live '01";

    // BUGFIX with "Construction" that becomes "filestruction"
    // https://www.youtube.com/watch?v=lSQ7pWUo3g4
    QTest::newRow("_Con_struction") << "Construction" << "Construction";

    QTest::newRow("capitalized") << "ALICE AND BOB" << "Alice And Bob";
    QTest::newRow("capitalized") << "ALICE and BOB" << "Alice And Bob";
    QTest::newRow("capitalized") << "alice and bob" << "Alice And Bob";
    QTest::newRow("capitalized") << "alice,and-bob" << "Alice,And-Bob";
    QTest::newRow("capitalized") << "'alice'" << "'Alice'";
    QTest::newRow("capitalized") << "\"alice\"" << "'Alice'";

    // Unicode UTF chars
    QTest::newRow("utf") << "لة الش" << "لة الش";
    QTest::newRow("utf") << "番剧" << "番剧";
}

void tst_FileUtils::cleanFileName()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);
    auto actual = FileUtils::cleanFileName(input);
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/
QTEST_APPLESS_MAIN(tst_FileUtils)

#include "tst_fileutils.moc"
