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

#include <Core/Format>

#include "../../utils/biginteger.h"

#include <QtCore/QDebug>
#include <QtCore/QTime>
#include <QtTest/QtTest>

class tst_Format : public QObject
{
    Q_OBJECT

private slots:
    void remaingTimeToString_data();
    void remaingTimeToString();

    void fileSizeToString_data();
    void fileSizeToString();

    void fileSizeThousandSeparator_data();
    void fileSizeThousandSeparator();

    void currentSpeedToString_data();
    void currentSpeedToString();

    void parsePercentDecimal_data();
    void parsePercentDecimal();

    void parseBytes_data();
    void parseBytes();
};


/******************************************************************************
 ******************************************************************************/
void tst_Format::remaingTimeToString_data()
{
    QTest::addColumn<QTime>("time");
    QTest::addColumn<QString>("expected");

    QTest::newRow("invalid time") << QTime() << "--:--";
    QTest::newRow("invalid time") << QTime(0, 0, 60, 0) << "--:--";
    QTest::newRow("invalid time") << QTime(1236, 0) << "--:--";

    QTest::newRow("0 sec") << QTime(0, 0, 0, 0) << "00:01";
    QTest::newRow("0 sec") << QTime(0, 0, 0, 500) << "00:01";
    QTest::newRow("0 sec") << QTime(0, 0, 0, 999) << "00:01";
    QTest::newRow("1 sec") << QTime(0, 0, 1, 0) << "00:01";
    QTest::newRow("1 sec") << QTime(0, 0, 1, 500) << "00:01";
    QTest::newRow("1 sec") << QTime(0, 0, 1, 999) << "00:01";
    QTest::newRow("60 sec") << QTime(0, 0, 59, 999) << "00:59";
    QTest::newRow("60 sec") << QTime(0, 1, 0, 0) << "01:00";

}

