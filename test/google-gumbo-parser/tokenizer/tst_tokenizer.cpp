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

#include "tokenizer.h"

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_Tokenizer : public GumboTest
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void TagEnumIncludesAllTags();
    void PartialTag();
    void PartialTagWithAttributes();
    void LexCharToken();
    void LexCharRef();
    void LexCharRef_NotCharRef();
    void LeadingWhitespace();
    void Doctype();
    void DoctypePublic();
    void DoctypeSystem();
    void DoctypeUnterminated();
    void RawtextEnd();
    void RCDataEnd();
    void ScriptEnd();
    void ScriptEscapedEnd();
    void ScriptCommentEscaped();
    void ScriptEscapedEmbeddedLessThan();
    void ScriptHasTagEmbedded();
    void ScriptDoubleEscaped();
    void CData();
    void StyleHasTagEmbedded();
    void PreWithNewlines();
    void SelfClosingStartTag();
    void OpenTagWithAttributes();
    void BogusComment1();
    void BogusComment2();
    void MultilineAttribute();
    void DoubleAmpersand();
    void MatchedTagPair();
    void BogusEndTag();

private:
    GumboToken token_;

    void setInput(const char* input);
    void advance(int num_tokens);
};

/******************************************************************************
 ******************************************************************************/
void tst_Tokenizer::setInput(const char* input)
{
    text_ = input;
    gumbo_tokenizer_state_destroy(&parser_);
    gumbo_tokenizer_state_init(&parser_, input, strlen(input));
}

void tst_Tokenizer::advance(int num_tokens)
{
    for (int i = 0; i < num_tokens; ++i) {
        /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
        gumbo_token_destroy(&parser_, &token_);
    }
}

/******************************************************************************
 ******************************************************************************/
void tst_Tokenizer::init()
{
    gumbo_tokenizer_state_init(&parser_, "", 0);
}

void tst_Tokenizer::cleanup()
{
#if defined(Q_CC_MSVC) || defined(Q_OS_WIN64)
    /// \todo FIXME
    /// maybe GumboTest::cleanup() ?
#else
    gumbo_tokenizer_state_destroy(&parser_);
    gumbo_token_destroy(&parser_, &token_);
#endif
}

/******************************************************************************
 ******************************************************************************/
void tst_Tokenizer::TagEnumIncludesAllTags()
{
    /*EXPECT_EQ*/ QVERIFY(150 == GUMBO_TAG_UNKNOWN);
    /*EXPECT_STREQ*/ QCOMPARE("", gumbo_normalized_tagname(GUMBO_TAG_UNKNOWN));
}

void tst_Tokenizer::PartialTag()
{
    setInput("<a");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_EOF, token_.type);
}

void tst_Tokenizer::PartialTagWithAttributes()
{
    setInput("<a href=foo /");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_EOF, token_.type);
}

void tst_Tokenizer::LexCharToken()
{
    setInput("a");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY(1 == token_.position.column);
    /*EXPECT_EQ*/ QVERIFY(1 == token_.position.line);
    /*EXPECT_EQ*/ QVERIFY(0 == token_.position.offset);
    /*EXPECT_EQ*/ QVERIFY('a' == *token_.original_text.data);
    /*EXPECT_EQ*/ QVERIFY(1 == token_.original_text.length);
    /*EXPECT_EQ*/ QVERIFY('a' == token_.v.character);

    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_EOF, token_.type);
    /*EXPECT_EQ*/ QVERIFY(1 == token_.position.offset);
}

void tst_Tokenizer::LexCharRef()
{
    setInput("&nbsp; Text");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY(1 == token_.position.column);
    /*EXPECT_EQ*/ QVERIFY(1 == token_.position.line);
    /*EXPECT_EQ*/ QVERIFY(0 == token_.position.offset);
    /*EXPECT_EQ*/ QVERIFY('&' == *token_.original_text.data);
    /*EXPECT_EQ*/ QVERIFY(6 == token_.original_text.length);
    /*EXPECT_EQ*/ QCOMPARE(0xA0, token_.v.character);

    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_WHITESPACE, token_.type);
    /*EXPECT_EQ*/ QVERIFY(' ' == *token_.original_text.data);
    /*EXPECT_EQ*/ QVERIFY(' ' == token_.v.character);
}

