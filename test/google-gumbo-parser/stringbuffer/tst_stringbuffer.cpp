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

#include "../test_utils.h"

#include "string_buffer.h"
#include "util.h"

#include <QtCore/QDebug>
#include <QtTest/QtTest>

#define INIT_GUMBO_STRING(varname, literal) \
    GumboStringPiece varname = {literal, sizeof(literal) - 1}

class tst_StringBuffer : public GumboTest
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void Reserve();
    void AppendString();
    void AppendStringWithResize();
    void AppendCodepoint_1Byte();
    void AppendCodepoint_2Bytes();
    void AppendCodepoint_3Bytes();
    void AppendCodepoint_4Bytes();
    void ToString();

private:
    GumboStringBuffer buffer_;

    void nullTerminateBuffer();
};

/******************************************************************************
 ******************************************************************************/
void tst_StringBuffer::init()
{
    gumbo_string_buffer_init(&parser_, &buffer_);
}

void tst_StringBuffer::cleanup()
{
    gumbo_string_buffer_destroy(&parser_, &buffer_);
}

void tst_StringBuffer::nullTerminateBuffer()
{
    buffer_.data[buffer_.length++] = 0;
}

/******************************************************************************
 ******************************************************************************/
void tst_StringBuffer::Reserve()
{
    gumbo_string_buffer_reserve(&parser_, 21, &buffer_);
    /*EXPECT_EQ*/ QVERIFY(40 == buffer_.capacity);
    qstrcpy(buffer_.data, "01234567890123456789");
    buffer_.length = 20;
    nullTerminateBuffer();
    /*EXPECT_EQ*/ QVERIFY(21 == buffer_.length);
    /*EXPECT_STREQ*/ QCOMPARE("01234567890123456789", buffer_.data);
}

void tst_StringBuffer::AppendString()
{
    INIT_GUMBO_STRING(str, "01234567");
    gumbo_string_buffer_append_string(&parser_, &str, &buffer_);
    nullTerminateBuffer();
    /*EXPECT_STREQ*/ QCOMPARE("01234567", buffer_.data);
}

void tst_StringBuffer::AppendStringWithResize()
{
    INIT_GUMBO_STRING(str, "01234567");
    gumbo_string_buffer_append_string(&parser_, &str, &buffer_);
    gumbo_string_buffer_append_string(&parser_, &str, &buffer_);
    nullTerminateBuffer();
    /*EXPECT_STREQ*/ QCOMPARE("0123456701234567", buffer_.data);
}

void tst_StringBuffer::AppendCodepoint_1Byte()
{
    gumbo_string_buffer_append_codepoint(&parser_, 'a', &buffer_);
    nullTerminateBuffer();
    /*EXPECT_STREQ*/ QCOMPARE("a", buffer_.data);
}

void tst_StringBuffer::AppendCodepoint_2Bytes()
{
    gumbo_string_buffer_append_codepoint(&parser_, 0xE5, &buffer_);
    nullTerminateBuffer();
    /*EXPECT_STREQ*/ QCOMPARE("\xC3\xA5", buffer_.data);
}

void tst_StringBuffer::AppendCodepoint_3Bytes()
{
    gumbo_string_buffer_append_codepoint(&parser_, 0x39E7, &buffer_);
    nullTerminateBuffer();
    /*EXPECT_STREQ*/ QCOMPARE("\xE3\xA7\xA7", buffer_.data);
}

void tst_StringBuffer::AppendCodepoint_4Bytes()
{
    gumbo_string_buffer_append_codepoint(&parser_, 0x679E7, &buffer_);
    nullTerminateBuffer();
    /*EXPECT_STREQ*/ QCOMPARE("\xF1\xA7\xA7\xA7", buffer_.data);
}

void tst_StringBuffer::ToString()
{
    gumbo_string_buffer_reserve(&parser_, 8, &buffer_);
    qstrcpy(buffer_.data, "012345");
    buffer_.length = 7;

    char* dest = gumbo_string_buffer_to_string(&parser_, &buffer_);
    /*EXPECT_STREQ*/ QCOMPARE("012345", dest);
    gumbo_parser_deallocate(&parser_, dest);
}


/******************************************************************************
 ******************************************************************************/

QTEST_APPLESS_MAIN(tst_StringBuffer)

#include "tst_stringbuffer.moc"
