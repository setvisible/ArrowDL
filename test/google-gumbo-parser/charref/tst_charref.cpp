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

#include "../test_utils.h"

#include "char_ref.h"
#include "utf8.h"

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_CharRef : public GumboTest
{
    Q_OBJECT

private slots:
    void whitespace();
    void numericHex();
    void numericDecimal();
    void numericInvalidDigit();
    void numericNoSemicolon();
    void numericReplacement();
    void numericInvalid();
    void numericUtfInvalid();
    void namedReplacement();
    void namedReplacementNoSemicolon();
    void namedReplacementWithInvalidUtf8();
    void namedReplacementInvalid();
    //void namedReplacementInvalidNoSemicolon();
    void additionalAllowedChar();
    void inAttribute();
    void multiChars();
    void charAfter();

private:
    Utf8Iterator iter_;
    OneOrTwoCodepoints output_;

    bool consumeCharRef(const char* input);
    bool consumeCharRef(const char* input, int additional_allowed_char, bool is_in_attribute);
};

/******************************************************************************
 ******************************************************************************/
bool tst_CharRef::consumeCharRef(const char* input)
{
    return consumeCharRef(input, ' ', false);
}

bool tst_CharRef::consumeCharRef(const char* input, int additional_allowed_char, bool is_in_attribute)
{
    text_ = input;
    utf8iterator_init(&parser_, input, strlen(input), &iter_);
    bool result = consume_char_ref(
                &parser_, &iter_, additional_allowed_char, is_in_attribute, &output_);
    fflush(stdout);
    return result;
}

/******************************************************************************
 ******************************************************************************/
void tst_CharRef::whitespace()
{
    /*EXPECT_TRUE*/ QVERIFY(consumeCharRef(" &nbsp;"));
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
}

void tst_CharRef::numericHex()
{
    /*EXPECT_TRUE*/ QVERIFY(consumeCharRef("&#x12ab;"));
    /*EXPECT_EQ*/ QCOMPARE(0x12ab, output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
}

void tst_CharRef::numericDecimal()
{
    /*EXPECT_TRUE*/ QVERIFY(consumeCharRef("&#1234;"));
    /*EXPECT_EQ*/ QCOMPARE(1234, output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
}

void tst_CharRef::numericInvalidDigit()
{
    errors_are_expected_ = true;
    /*EXPECT_FALSE*/ QVERIFY(!consumeCharRef("&#google"));
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
    /*EXPECT_EQ*/ QVERIFY('&' == utf8iterator_current(&iter_));
}

void tst_CharRef::numericNoSemicolon()
{
    errors_are_expected_ = true;
    /*EXPECT_FALSE*/ QVERIFY(!consumeCharRef("&#1234google"));
    /*EXPECT_EQ*/ QCOMPARE(1234, output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
    /*EXPECT_EQ*/ QVERIFY('g' == utf8iterator_current(&iter_));
}

void tst_CharRef::numericReplacement()
{
    errors_are_expected_ = true;
    /*EXPECT_FALSE*/ QVERIFY(!consumeCharRef("&#X82"));
    // Low quotation mark character.
    /*EXPECT_EQ*/ QCOMPARE(0x201A, output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
}

void tst_CharRef::numericInvalid()
{
    errors_are_expected_ = true;
    /*EXPECT_FALSE*/ QVERIFY(!consumeCharRef("&#xDA00"));
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
}

void tst_CharRef::numericUtfInvalid()
{
    errors_are_expected_ = true;
    /*EXPECT_FALSE*/ QVERIFY(!consumeCharRef("&#x007"));
    /*EXPECT_EQ*/ QCOMPARE(0x7, output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
}

void tst_CharRef::namedReplacement()
{
    /*EXPECT_TRUE*/ QVERIFY(consumeCharRef("&lt;"));
    /*EXPECT_EQ*/ QVERIFY('<' == output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
}

void tst_CharRef::namedReplacementNoSemicolon()
{
    errors_are_expected_ = true;
    /*EXPECT_FALSE*/ QVERIFY(!consumeCharRef("&gt"));
    /*EXPECT_EQ*/ QVERIFY('>' == output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
}

void tst_CharRef::namedReplacementWithInvalidUtf8()
{
    errors_are_expected_ = true;
    /*EXPECT_TRUE*/ QVERIFY(consumeCharRef("&\xc3\xa5"));
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
}

void tst_CharRef::namedReplacementInvalid()
{
    errors_are_expected_ = true;
    /*EXPECT_FALSE*/ QVERIFY(!consumeCharRef("&google;"));
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
    /*EXPECT_EQ*/ QVERIFY('&' == utf8iterator_current(&iter_));
}

// void tst_CharRef::namedReplacementInvalidNoSemicolon()
//{
//    /*EXPECT_FALSE*/ QVERIFY(!consumeCharRef("&google"));
//    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.first);
//    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
//    /*EXPECT_EQ*/ QVERIFY('&' == utf8iterator_current(&iter_));
//}

void tst_CharRef::additionalAllowedChar()
{
    /*EXPECT_TRUE*/ QVERIFY(consumeCharRef("&\"", '"', false));
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
    /*EXPECT_EQ*/ QVERIFY('&' == utf8iterator_current(&iter_));
}

void tst_CharRef::inAttribute()
{
    /*EXPECT_TRUE*/ QVERIFY(consumeCharRef("&noted", ' ', true));
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
    /*EXPECT_EQ*/ QVERIFY('&' == utf8iterator_current(&iter_));
}

void tst_CharRef::multiChars()
{
    /*EXPECT_TRUE*/ QVERIFY(consumeCharRef("&notindot;"));
    /*EXPECT_EQ*/ QCOMPARE(0x22F5, output_.first);
    /*EXPECT_EQ*/ QCOMPARE(0x0338, output_.second);
}

void tst_CharRef::charAfter()
{
    /*EXPECT_TRUE*/ QVERIFY(consumeCharRef("&lt;x"));
    /*EXPECT_EQ*/ QVERIFY('<' == output_.first);
    /*EXPECT_EQ*/ QCOMPARE(kGumboNoChar, output_.second);
    /*EXPECT_EQ*/ QVERIFY('x' == utf8iterator_current(&iter_));
}

/******************************************************************************
 ******************************************************************************/

QTEST_APPLESS_MAIN(tst_CharRef)

#include "tst_charref.moc"