void tst_Tokenizer::LexCharRef_NotCharRef()
{
    setInput("&xyz");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY(0 == token_.position.offset);
    /*EXPECT_EQ*/ QVERIFY('&' == token_.v.character);

    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY(1 == token_.position.offset);
    /*EXPECT_EQ*/ QVERIFY('x' == token_.v.character);
}

void tst_Tokenizer::LeadingWhitespace()
{
    setInput("<div>\n"
             "  <span class=foo>");

    advance(4);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));  // <span>

    GumboTokenStartTag* start_tag = &token_.v.start_tag;
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SPAN, start_tag->tag);
    /*EXPECT_EQ*/ QVERIFY(2 == token_.position.line);
    /*EXPECT_EQ*/ QVERIFY(3 == token_.position.column);
    /*ASSERT_EQ*/ QVERIFY(1 == start_tag->attributes.length);

    GumboAttribute* clas = static_cast<GumboAttribute*>(start_tag->attributes.data[0]);
    /*EXPECT_STREQ*/ QCOMPARE("class", clas->name);
    /*EXPECT_EQ*/ QVERIFY("class"== ToString(clas->original_name));
    /*EXPECT_EQ*/ QVERIFY(2 == clas->name_start.line);
    /*EXPECT_EQ*/ QVERIFY(9 == clas->name_start.column);
    /*EXPECT_EQ*/ QVERIFY(14 == clas->name_end.column);
    /*EXPECT_STREQ*/ QCOMPARE("foo", clas->value);
    /*EXPECT_EQ*/ QVERIFY("foo"== ToString(clas->original_value));
    /*EXPECT_EQ*/ QVERIFY(15 == clas->value_start.column);
    /*EXPECT_EQ*/ QVERIFY(18 == clas->value_end.column);
}

void tst_Tokenizer::Doctype()
{
    setInput("<!doctype html>");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QVERIFY(GUMBO_TOKEN_DOCTYPE == token_.type);
    /*EXPECT_EQ*/ QVERIFY(0 == token_.position.offset);

    GumboTokenDocType* doc_type = &token_.v.doc_type;
    /*EXPECT_FALSE*/ QVERIFY(!doc_type->force_quirks);
    /*EXPECT_FALSE*/ QVERIFY(!doc_type->has_public_identifier);
    /*EXPECT_FALSE*/ QVERIFY(!doc_type->has_system_identifier);
    /*EXPECT_STREQ*/ QCOMPARE("html", doc_type->name);
    /*EXPECT_STREQ*/ QCOMPARE("", doc_type->public_identifier);
    /*EXPECT_STREQ*/ QCOMPARE("", doc_type->system_identifier);
}

void tst_Tokenizer::DoctypePublic()
{
    setInput("<!DOCTYPE html PUBLIC "
             "\"-//W3C//DTD XHTML 1.0 Transitional//EN\" "
             "'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'>");

    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QVERIFY(GUMBO_TOKEN_DOCTYPE == token_.type);
    /*EXPECT_EQ*/ QVERIFY(0 == token_.position.offset);

    GumboTokenDocType* doc_type = &token_.v.doc_type;
    /*EXPECT_FALSE*/ QVERIFY(!doc_type->force_quirks);
    /*EXPECT_TRUE*/ QVERIFY(doc_type->has_public_identifier);
    /*EXPECT_TRUE*/ QVERIFY(doc_type->has_system_identifier);
    /*EXPECT_STREQ*/ QCOMPARE("html", doc_type->name);
    /*EXPECT_STREQ*/ QCOMPARE(
                "-//W3C//DTD XHTML 1.0 Transitional//EN", doc_type->public_identifier);
    /*EXPECT_STREQ*/ QCOMPARE("http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd",
                              doc_type->system_identifier);
}

