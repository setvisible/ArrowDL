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

//#include <Core/TorrentContext>
#include "../../../src/core/torrentcontext_p.h"

#include "libtorrent/bitfield.hpp"      // lt::typed_bitfield

#include <QtCore/QDebug>
#include <QtTest/QtTest>

using namespace Qt::Literals::StringLiterals;

class tst_TorrentContext : public QObject
{
    Q_OBJECT

private slots:
    void toBitArray_data();
    void toBitArray();
    void dump_invalid();
};

class FriendlyWorkerThread : public WorkerThread
{
    friend class tst_TorrentContext;
public:
    explicit FriendlyWorkerThread(QObject *parent) : WorkerThread(parent) {}
};

/******************************************************************************
 ******************************************************************************/
/**
 * Helper function to initialize a bitarray from a string
 */
static QBitArray QStringToQBitArray(const QString &str)
{
    QBitArray ba;
    ba.resize(str.length());
    int i;
    QChar tru('1');
    for (i = 0; i < str.length(); i++) {
        if (str.at(i) == tru) {
            ba.setBit(i, true);
        }
    }
    return ba;
}

void tst_TorrentContext::toBitArray_data()
{
    QTest::addColumn<QBitArray>("input");

    QTest::newRow("null") << QStringToQBitArray(QString());

    QTest::newRow("data0") << QStringToQBitArray(QString("0"));
    QTest::newRow("data1") << QStringToQBitArray(QString("1"));

    QTest::newRow("data2") << QStringToQBitArray(QString("000"));
    QTest::newRow("data3") << QStringToQBitArray(QString("111"));

    QTest::newRow("data4") << QStringToQBitArray(QString("11111111"));
    QTest::newRow("data5") << QStringToQBitArray(QString("00101100"));
    QTest::newRow("data6") << QStringToQBitArray(QString("11010011"));
    QTest::newRow("data7") << QStringToQBitArray(QString("11011011"));
    QTest::newRow("data8") << QStringToQBitArray(QString("11110111"));
    QTest::newRow("data9") << QStringToQBitArray(QString("01000010"));
    QTest::newRow("data10") << QStringToQBitArray(QString("11100011"));
    QTest::newRow("data11") << QStringToQBitArray(QString("10100001"));
    QTest::newRow("data12") << QStringToQBitArray(QString("11100011011"));
    QTest::newRow("data13") << QStringToQBitArray(QString("00101100111"));
    QTest::newRow("data14") << QStringToQBitArray(QString("01000010111"));
}

void tst_TorrentContext::toBitArray()
{
    // Given
    QFETCH(QBitArray, input);
    lt::typed_bitfield<lt::piece_index_t> pieces(input.size(), false);
    for (auto i = 0; i < input.size(); ++i) {
        if (input.testBit(i)) {
            pieces.set_bit(static_cast<lt::piece_index_t>(i));
        }
    }
    QBitArray expected = input;

    // When
    QBitArray actual = TorrentUtils::toBitArray(pieces);

    // Then
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/
void tst_TorrentContext::dump_invalid()
{
    // Given
    auto torrentFile("./data/ill-formed.torrent"_L1);
    QVERIFY(QFileInfo::exists(torrentFile));

    TorrentInitialMetaInfo expected;

    // When
    FriendlyWorkerThread target(this);
    TorrentInitialMetaInfo actual = target.dump(torrentFile);

    // Then
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/
QTEST_APPLESS_MAIN(tst_TorrentContext)

#include "tst_torrentcontext.moc"
