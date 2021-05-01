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

#include "../test_utils.h"

#include "utf8.h"

#include "error.h"
#include "gumbo.h"
#include "parser.h"

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_Utf8 : public GumboTest
{
    Q_OBJECT

private slots:
    void EmptyString();
    void GetPosition_EmptyString();
    void Null();
    void OneByteChar();
    void ContinuationByte();
    void MultipleContinuationBytes();
    void OverlongEncoding();
    void OverlongEncodingWithContinuationByte();
    void TwoByteChar();
    void TwoByteChar2();
    void ThreeByteChar();
    void FourByteChar();
    void FourByteCharWithoutContinuationChars();
    void FiveByteCharIsError();
    void SixByteCharIsError();
    void SevenByteCharIsError();
    void Hex0xFFIsError();
    void InvalidControlCharIsError();
    void TruncatedInput();
    void Html5SpecExample();
    void MultipleEOFReads();
    void AsciiOnly();
    void NewlinePosition();
    void TabPositionFreshTabstop();
    void TabPositionMidTabstop();
    void ConfigurableTabstop();
    void CRLF();
    void CarriageReturn();
    void Matches();
    void MatchesOverflow();
    void MatchesEof();
    void MatchesCaseSensitivity();
    void MatchesCaseInsensitive();
    void MatchFollowedByNullByte();
    void MarkReset();

private:
    Utf8Iterator input_;

    void advance(int num_chars);
    void resetText(const char* text);
    GumboError* getFirstError();
    unsigned int getNumErrors();
};

/******************************************************************************
 ******************************************************************************/
void tst_Utf8::advance(int num_chars)
{
    for (int i = 0; i < num_chars; ++i) {
        utf8iterator_next(&input_);
    }
}

void tst_Utf8::resetText(const char* text)
{
    text_ = text;
    utf8iterator_init(&parser_, text, strlen(text), &input_);
}

GumboError* tst_Utf8::getFirstError()
{
    return static_cast<GumboError*>(parser_._output->errors.data[0]);
}

unsigned int tst_Utf8::getNumErrors()
{
    return parser_._output->errors.length;
}

/******************************************************************************
 ******************************************************************************/
void tst_Utf8::EmptyString()
{
    resetText("");
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::GetPosition_EmptyString()
{
    resetText("");
    GumboSourcePosition pos;

    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.line);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.column);
    /*EXPECT_EQ*/ QVERIFY(0 == pos.offset);
}