void tst_Tokenizer::DoctypeSystem()
{
    setInput("<!DOCtype root_element SYSTEM \"DTD_location\">");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QVERIFY(GUMBO_TOKEN_DOCTYPE == token_.type);
    /*EXPECT_EQ*/ QVERIFY(0 == token_.position.offset);

    GumboTokenDocType* doc_type = &token_.v.doc_type;
    /*EXPECT_FALSE*/ QVERIFY(!doc_type->force_quirks);
    /*EXPECT_FALSE*/ QVERIFY(!doc_type->has_public_identifier);
    /*EXPECT_TRUE*/ QVERIFY(doc_type->has_system_identifier);
    /*EXPECT_STREQ*/ QCOMPARE("root_element", doc_type->name);
    /*EXPECT_STREQ*/ QCOMPARE("DTD_location", doc_type->system_identifier);
}

void tst_Tokenizer::DoctypeUnterminated()
{
    setInput("<!DOCTYPE a PUBLIC''");
    /*EXPECT_FALSE*/ QVERIFY(!gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QVERIFY(GUMBO_TOKEN_DOCTYPE == token_.type);
    /*EXPECT_EQ*/ QVERIFY(0 == token_.position.offset);

    GumboTokenDocType* doc_type = &token_.v.doc_type;
    /*EXPECT_TRUE*/ QVERIFY(doc_type->force_quirks);
    /*EXPECT_TRUE*/ QVERIFY(doc_type->has_public_identifier);
    /*EXPECT_FALSE*/ QVERIFY(!doc_type->has_system_identifier);
    /*EXPECT_STREQ*/ QCOMPARE("a", doc_type->name);
    /*EXPECT_STREQ*/ QCOMPARE("", doc_type->system_identifier);
}

void tst_Tokenizer::RawtextEnd()
{
    setInput("<title>x ignores <tag></title>");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_START_TAG, token_.type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TITLE, token_.v.start_tag.tag);

    gumbo_tokenizer_set_state(&parser_, GUMBO_LEX_RAWTEXT);
    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('x' == token_.v.character);
    gumbo_token_destroy(&parser_, &token_);

    advance(9);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('<' == token_.v.character);
    gumbo_token_destroy(&parser_, &token_);

    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('t' == token_.v.character);
    gumbo_token_destroy(&parser_, &token_);

    advance(3);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_END_TAG, token_.type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TITLE, token_.v.end_tag);
}

void tst_Tokenizer::RCDataEnd()
{
    setInput("<title>x</title>");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_START_TAG, token_.type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TITLE, token_.v.start_tag.tag);

    gumbo_tokenizer_set_state(&parser_, GUMBO_LEX_RCDATA);
    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('x' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_END_TAG, token_.type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TITLE, token_.v.end_tag);
}