void tst_Format::remaingTimeToString()
{
    QFETCH(QTime, time);
    QFETCH(QString, expected);
    QString actual = Format::remaingTimeToString(time);
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/
void tst_Format::fileSizeToString_data()
{
    QTest::addColumn<BigInteger>("size");
    QTest::addColumn<QString>("expected");

    QTest::newRow("zero") << BigInteger(0) << "0 bytes";
    QTest::newRow("negative zero") << BigInteger(-0) << "0 bytes";

    QTest::newRow("negative") << BigInteger(-1) << "Unknown";
    QTest::newRow("negative 1024 bytes") << BigInteger(-1024) << "Unknown";

    QTest::newRow("1 bytes") << BigInteger(1) << "1 byte";
    QTest::newRow("8 bytes") << BigInteger(8) << "8 bytes";
    QTest::newRow("256 bytes") << BigInteger(256) << "256 bytes";
    QTest::newRow("512 bytes") << BigInteger(512) << "512 bytes";
    QTest::newRow("1024 bytes") << BigInteger(1024) << "1 KB";
    QTest::newRow("1025 bytes") << BigInteger(1025) << "1 KB";
    QTest::newRow("10240 bytes") << BigInteger(10240) << "10 KB";
    QTest::newRow("123456 bytes") << BigInteger(123456) << "121 KB";
    QTest::newRow("123456789 bytes") << BigInteger(123456789) << "117.7 MB";
    QTest::newRow("1234567890 bytes") << BigInteger(1234567890) << "1.15 GB";
    QTest::newRow("1234567890123 bytes") << BigInteger(1234567890123) << "1.123 TB";
    QTest::newRow("1234567890123456 bytes") << BigInteger(1234567890123456) << "1122.833 TB";

    QTest::newRow("MIN") << BigInteger(INT64_MIN) << "Unknown";
    QTest::newRow("MAX") << BigInteger(INT64_MAX) << "Unknown";
}

void tst_Format::fileSizeToString()
{
    QFETCH(BigInteger, size);
    QFETCH(QString, expected);
    QString actual = Format::fileSizeToString(size.value);
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/
void tst_Format::fileSizeThousandSeparator_data()
{
    QTest::addColumn<BigInteger>("number");
    QTest::addColumn<QString>("expected");

    QTest::newRow("zero") << BigInteger(0) << "0";
    QTest::newRow("negative zero") << BigInteger(-0) << "0";
    QTest::newRow("negative") << BigInteger(-1) << "-1";
    QTest::newRow("negative -1024") << BigInteger(-1024) << "-1,024";

    QTest::newRow("1") << BigInteger(1) << "1";
    QTest::newRow("8") << BigInteger(8) << "8";
    QTest::newRow("256") << BigInteger(256) << "256";
    QTest::newRow("512") << BigInteger(512) << "512";
    QTest::newRow("1024") << BigInteger(1024) << "1,024";
    QTest::newRow("1025") << BigInteger(1025) << "1,025";
    QTest::newRow("10240") << BigInteger(10240) << "10,240";
    QTest::newRow("123456") << BigInteger(123456) << "123,456";
    QTest::newRow("123456789") << BigInteger(123456789) << "123,456,789";
    QTest::newRow("1234567890") << BigInteger(1234567890) << "1,234,567,890";
    QTest::newRow("1234567890123") << BigInteger(1234567890123) << "1,234,567,890,123";
    QTest::newRow("1234567890123456") << BigInteger(1234567890123456) << "1,234,567,890,123,456";
}

void tst_Format::fileSizeThousandSeparator()
{
    QFETCH(BigInteger, number);
    QFETCH(QString, expected);
    QString actual = Format::fileSizeThousandSeparator(number.value);
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/
void tst_Format::currentSpeedToString_data()
{
    QTest::addColumn<qreal>("speed");
    QTest::addColumn<QString>("expected");

    QTest::newRow("zero") << 0.0 << "0 KB/s";
    QTest::newRow("negative zero") << -0.0 << "0 KB/s";

    QTest::newRow("negative") << -1.0 << "-";
    QTest::newRow("negative 1024 bytes") << -1024.0 << "-";

    QTest::newRow("1 bytes") << 1.0 << "0 KB/s";
    QTest::newRow("8 bytes") << 8.0 << "0 KB/s";
    QTest::newRow("256 bytes") << 256.0 << "0 KB/s";
    QTest::newRow("512 bytes") << 512.0 << "0 KB/s";
    QTest::newRow("1024 bytes") << 1024.0 << "1 KB/s";
    QTest::newRow("123456 bytes") << 123456.0 << "121 KB/s";
    QTest::newRow("123456789 bytes") << 123456789.0 << "117.7 MB/s";
    QTest::newRow("1234567890 bytes") << 1234567890.0 << "1.15 GB/s";
    QTest::newRow("1234567890123 bytes") << 1234567890123.0 << "1149.78 GB/s";

    QTest::newRow("INFINITY") << qInf() << "-";
    QTest::newRow("NaN") << qQNaN() << "-";
    QTest::newRow("NaN") << qSNaN() << "-";
}

void tst_Format::currentSpeedToString()
{
    QFETCH(qreal, speed);
    QFETCH(QString, expected);
    QString actual = Format::currentSpeedToString(speed);
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/
void tst_Format::parsePercentDecimal_data()
{
    QTest::addColumn<QString>("str");
    QTest::addColumn<double>("expected");

    /* Invalid */
    QTest::newRow("invalid") << "" << -1.0;
    QTest::newRow("invalid") << QString() << -1.0;
    QTest::newRow("invalid") << "azerty" << -1.0;
    QTest::newRow("invalid") << "NaN" << -1.0;
    QTest::newRow("negative") << "-1.0%" << -1.0;

    /* Valid */
    QTest::newRow("zero") << "0%" << 0.0;
    QTest::newRow("zero") << "0.0%" << 0.0;
    QTest::newRow("zero") << "0.0 %" << 0.0;
    QTest::newRow("zero") << "0.0   %" << 0.0;

    QTest::newRow("8.2") << "8.2%" << 8.2;
    QTest::newRow("100.0") << "100.0%" << 100.0;
    QTest::newRow("100.0") << "1325.6654%" << 1325.6654;
}

void tst_Format::parsePercentDecimal()
{
    QFETCH(QString, str);
    QFETCH(double, expected);
    double actual = Format::parsePercentDecimal(str);
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/
void tst_Format::parseBytes_data()
{
    QTest::addColumn<QString>("str");
    QTest::addColumn<BigInteger>("expected");

    /* Invalid */
    QTest::newRow("invalid") << ""          << BigInteger(-1);
    QTest::newRow("invalid") << QString()   << BigInteger(-1);
    QTest::newRow("invalid") << "azerty"    << BigInteger(-1);
    QTest::newRow("invalid") << "NaN"       << BigInteger(-1);

    QTest::newRow("unitless") << "0"        << BigInteger(-1);
    QTest::newRow("unitless") << "0.0"      << BigInteger(-1);

    QTest::newRow("unknown") << "-"         << BigInteger(-1);
    QTest::newRow("unknown") << "-.-"       << BigInteger(-1);
    QTest::newRow("unknown") << "--.--"     << BigInteger(-1);
    QTest::newRow("unknown") << "..."       << BigInteger(-1);
    QTest::newRow("unknown") << "?"         << BigInteger(-1);

    /* Valid */
    QTest::newRow("zero") << "0  KiB" << BigInteger(0);
    QTest::newRow("zero") << "0.0KiB" << BigInteger(0);

    QTest::newRow("1 byte") << "1 B" << BigInteger(1);
    QTest::newRow("1 byte") << "1 byte" << BigInteger(1);
    QTest::newRow("2 bytes") << "2 bytes" << BigInteger(2);

    QTest::newRow("1 KB") << "1 KB" << BigInteger(1000);
    QTest::newRow("1 MB") << "1 MB" << BigInteger(1000*1000);
    QTest::newRow("1 GB") << "1 GB" << BigInteger(1000*1000*1000);

    QTest::newRow("1 KiB") << "1 KiB" << BigInteger(1024);
    QTest::newRow("1 MiB") << "1 MiB" << BigInteger(1024*1024);
    QTest::newRow("1 GiB") << "1 GiB" << BigInteger(1024*1024*1024);

    QTest::newRow("167.85MiB") << "167.85MiB" << BigInteger(176003482);
    QTest::newRow("167.85MiB") << "167.85 MiB" << BigInteger(176003482);
    QTest::newRow("167.85MiB") << "167.85    MiB" << BigInteger(176003482);

    QTest::newRow("estim") << "~55.43MiB" << BigInteger(58122568);
    QTest::newRow("estim") << "~55.43  MiB" << BigInteger(58122568);
    QTest::newRow("estim") << " ~ 55.43   MiB" << BigInteger(58122568);
}

void tst_Format::parseBytes()
{
    QFETCH(QString, str);
    QFETCH(BigInteger, expected);
    qint64 actual = Format::parseBytes(str);
    QCOMPARE(actual, expected.value);
}

/******************************************************************************
 ******************************************************************************/

QTEST_APPLESS_MAIN(tst_Format)

#include "tst_format.moc"