void tst_Utf8::Null()
{
    // Can't use ResetText, as the implicit strlen will choke on the null.
    text_ = "\0f";
    utf8iterator_init(&parser_, text_, 2, &input_);

    /*EXPECT_EQ*/ QCOMPARE(0, utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\0', *utf8iterator_get_char_pointer(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QVERIFY('f' == utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('f', *utf8iterator_get_char_pointer(&input_));
}

void tst_Utf8::OneByteChar()
{
    resetText("a");

    /*EXPECT_EQ*/ QCOMPARE(0, getNumErrors());
    /*EXPECT_EQ*/ QVERIFY('a' == utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('a', *utf8iterator_get_char_pointer(&input_));

    GumboSourcePosition pos;
    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.line);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.column);
    /*EXPECT_EQ*/ QVERIFY(0 == pos.offset);

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::ContinuationByte()
{
    resetText("\x85");
    errors_are_expected_ = true;

    /*EXPECT_EQ*/ QCOMPARE(1, getNumErrors());
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\x85', *utf8iterator_get_char_pointer(&input_));

    GumboError* error = getFirstError();
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_ERR_UTF8_INVALID, error->type);
    /*EXPECT_EQ*/ QCOMPARE('\x85', *error->original_text);
    /*EXPECT_EQ*/ QVERIFY(0x85 == error->v.codepoint);

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::MultipleContinuationBytes()
{
    resetText("a\x85\xA0\xC2x\x9A");
    errors_are_expected_ = true;

    /*EXPECT_EQ*/ QVERIFY('a' == utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QVERIFY('x' == utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
    utf8iterator_next(&input_);

    /*EXPECT_EQ*/ QCOMPARE(4, getNumErrors());
}

void tst_Utf8::OverlongEncoding()
{
    // \xC0\x75 = 11000000 01110101.
    resetText("\xC0\x75");
    errors_are_expected_ = true;

    /*ASSERT_EQ*/ QCOMPARE(1, getNumErrors());
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\xC0', *utf8iterator_get_char_pointer(&input_));

    GumboError* error = getFirstError();
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_ERR_UTF8_INVALID, error->type);
    /*EXPECT_EQ*/ QVERIFY(1 == error->position.line);
    /*EXPECT_EQ*/ QVERIFY(1 == error->position.column);
    /*EXPECT_EQ*/ QVERIFY(0 == error->position.offset);
    /*EXPECT_EQ*/ QCOMPARE('\xC0', *error->original_text);
    /*EXPECT_EQ*/ QVERIFY(0xC0 == error->v.codepoint);

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(0x75, utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\x75', *utf8iterator_get_char_pointer(&input_));

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::OverlongEncodingWithContinuationByte()
{
    // \xC0\x85 = 11000000 10000101.
    resetText("\xC0\x85");
    errors_are_expected_ = true;

    /*ASSERT_EQ*/ QCOMPARE(1, getNumErrors());
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\xC0', *utf8iterator_get_char_pointer(&input_));

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));

    GumboError* error = getFirstError();
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_ERR_UTF8_INVALID, error->type);
    /*EXPECT_EQ*/ QVERIFY(1 == error->position.line);
    /*EXPECT_EQ*/ QVERIFY(1 == error->position.column);
    /*EXPECT_EQ*/ QVERIFY(0 == error->position.offset);
    /*EXPECT_EQ*/ QCOMPARE('\xC0', *error->original_text);
    /*EXPECT_EQ*/ QVERIFY(0xC0 == error->v.codepoint);

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::TwoByteChar()
{
    // \xC3\xA5 = 11000011 10100101.
    resetText("\xC3\xA5o");

    /*EXPECT_EQ*/ QCOMPARE(0, getNumErrors());
    // Codepoint = 000 11100101 = 0xE5.
    /*EXPECT_EQ*/ QCOMPARE(0xE5, utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\xC3', *utf8iterator_get_char_pointer(&input_));

    GumboSourcePosition pos;
    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.line);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.column);
    /*EXPECT_EQ*/ QVERIFY(0 == pos.offset);

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QVERIFY('o' == utf8iterator_current(&input_));

    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.line);
    /*EXPECT_EQ*/ QVERIFY(2 == pos.column);
    /*EXPECT_EQ*/ QVERIFY(2 == pos.offset);
}

