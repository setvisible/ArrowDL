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

#include "string_piece.h"
#include "parser.h"
#include "util.h"

#include <QtCore/QDebug>
#include <QtTest/QtTest>

#define INIT_GUMBO_STRING(varname, literal) \
    GumboStringPiece varname = {literal, sizeof(literal) - 1}

class tst_StringPiece : public GumboTest
{
    Q_OBJECT

private slots:
    void Equal();
    void NotEqual_DifferingCase();
    void NotEqual_Str1Shorter();
    void NotEqual_Str2Shorter();
    void NotEqual_DifferentText();
    void CaseEqual();
    void CaseNotEqual_Str2Shorter();
    void Copy();
};

/******************************************************************************
 ******************************************************************************/

void tst_StringPiece::Equal()
{
    INIT_GUMBO_STRING(str1, "foo");
    INIT_GUMBO_STRING(str2, "foo");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_string_equals(&str1, &str2));
}

void tst_StringPiece::NotEqual_DifferingCase()
{
    INIT_GUMBO_STRING(str1, "foo");
    INIT_GUMBO_STRING(str2, "Foo");
    /*EXPECT_FALSE*/ QVERIFY(!gumbo_string_equals(&str1, &str2));
}

void tst_StringPiece::NotEqual_Str1Shorter()
{
    INIT_GUMBO_STRING(str1, "foo");
    INIT_GUMBO_STRING(str2, "foobar");
    /*EXPECT_FALSE*/ QVERIFY(!gumbo_string_equals(&str1, &str2));
}

void tst_StringPiece::NotEqual_Str2Shorter()
{
    INIT_GUMBO_STRING(str1, "foobar");
    INIT_GUMBO_STRING(str2, "foo");
    /*EXPECT_FALSE*/ QVERIFY(!gumbo_string_equals(&str1, &str2));
}

void tst_StringPiece::NotEqual_DifferentText()
{
    INIT_GUMBO_STRING(str1, "bar");
    INIT_GUMBO_STRING(str2, "foo");
    /*EXPECT_FALSE*/ QVERIFY(!gumbo_string_equals(&str1, &str2));
}

void tst_StringPiece::CaseEqual()
{
    INIT_GUMBO_STRING(str1, "foo");
    INIT_GUMBO_STRING(str2, "fOO");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_string_equals_ignore_case(&str1, &str2));
}

void tst_StringPiece::CaseNotEqual_Str2Shorter()
{
    INIT_GUMBO_STRING(str1, "foobar");
    INIT_GUMBO_STRING(str2, "foo");
    /*EXPECT_FALSE*/ QVERIFY(!gumbo_string_equals_ignore_case(&str1, &str2));
}

void tst_StringPiece::Copy()
{
    GumboParser parser;
    parser._options = &kGumboDefaultOptions;
    INIT_GUMBO_STRING(str1, "bar");
    GumboStringPiece str2;
    gumbo_string_copy(&parser, &str2, &str1);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_string_equals(&str1, &str2));
    gumbo_parser_deallocate(&parser, (void*) str2.data);
}

/******************************************************************************
 ******************************************************************************/

QTEST_APPLESS_MAIN(tst_StringPiece)

#include "tst_stringpiece.moc"