void tst_Tokenizer::ScriptEnd()
{
    setInput("<script>x = '\"></';</script>");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_START_TAG, token_.type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SCRIPT, token_.v.start_tag.tag);

    gumbo_tokenizer_set_state(&parser_, GUMBO_LEX_SCRIPT);
    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('x' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    advance(6);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('<' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('/' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('\'' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    advance(1);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_END_TAG, token_.type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SCRIPT, token_.v.end_tag);
}

void tst_Tokenizer::ScriptEscapedEnd()
{
    setInput("<title>x</title>");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_START_TAG, token_.type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TITLE, token_.v.start_tag.tag);

    gumbo_tokenizer_set_state(&parser_, GUMBO_LEX_SCRIPT_ESCAPED);
    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('x' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_END_TAG, token_.type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TITLE, token_.v.end_tag);
}

void tst_Tokenizer::ScriptCommentEscaped()
{
    setInput("<script><!-- var foo = x < 7 + '</div>-- <A href=\"foo\"></a>';\n"
             "-->\n"
             "</script>");
    advance(1);
    gumbo_tokenizer_set_state(&parser_, GUMBO_LEX_SCRIPT);
    advance(15);

    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('x' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_WHITESPACE, token_.type);
    /*EXPECT_EQ*/ QVERIFY(' ' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('<' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_WHITESPACE, token_.type);
    /*EXPECT_EQ*/ QVERIFY(' ' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('7' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    advance(4);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('<' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('/' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('d' == token_.v.character);
    gumbo_token_destroy(&parser_, &token_);
    advance(25);
}

void tst_Tokenizer::ScriptEscapedEmbeddedLessThan()
{
    setInput("<script>/*<![CDATA[*/ x<7 /*]]>*/</script>");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_START_TAG, token_.type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SCRIPT, token_.v.start_tag.tag);

    gumbo_tokenizer_set_state(&parser_, GUMBO_LEX_SCRIPT);
    gumbo_token_destroy(&parser_, &token_);
    advance(14);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('x' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('<' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('7' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    advance(8);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_END_TAG, token_.type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SCRIPT, token_.v.end_tag);
}

void tst_Tokenizer::ScriptHasTagEmbedded()
{
    setInput("<script>var foo = '</div>';</script>");
    advance(1);
    gumbo_tokenizer_set_state(&parser_, GUMBO_LEX_SCRIPT);
    advance(11);

    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('<' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('/' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('d' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('i' == token_.v.character);
}

void tst_Tokenizer::ScriptDoubleEscaped()
{
    setInput("<script><!--var foo = '<a href=\"foo\"></a>\n"
             "<sCrIpt>i--<f</script>'-->;</script>");

    advance(1);
    gumbo_tokenizer_set_state(&parser_, GUMBO_LEX_SCRIPT);
    advance(34);

    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('<' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('s' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('C' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    advance(20);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('-' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('-' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('>' == token_.v.character);
}

void tst_Tokenizer::CData()
{
    // SetInput uses strlen and so can't handle nulls.
    text_ = "<![CDATA[\0filler\0text\0]]>";
    gumbo_tokenizer_state_destroy(&parser_);
    gumbo_tokenizer_state_init(
                &parser_, text_, sizeof("<![CDATA[\0filler\0text\0]]>") - 1);
    gumbo_tokenizer_set_is_current_node_foreign(&parser_, true);

    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_NULL, token_.type);
    /*EXPECT_EQ*/ QCOMPARE(0, token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CDATA, token_.type);
    /*EXPECT_EQ*/ QVERIFY('f' == token_.v.character);
}

void tst_Tokenizer::StyleHasTagEmbedded()
{
    setInput("<style>/* For <head> */</style>");
    advance(1);
    gumbo_tokenizer_set_state(&parser_, GUMBO_LEX_RCDATA);
    advance(7);

    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('<' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('h' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('e' == token_.v.character);
}

void tst_Tokenizer::PreWithNewlines()
{
    setInput("<!DOCTYPE html><pre>\r\na</pre>");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QVERIFY(GUMBO_TOKEN_DOCTYPE == token_.type);
    /*EXPECT_EQ*/ QVERIFY(0 == token_.position.offset);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_TOKEN_START_TAG, token_.type);
    /*EXPECT_EQ*/ QVERIFY("<pre>" == ToString(token_.original_text));
    /*EXPECT_EQ*/ QVERIFY(15 == token_.position.offset);
}

void tst_Tokenizer::SelfClosingStartTag()
{
    setInput("<br />");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QVERIFY(GUMBO_TOKEN_START_TAG == token_.type);
    /*EXPECT_EQ*/ QVERIFY(0 == token_.position.offset);
    /*EXPECT_EQ*/ QVERIFY("<br />" == ToString(token_.original_text));

    GumboTokenStartTag* start_tag = &token_.v.start_tag;
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BR, start_tag->tag);
    /*EXPECT_EQ*/ QVERIFY(0 == start_tag->attributes.length);
    /*EXPECT_TRUE*/ QVERIFY(start_tag->is_self_closing);
}

void tst_Tokenizer::OpenTagWithAttributes()
{
    setInput("<a href ='/search?q=foo&amp;hl=en'  id=link>");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QVERIFY(GUMBO_TOKEN_START_TAG == token_.type);

    GumboTokenStartTag* start_tag = &token_.v.start_tag;
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_A, start_tag->tag);
    /*EXPECT_FALSE*/ QVERIFY(!start_tag->is_self_closing);
    /*ASSERT_EQ*/ QVERIFY(2 == start_tag->attributes.length);

    GumboAttribute* href = static_cast<GumboAttribute*>(start_tag->attributes.data[0]);
    /*EXPECT_STREQ*/ QCOMPARE("href", href->name);
    /*EXPECT_EQ*/ QVERIFY("href" == ToString(href->original_name));
    /*EXPECT_STREQ*/ QCOMPARE("/search?q=foo&hl=en", href->value);
    /*EXPECT_EQ*/ QVERIFY("'/search?q=foo&amp;hl=en'" == ToString(href->original_value));

    GumboAttribute* id = static_cast<GumboAttribute*>(start_tag->attributes.data[1]);
    /*EXPECT_STREQ*/ QCOMPARE("id", id->name);
    /*EXPECT_EQ*/ QVERIFY("id" == ToString(id->original_name));
    /*EXPECT_STREQ*/ QCOMPARE("link", id->value);
    /*EXPECT_EQ*/ QVERIFY("link" == ToString(id->original_value));
}

void tst_Tokenizer::BogusComment1()
{
    setInput("<?xml is bogus-comment>Text");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QVERIFY(GUMBO_TOKEN_COMMENT == token_.type);
    /*EXPECT_STREQ*/ QCOMPARE("?xml is bogus-comment", token_.v.text);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY('T' == token_.v.character);

    errors_are_expected_ = true;
}

void tst_Tokenizer::BogusComment2()
{
    setInput("</#bogus-comment");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QVERIFY(GUMBO_TOKEN_COMMENT == token_.type);
    /*EXPECT_STREQ*/ QCOMPARE("#bogus-comment", token_.v.text);

    gumbo_token_destroy(&parser_, &token_);
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TOKEN_EOF, token_.type);
    errors_are_expected_ = true;
}

void tst_Tokenizer::MultilineAttribute()
{
    setInput("<foo long_attr=\"SomeCode;\n"
             "  calls_a_big_long_function();\n"
             "  return true;\" />");

    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QVERIFY(GUMBO_TOKEN_START_TAG == token_.type);

    GumboTokenStartTag* start_tag = &token_.v.start_tag;
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_UNKNOWN, start_tag->tag);
    /*EXPECT_TRUE*/ QVERIFY(start_tag->is_self_closing);
    /*ASSERT_EQ*/ QVERIFY(1 == start_tag->attributes.length);

    GumboAttribute* long_attr =
            static_cast<GumboAttribute*>(start_tag->attributes.data[0]);
    /*EXPECT_STREQ*/ QCOMPARE("long_attr", long_attr->name);
    /*EXPECT_EQ*/ QVERIFY("long_attr" == ToString(long_attr->original_name));
    /*EXPECT_STREQ*/ QCOMPARE(
                "SomeCode;\n"
                "  calls_a_big_long_function();\n"
                "  return true;",
                long_attr->value);
}

void tst_Tokenizer::DoubleAmpersand()
{
    setInput("<span jsif=\"foo && bar\">");
    /*EXPECT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QVERIFY(GUMBO_TOKEN_START_TAG == token_.type);

    GumboTokenStartTag* start_tag = &token_.v.start_tag;
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SPAN, start_tag->tag);
    /*EXPECT_FALSE*/ QVERIFY(!start_tag->is_self_closing);
    /*ASSERT_EQ*/ QVERIFY(1 == start_tag->attributes.length);

    GumboAttribute* jsif = static_cast<GumboAttribute*>(start_tag->attributes.data[0]);
    /*EXPECT_STREQ*/ QCOMPARE("jsif", jsif->name);
    /*EXPECT_EQ*/ QVERIFY("jsif" == ToString(jsif->original_name));
    /*EXPECT_STREQ*/ QCOMPARE("foo && bar", jsif->value);
    /*EXPECT_EQ*/ QVERIFY("\"foo && bar\"" == ToString(jsif->original_value));
}

void tst_Tokenizer::MatchedTagPair()
{
    setInput("<div id=dash<-Dash data-test=\"bar\">a</div>");
    /*ASSERT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QVERIFY(GUMBO_TOKEN_START_TAG == token_.type);
    /*EXPECT_EQ*/ QVERIFY(0 == token_.position.offset);

    GumboTokenStartTag* start_tag = &token_.v.start_tag;
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DIV, start_tag->tag);
    /*EXPECT_FALSE*/ QVERIFY(!start_tag->is_self_closing);
    /*ASSERT_EQ*/ QVERIFY(2 == start_tag->attributes.length);

    GumboAttribute* id =
            static_cast<GumboAttribute*>(start_tag->attributes.data[0]);
    /*EXPECT_STREQ*/ QCOMPARE("id", id->name);
    /*EXPECT_EQ*/ QVERIFY("id" == ToString(id->original_name));
    /*EXPECT_EQ*/ QVERIFY(1 == id->name_start.line);
    /*EXPECT_EQ*/ QVERIFY(5 == id->name_start.offset);
    /*EXPECT_EQ*/ QVERIFY(6 == id->name_start.column);
    /*EXPECT_EQ*/ QVERIFY(8 == id->name_end.column);
    /*EXPECT_STREQ*/ QCOMPARE("dash<-Dash", id->value);
    /*EXPECT_EQ*/ QVERIFY("dash<-Dash" == ToString(id->original_value));
    /*EXPECT_EQ*/ QVERIFY(9 == id->value_start.column);
    /*EXPECT_EQ*/ QVERIFY(19 == id->value_end.column);

    GumboAttribute* data_attr =
            static_cast<GumboAttribute*>(start_tag->attributes.data[1]);
    /*EXPECT_STREQ*/ QCOMPARE("data-test", data_attr->name);
    /*EXPECT_EQ*/ QVERIFY("data-test" == ToString(data_attr->original_name));
    /*EXPECT_EQ*/ QVERIFY(20 == data_attr->name_start.column);
    /*EXPECT_EQ*/ QVERIFY(29 == data_attr->name_end.column);
    /*EXPECT_STREQ*/ QCOMPARE("bar", data_attr->value);
    /*EXPECT_EQ*/ QVERIFY("\"bar\"" == ToString(data_attr->original_value));
    /*EXPECT_EQ*/ QVERIFY(30 == data_attr->value_start.column);
    /*EXPECT_EQ*/ QVERIFY(35 == data_attr->value_end.column);

    gumbo_token_destroy(&parser_, &token_);
    /*ASSERT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_TOKEN_CHARACTER, token_.type);
    /*EXPECT_EQ*/ QVERIFY(35 == token_.position.offset);
    /*EXPECT_EQ*/ QVERIFY('a' == token_.v.character);

    gumbo_token_destroy(&parser_, &token_);
    /*ASSERT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_TOKEN_END_TAG, token_.type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DIV, token_.v.end_tag);
    errors_are_expected_ = true;
}

void tst_Tokenizer::BogusEndTag()
{
    // According to the spec, the correct parse of this is an end tag token for
    // "<div<>" (notice the ending bracket) with the attribute "th=th" (ignored
    // because end tags don't take attributes), with the tokenizer passing through
    // the self-closing tag state in the process.
    setInput("</div</th>");
    /*ASSERT_TRUE*/ QVERIFY(gumbo_lex(&parser_, &token_));
    /*ASSERT_EQ*/ QVERIFY(GUMBO_TOKEN_END_TAG == token_.type);
    /*EXPECT_EQ*/ QVERIFY(0 == token_.position.offset);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_UNKNOWN, token_.v.end_tag);
    /*EXPECT_EQ*/ QVERIFY("</div</th>" == ToString(token_.original_text));
    errors_are_expected_ = true;
}

/******************************************************************************
 ******************************************************************************/

QTEST_APPLESS_MAIN(tst_Tokenizer)

#include "tst_tokenizer.moc"