void tst_Utf8::TwoByteChar2()
{
    // \xC2\xA5 = 11000010 10100101.
    resetText("\xC2\xA5");

    /*EXPECT_EQ*/ QCOMPARE(0, getNumErrors());
    // Codepoint = 000 10100101 = 0xA5.
    /*EXPECT_EQ*/ QCOMPARE(0xA5, utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\xC2', *utf8iterator_get_char_pointer(&input_));

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::ThreeByteChar()
{
    // \xE3\xA7\xA7 = 11100011 10100111 10100111
    resetText("\xE3\xA7\xA7\xB0");
    errors_are_expected_ = true;

    /*EXPECT_EQ*/ QCOMPARE(0, getNumErrors());
    // Codepoint = 00111001 11100111 = 0x39E7
    /*EXPECT_EQ*/ QCOMPARE(0x39E7, utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\xE3', *utf8iterator_get_char_pointer(&input_));

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(1, getNumErrors());
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\xB0', *utf8iterator_get_char_pointer(&input_));

    GumboSourcePosition pos;
    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.line);
    /*EXPECT_EQ*/ QVERIFY(2 == pos.column);
    /*EXPECT_EQ*/ QVERIFY(3 == pos.offset);

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::FourByteChar()
{
    // \xC3\x9A = 11000011 10011010
    // \xF1\xA7\xA7\xA7 = 11110001 10100111 10100111 10100111
    resetText("\xC3\x9A\xF1\xA7\xA7\xA7");

    // Codepoint = 000 11011010 = 0xDA.
    /*EXPECT_EQ*/ QCOMPARE(0xDA, utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\xC3', *utf8iterator_get_char_pointer(&input_));

    utf8iterator_next(&input_);
    // Codepoint = 00110 01111001 11100111 = 0x679E7.
    /*EXPECT_EQ*/ QCOMPARE(0x679E7, utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\xF1', *utf8iterator_get_char_pointer(&input_));

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::FourByteCharWithoutContinuationChars()
{
    // \xF1\xA7\xA7\xA7 = 11110001 10100111 10100111 10100111
    resetText("\xF1\xA7\xA7-");
    errors_are_expected_ = true;

    /*EXPECT_EQ*/ QCOMPARE(1, getNumErrors());
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\xF1', *utf8iterator_get_char_pointer(&input_));

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QVERIFY('-' == utf8iterator_current(&input_));

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::FiveByteCharIsError()
{
    resetText("\xF6\xA7\xA7\xA7\xA7x");
    errors_are_expected_ = true;

    /*EXPECT_EQ*/ QCOMPARE(1, getNumErrors());
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));

    utf8iterator_next(&input_);
    utf8iterator_next(&input_);
    utf8iterator_next(&input_);
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QVERIFY('x' == utf8iterator_current(&input_));
}

void tst_Utf8::SixByteCharIsError()
{
    resetText("\xF8\xA7\xA7\xA7\xA7\xA7x");
    errors_are_expected_ = true;

    /*EXPECT_EQ*/ QCOMPARE(1, getNumErrors());
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));

    utf8iterator_next(&input_);
    utf8iterator_next(&input_);
    utf8iterator_next(&input_);
    utf8iterator_next(&input_);
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QVERIFY('x' == utf8iterator_current(&input_));
}

void tst_Utf8::SevenByteCharIsError()
{
    resetText("\xFC\xA7\xA7\xA7\xA7\xA7\xA7x");
    errors_are_expected_ = true;

    /*EXPECT_EQ*/ QCOMPARE(1, getNumErrors());
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));

    utf8iterator_next(&input_);
    utf8iterator_next(&input_);
    utf8iterator_next(&input_);
    utf8iterator_next(&input_);
    utf8iterator_next(&input_);
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QVERIFY('x' == utf8iterator_current(&input_));
}

void tst_Utf8::Hex0xFFIsError()
{
    resetText("\xFFx");
    errors_are_expected_ = true;

    /*EXPECT_EQ*/ QCOMPARE(1, getNumErrors());
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QVERIFY('x' == utf8iterator_current(&input_));
}

void tst_Utf8::InvalidControlCharIsError()
{
    resetText("\x1Bx");
    errors_are_expected_ = true;

    /*EXPECT_EQ*/ QCOMPARE(1, getNumErrors());
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QVERIFY('x' == utf8iterator_current(&input_));
}

void tst_Utf8::TruncatedInput()
{
    resetText("\xF1\xA7");
    errors_are_expected_ = true;

    /*EXPECT_EQ*/ QCOMPARE(1, getNumErrors());
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));

    GumboError* error = getFirstError();
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_ERR_UTF8_TRUNCATED, error->type);
    /*EXPECT_EQ*/ QVERIFY(1 == error->position.line);
    /*EXPECT_EQ*/ QVERIFY(1 == error->position.column);
    /*EXPECT_EQ*/ QVERIFY(0 == error->position.offset);
    /*EXPECT_EQ*/ QCOMPARE('\xF1', *error->original_text);
    /*EXPECT_EQ*/ QVERIFY(0xF1A7 == error->v.codepoint);

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::Html5SpecExample()
{
    // This example has since been removed from the spec, and the spec has been
    // changed to reference the Unicode Standard 6.2, 5.22 "Best practices for
    // U+FFFD substitution."
    resetText("\x41\x98\xBA\x42\xE2\x98\x43\xE2\x98\xBA\xE2\x98");
    errors_are_expected_ = true;

    /*EXPECT_EQ*/ QVERIFY('A' == utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QVERIFY('B' == utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QVERIFY('C' == utf8iterator_current(&input_));
    utf8iterator_next(&input_);

    // \xE2\x98\xBA = 11100010 10011000 10111010
    // Codepoint = 00100110 00111010 = 0x263A
    /*EXPECT_EQ*/ QCOMPARE(0x263A, utf8iterator_current(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(0xFFFD, utf8iterator_current(&input_));
    utf8iterator_next(&input_);
}

void tst_Utf8::MultipleEOFReads()
{
    resetText("a");
    advance(2);
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));

    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::AsciiOnly()
{
    resetText("hello");
    advance(4);

    /*EXPECT_EQ*/ QVERIFY('o' == utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('o', *utf8iterator_get_char_pointer(&input_));

    GumboSourcePosition pos;
    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.line);
    /*EXPECT_EQ*/ QVERIFY(5 == pos.column);
    /*EXPECT_EQ*/ QVERIFY(4 == pos.offset);

    advance(1);
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::NewlinePosition()
{
    resetText("a\nnewline");
    advance(1);

    // Newline itself should register as being at the end of a line.
    GumboSourcePosition pos;
    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.line);
    /*EXPECT_EQ*/ QVERIFY(2 == pos.column);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.offset);

    // The next character should be at the next line.
    advance(1);
    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(2 == pos.line);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.column);
    /*EXPECT_EQ*/ QVERIFY(2 == pos.offset);
}

void tst_Utf8::TabPositionFreshTabstop()
{
    resetText("a\n\ttab");
    advance(sizeof("a\n\t") - 1);

    GumboSourcePosition pos;
    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(2 == pos.line);
    /*EXPECT_EQ*/ QVERIFY(8 == pos.column);
    /*EXPECT_EQ*/ QVERIFY(3 == pos.offset);
}

void tst_Utf8::TabPositionMidTabstop()
{
    resetText("a tab\tinline");
    advance(sizeof("a tab\t") - 1);

    GumboSourcePosition pos;
    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.line);
    /*EXPECT_EQ*/ QVERIFY(8 == pos.column);
    /*EXPECT_EQ*/ QVERIFY(6 == pos.offset);
}

void tst_Utf8::ConfigurableTabstop()
{
    options_.tab_stop = 4;
    resetText("a\n\ttab");
    advance(sizeof("a\n\t") - 1);

    GumboSourcePosition pos;
    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(2 == pos.line);
    /*EXPECT_EQ*/ QVERIFY(4 == pos.column);
    /*EXPECT_EQ*/ QVERIFY(3 == pos.offset);
}

void tst_Utf8::CRLF()
{
    resetText("Windows\r\nlinefeeds");
    advance(sizeof("Windows") - 1);

    /*EXPECT_EQ*/ QVERIFY('\n' == utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\n', *utf8iterator_get_char_pointer(&input_));

    GumboSourcePosition pos;
    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.line);
    // The carriage return should be ignore in column calculations, treating the
    // CRLF combination as one character.
    /*EXPECT_EQ*/ QVERIFY(8 == pos.column);
    // However, it should not be ignored in computing offsets, which are often
    // used by other tools to index into the original buffer.  We don't expect
    // other unicode-aware tools to have the same \r\n handling as HTML5.
    /*EXPECT_EQ*/ QVERIFY(8 == pos.offset);
}

void tst_Utf8::CarriageReturn()
{
    resetText("Mac\rlinefeeds");
    advance(sizeof("Mac") - 1);

    /*EXPECT_EQ*/ QVERIFY('\n' == utf8iterator_current(&input_));
    // We don't change the original pointer, which is part of the const input
    // buffer.  original_text pointers will see a carriage return as original
    // written.
    /*EXPECT_EQ*/ QCOMPARE('\r', *utf8iterator_get_char_pointer(&input_));

    GumboSourcePosition pos;
    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.line);
    /*EXPECT_EQ*/ QVERIFY(4 == pos.column);
    /*EXPECT_EQ*/ QVERIFY(3 == pos.offset);

    advance(1);
    /*EXPECT_EQ*/ QVERIFY('l' == utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('l', *utf8iterator_get_char_pointer(&input_));

    utf8iterator_get_position(&input_, &pos);
    /*EXPECT_EQ*/ QVERIFY(2 == pos.line);
    /*EXPECT_EQ*/ QVERIFY(1 == pos.column);
    /*EXPECT_EQ*/ QVERIFY(4 == pos.offset);
}

void tst_Utf8::Matches()
{
    resetText("\xC2\xA5goobar");
    advance(1);
    /*EXPECT_TRUE*/ QVERIFY(utf8iterator_maybe_consume_match(&input_, "goo", 3, true));
    /*EXPECT_EQ*/ QVERIFY('b' == utf8iterator_current(&input_));
}

void tst_Utf8::MatchesOverflow()
{
    resetText("goo");
    /*EXPECT_FALSE*/ QVERIFY(!utf8iterator_maybe_consume_match(&input_, "goobar", 6, true));
    /*EXPECT_EQ*/ QVERIFY('g' == utf8iterator_current(&input_));
}

void tst_Utf8::MatchesEof()
{
    resetText("goo");
    /*EXPECT_TRUE*/ QVERIFY(utf8iterator_maybe_consume_match(&input_, "goo", 3, true));
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::MatchesCaseSensitivity()
{
    resetText("gooBAR");
    /*EXPECT_FALSE*/ QVERIFY(!utf8iterator_maybe_consume_match(&input_, "goobar", 6, true));
    /*EXPECT_EQ*/ QVERIFY('g' == utf8iterator_current(&input_));
}

void tst_Utf8::MatchesCaseInsensitive()
{
    resetText("gooBAR");
    /*EXPECT_TRUE*/ QVERIFY(utf8iterator_maybe_consume_match(&input_, "goobar", 6, false));
    /*EXPECT_EQ*/ QCOMPARE(-1, utf8iterator_current(&input_));
}

void tst_Utf8::MatchFollowedByNullByte()
{
    // Can't use ResetText, as the implicit strlen will choke on the null.
    text_ = "CDATA\0f";
    utf8iterator_init(&parser_, text_, 7, &input_);

    /*EXPECT_TRUE*/ QVERIFY(utf8iterator_maybe_consume_match(
                                &input_, "cdata", sizeof("cdata") - 1, false));

    /*EXPECT_EQ*/ QCOMPARE(0, utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('\0', *utf8iterator_get_char_pointer(&input_));
    utf8iterator_next(&input_);
    /*EXPECT_EQ*/ QVERIFY('f' == utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('f', *utf8iterator_get_char_pointer(&input_));
}

void tst_Utf8::MarkReset()
{
    resetText("this is a test");
    advance(5);
    /*EXPECT_EQ*/ QVERIFY('i' == utf8iterator_current(&input_));
    utf8iterator_mark(&input_);

    advance(3);
    /*EXPECT_EQ*/ QVERIFY('a' == utf8iterator_current(&input_));

    GumboError error;
    utf8iterator_fill_error_at_mark(&input_, &error);
    /*EXPECT_EQ*/ QCOMPARE('i', *error.original_text);
    /*EXPECT_EQ*/ QVERIFY(1 == error.position.line);
    /*EXPECT_EQ*/ QVERIFY(6 == error.position.column);
    /*EXPECT_EQ*/ QVERIFY(5 == error.position.offset);

    utf8iterator_reset(&input_);
    /*EXPECT_EQ*/ QVERIFY('i' == utf8iterator_current(&input_));
    /*EXPECT_EQ*/ QCOMPARE('i', *utf8iterator_get_char_pointer(&input_));

    GumboSourcePosition position;
    utf8iterator_get_position(&input_, &position);
    /*EXPECT_EQ*/ QVERIFY(1 == error.position.line);
    /*EXPECT_EQ*/ QVERIFY(6 == error.position.column);
    /*EXPECT_EQ*/ QVERIFY(5 == error.position.offset);
}

/******************************************************************************
 ******************************************************************************/

QTEST_APPLESS_MAIN(tst_Utf8)

#include "tst_utf8.moc"
