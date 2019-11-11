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

#include "gumbo.h"

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_Parser : public GumboTest
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void NullDocument();
    void ParseTwice();
    void OneChar();
    void TextOnly();
    void SelfClosingTagError();
    void UnexpectedEndBreak();
    void CaseSensitiveAttributes();
    void ExplicitHtmlStructure();
    void Whitespace();
    void DuplicateAttributes();
    void LinkTagsInHead();
    void WhitespaceBeforeHtml();
    void TextAfterHtml();
    void WhitespaceInHead();
    void Doctype();
    void InvalidDoctype();
    void SingleComment();
    void CommentInText();
    void CommentBeforeNode();
    void CommentInVerbatimMode();
    void UnknownTag();
    void UnknownTag2();
    void InvalidEndTag();
    void Tables();
    void StartParagraphInTable();
    void EndParagraphInTable();
    void UnknownTagInTable();
    void UnclosedTableTags();
    void MisnestedTable();
    void MisnestedTable2();
    void Select();
    void ComplicatedSelect();
    void DoubleSelect();
    void InputInSelect();
    void SelectInTable();
    void ImplicitColgroup();
    void Form();
    void NestedForm();
    void MisnestedFormInTable();
    void IsIndex();
    void IsIndexDuplicateAttribute();
    void NestedRawtextTags();
    void RawtextInBody();
    void MetaBeforeHead();
    void NoahsArkClause();
    void AdoptionAgency1();
    void AdoptionAgency2();
    void AdoptionAgency3();
    void ImplicitlyCloseLists();
    void CData();
    void CDataUnsafe();
    void CDataInBody();
    void FormattingTagsInHeading();
    void ExtraReconstruction();
    void LinkifiedHeading();
    void MisnestedHeading();
    void DoubleBody();
    void ThInMathMl();
    void TdInMathml();
    void SelectInForeignContent();
    void TemplateInForeignContent();
    void TemplateNull();
    void FragmentWithNamespace();

private:
    GumboOutput* output_;
    GumboNode* root_;

    virtual void parse(const char* input);
    virtual void parse(const std::string& input);
    virtual void parseFragment(const char* input, GumboTag context, GumboNamespaceEnum context_ns);
};

/******************************************************************************
 ******************************************************************************/
void tst_Parser::init()
{
    options_ = kGumboDefaultOptions;
    output_ = nullptr;
    root_ = nullptr;
    InitLeakDetection(&options_, &malloc_stats_);
}

void tst_Parser::cleanup()
{
    if (output_) {
        gumbo_destroy_output(&options_, output_);
    }
    /*EXPECT_EQ*/ QCOMPARE(malloc_stats_.objects_allocated, malloc_stats_.objects_freed);
}

/******************************************************************************
 ******************************************************************************/
void tst_Parser::parse(const char* input)
{
    if (output_) {
        gumbo_destroy_output(&options_, output_);
    }
    output_ = gumbo_parse_with_options(&options_, input, strlen(input));
    // The naming inconsistency is because these tests were initially written
    // when gumbo_parse returned the document element instead of an GumboOutput
    // structure.
    root_ = output_->document;
}

void tst_Parser::parse(const std::string& input)
{
    // This overload is so we can test/demonstrate that computing offsets from
    // the .data() member of an STL string works properly.
    if (output_) {
        gumbo_destroy_output(&options_, output_);
    }
    output_ = gumbo_parse_with_options(&options_, input.data(), input.length());
    root_ = output_->document;
    SanityCheckPointers(input.data(), input.length(), output_->root, 1000);
}

void tst_Parser::parseFragment(const char* input, GumboTag context, GumboNamespaceEnum context_ns)
{
    if (output_) {
        gumbo_destroy_output(&options_, output_);
    }
    options_.fragment_context = context;
    options_.fragment_namespace = context_ns;
    output_ = gumbo_parse_with_options(&options_, input, strlen(input));
    root_ = output_->document;
}


/******************************************************************************
 ******************************************************************************/
void tst_Parser::NullDocument()
{
    parse("");
    /*ASSERT_TRUE*/ QVERIFY(root_);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_DOCUMENT, root_->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_BY_PARSER, root_->parse_flags);

    GumboNode* body;
    GetAndAssertBody(root_, &body);
}

void tst_Parser::ParseTwice()
{
    parse("");
    /*ASSERT_TRUE*/ QVERIFY(root_);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_DOCUMENT, root_->type);

    std::string second_input("");
    parse(second_input);
    /*ASSERT_TRUE*/ QVERIFY(root_);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_DOCUMENT, root_->type);

    GumboNode* body;
    GetAndAssertBody(root_, &body);
}

void tst_Parser::OneChar()
{
    std::string input("T");
    parse(input);
    /*ASSERT_TRUE*/ QVERIFY(root_);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_DOCUMENT, root_->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_BY_PARSER, root_->parse_flags);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(root_));

    GumboNode* html = GetChild(root_, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, html->type);
    /*EXPECT_TRUE*/ QVERIFY(html->parse_flags & GUMBO_INSERTION_BY_PARSER);
    /*EXPECT_TRUE*/ QVERIFY(html->parse_flags & GUMBO_INSERTION_IMPLICIT_END_TAG);
    /*EXPECT_TRUE*/ QVERIFY(html->parse_flags & GUMBO_INSERTION_IMPLIED);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HTML, html->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(html));

    GumboNode* head = GetChild(html, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, head->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HEAD, head->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(0, GetChildCount(head));

    GumboNode* body = GetChild(html, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, body->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BODY, body->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));
    /*EXPECT_EQ*/ QVERIFY(1 == body->v.element.start_pos.line);
    /*EXPECT_EQ*/ QVERIFY(1 == body->v.element.start_pos.column);
    /*EXPECT_EQ*/ QVERIFY(0 == body->v.element.start_pos.offset);
    /*EXPECT_EQ*/ QVERIFY(1 == body->v.element.end_pos.line);
    /*EXPECT_EQ*/ QVERIFY(2 == body->v.element.end_pos.column);
    /*EXPECT_EQ*/ QVERIFY(1 == body->v.element.end_pos.offset);

    GumboNode* text = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("T", text->v.text.text);
    /*EXPECT_EQ*/ QVERIFY(1 == text->v.text.start_pos.line);
    /*EXPECT_EQ*/ QVERIFY(1 == text->v.text.start_pos.column);
    /*EXPECT_EQ*/ QVERIFY(0 == text->v.text.start_pos.offset);
    /*EXPECT_EQ*/ QCOMPARE(input.data(), text->v.text.original_text.data);
    /*EXPECT_EQ*/ QVERIFY(1 == text->v.text.original_text.length);
}

void tst_Parser::TextOnly()
{
    parse("Test");
    /*EXPECT_EQ*/ QVERIFY(1 == output_->errors.length);  // No doctype.
    /*ASSERT_EQ*/ QVERIFY(1 == GetChildCount(root_));

    GumboNode* html = GetChild(root_, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, html->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HTML, html->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(html));

    GumboNode* head = GetChild(html, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, head->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HEAD, head->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(0, GetChildCount(head));

    GumboNode* body = GetChild(html, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, body->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BODY, body->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* text = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("Test", text->v.text.text);
}

void tst_Parser::SelfClosingTagError()
{
    parse("<div/>");
    // TODO(jdtang): I think this is double-counting some error cases, I think we
    // may ultimately want to de-dup errors that occur on the same token.
    /*EXPECT_EQ*/ QVERIFY(8 == output_->errors.length);
}

void tst_Parser::UnexpectedEndBreak()
{
    parse("</br><div></div>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* br = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, br->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BR, br->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(br));

    GumboNode* div = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, div->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DIV, div->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(div));
}

void tst_Parser::CaseSensitiveAttributes()
{
    parse("<div class=CamelCase>");
    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* div = GetChild(body, 0);
    GumboVector* attributes = &div->v.element.attributes;
    /*ASSERT_EQ*/ QVERIFY(1 == attributes->length);

    GumboAttribute* clas = static_cast<GumboAttribute*>(attributes->data[0]);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_ATTR_NAMESPACE_NONE, clas->attr_namespace);
    /*EXPECT_STREQ*/ QCOMPARE("class", clas->name);
    /*EXPECT_STREQ*/ QCOMPARE("CamelCase", clas->value);
}

void tst_Parser::ExplicitHtmlStructure()
{
    parse("<!doctype html>\n<html>"
          "<head><title>Foo</title></head>\n"
          "<body><div class=bar>Test</div></body></html>");

    /*ASSERT_EQ*/ QVERIFY(1 == GetChildCount(root_));
    /*EXPECT_EQ*/ QVERIFY(0 == output_->errors.length);

    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_DOCUMENT, root_->type);
    /*EXPECT_STREQ*/ QCOMPARE("html", root_->v.document.name);
    /*EXPECT_STREQ*/ QCOMPARE("", root_->v.document.public_identifier);
    /*EXPECT_STREQ*/ QCOMPARE("", root_->v.document.system_identifier);

    GumboNode* html = GetChild(root_, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, html->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, html->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HTML, html->v.element.tag);
    /*EXPECT_EQ*/ QVERIFY(2 == html->v.element.start_pos.line);
    /*EXPECT_EQ*/ QVERIFY(1 == html->v.element.start_pos.column);
    /*EXPECT_EQ*/ QVERIFY(16 == html->v.element.start_pos.offset);
    /*EXPECT_EQ*/ QVERIFY(3 == html->v.element.end_pos.line);
    /*EXPECT_EQ*/ QVERIFY(39 == html->v.element.end_pos.column);
    /*EXPECT_EQ*/ QVERIFY(92 == html->v.element.end_pos.offset);
    /*EXPECT_EQ*/ QVERIFY("<html>" == ToString(html->v.element.original_tag));
    /*EXPECT_EQ*/ QVERIFY("</html>" == ToString(html->v.element.original_end_tag));
    /*ASSERT_EQ*/ QVERIFY(3 == GetChildCount(html));

    GumboNode* head = GetChild(html, 0);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, head->parse_flags);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, head->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HEAD, head->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(html, head->parent);
    /*EXPECT_EQ*/ QVERIFY(0 == head->index_within_parent);
    /*EXPECT_EQ*/ QCOMPARE(1, GetChildCount(head));

    GumboNode* body = GetChild(html, 2);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, body->parse_flags);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, body->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BODY, body->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(html, body->parent);
    /*EXPECT_EQ*/ QVERIFY(3 == body->v.element.start_pos.line);
    /*EXPECT_EQ*/ QVERIFY(1 == body->v.element.start_pos.column);
    /*EXPECT_EQ*/ QVERIFY(54 == body->v.element.start_pos.offset);
    /*EXPECT_EQ*/ QVERIFY(3 == body->v.element.end_pos.line);
    /*EXPECT_EQ*/ QVERIFY(32 == body->v.element.end_pos.column);
    /*EXPECT_EQ*/ QVERIFY(85 == body->v.element.end_pos.offset);
    /*EXPECT_EQ*/ QVERIFY("<body>" == ToString(body->v.element.original_tag));
    /*EXPECT_EQ*/ QVERIFY("</body>" == ToString(body->v.element.original_end_tag));
    /*EXPECT_EQ*/ QVERIFY(2 == body->index_within_parent);
    /*ASSERT_EQ*/ QVERIFY(1 == GetChildCount(body));

    GumboNode* div = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, div->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DIV, div->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(body, div->parent);
    /*EXPECT_EQ*/ QVERIFY(0 == div->index_within_parent);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(div));

    /*ASSERT_EQ*/ QCOMPARE(1, GetAttributeCount(div));
    GumboAttribute* clas = GetAttribute(div, 0);
    /*EXPECT_STREQ*/ QCOMPARE("class", clas->name);
    /*EXPECT_EQ*/ QVERIFY("class" == ToString(clas->original_name));
    /*EXPECT_STREQ*/ QCOMPARE("bar", clas->value);
    /*EXPECT_EQ*/ QVERIFY("bar" == ToString(clas->original_value));

    GumboNode* text = GetChild(div, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("Test", text->v.text.text);
}

void tst_Parser::Whitespace()
{
    parse("<ul>\n  <li>Text\n</ul>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* ul = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, ul->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_UL, ul->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(ul));

    GumboNode* whitespace = GetChild(ul, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_WHITESPACE, whitespace->type);
    /*EXPECT_STREQ*/ QCOMPARE("\n  ", whitespace->v.text.text);

    GumboNode* li = GetChild(ul, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, li->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_LI, li->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(li));

    GumboNode* text = GetChild(li, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("Text\n", text->v.text.text);
}

void tst_Parser::DuplicateAttributes()
{
    std::string text("<input checked=\"false\" checked=true id=foo id='bar'>");
    parse(text);

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* input = GetChild(body, 0);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_IMPLICIT_END_TAG, input->parse_flags);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, input->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_INPUT, input->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(0, GetChildCount(input));
    /*ASSERT_EQ*/ QCOMPARE(2, GetAttributeCount(input));

    GumboAttribute* checked = GetAttribute(input, 0);
    /*EXPECT_STREQ*/ QCOMPARE("checked", checked->name);
    /*EXPECT_STREQ*/ QCOMPARE("false", checked->value);
    /*EXPECT_EQ*/ QVERIFY(1 == checked->name_start.line);
    /*EXPECT_EQ*/ QVERIFY(8 == checked->name_start.column);
    /*EXPECT_EQ*/ QVERIFY(15 == checked->name_end.column);
    /*EXPECT_EQ*/ QVERIFY(16 == checked->value_start.column);
    /*EXPECT_EQ*/ QVERIFY(23 == checked->value_end.column);
    /*EXPECT_EQ*/ QVERIFY(7 == checked->original_name.data - text.data());
    /*EXPECT_EQ*/ QVERIFY(7 == checked->original_name.length);
    /*EXPECT_EQ*/ QVERIFY(15 == checked->original_value.data - text.data());
    /*EXPECT_EQ*/ QVERIFY(7 == checked->original_value.length);

    GumboAttribute* id = GetAttribute(input, 1);
    /*EXPECT_STREQ*/ QCOMPARE("id", id->name);
    /*EXPECT_STREQ*/ QCOMPARE("foo", id->value);

    // TODO(jdtang): Run some assertions on the parse error that's added.
}

void tst_Parser::LinkTagsInHead()
{
    parse("<html>\n"
          "  <head>\n"
          "    <title>Sample title></title>\n\n"
          "    <link rel=stylesheet>\n"
          "    <link rel=author>\n"
          "  </head>\n"
          "  <body>Foo</body>");

    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(root_));

    GumboNode* html = GetChild(root_, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, html->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_IMPLICIT_END_TAG, html->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HTML, html->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(3, GetChildCount(html));

    GumboNode* head = GetChild(html, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, head->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, head->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HEAD, head->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(7, GetChildCount(head));

    GumboNode* text1 = GetChild(head, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_WHITESPACE, text1->type);
    /*EXPECT_STREQ*/ QCOMPARE("\n\n    ", text1->v.text.text);

    GumboNode* link1 = GetChild(head, 3);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, link1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_LINK, link1->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_IMPLICIT_END_TAG, link1->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(0, GetChildCount(link1));

    GumboNode* text2 = GetChild(head, 4);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_WHITESPACE, text2->type);
    /*EXPECT_STREQ*/ QCOMPARE("\n    ", text2->v.text.text);

    GumboNode* link2 = GetChild(head, 5);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, link2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_LINK, link2->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_IMPLICIT_END_TAG, link2->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(0, GetChildCount(link2));

    GumboNode* text3 = GetChild(head, 6);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_WHITESPACE, text3->type);
    /*EXPECT_STREQ*/ QCOMPARE("\n  ", text3->v.text.text);

    GumboNode* body = GetChild(html, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, body->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, body->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BODY, body->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));
}

void tst_Parser::WhitespaceBeforeHtml()
{
    parse("<!doctype html>\n<html>Test</html>");
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(root_));

    GumboNode* body = GetChild(GetChild(root_, 0), 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, body->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BODY, GetTag(body));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* text = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("Test", text->v.text.text);
}

void tst_Parser::TextAfterHtml()
{
    parse("<html>Test</html> after doc");
    GumboNode* body;
    GetAndAssertBody(root_, &body);

    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, body->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BODY, GetTag(body));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* text = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("Test after doc", text->v.text.text);
}

void tst_Parser::WhitespaceInHead()
{
    parse("<html>  Test</html>");

    GumboNode* html = GetChild(root_, 0);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, html->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HTML, GetTag(html));
    /*EXPECT_EQ*/ QCOMPARE(2, GetChildCount(html));

    GumboNode* head = GetChild(html, 0);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, head->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HEAD, GetTag(head));
    /*EXPECT_EQ*/ QCOMPARE(0, GetChildCount(head));

    GumboNode* body = GetChild(html, 1);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, body->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BODY, GetTag(body));

    GumboNode* text = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("Test", text->v.text.text);
}

void tst_Parser::Doctype()
{
    parse("<!doctype html>Test");
    GumboDocument* doc = &root_->v.document;
    /*EXPECT_EQ*/ QVERIFY(1 == doc->children.length);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_DOCTYPE_NO_QUIRKS, doc->doc_type_quirks_mode);

    /*EXPECT_STREQ*/ QCOMPARE("html", doc->name);
    /*EXPECT_STREQ*/ QCOMPARE("", doc->public_identifier);
    /*EXPECT_STREQ*/ QCOMPARE("", doc->system_identifier);
}

void tst_Parser::InvalidDoctype()
{
    parse("Test<!doctype root_element SYSTEM \"DTD_location\">");

    // Default doc token; the declared one is ignored.
    GumboDocument* doc = &root_->v.document;
    /*EXPECT_EQ*/ QVERIFY(1 == doc->children.length);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_DOCTYPE_QUIRKS, doc->doc_type_quirks_mode);

    /*EXPECT_STREQ*/ QCOMPARE("", doc->name);
    /*EXPECT_STREQ*/ QCOMPARE("", doc->public_identifier);
    /*EXPECT_STREQ*/ QCOMPARE("", doc->system_identifier);

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, body->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BODY, GetTag(body));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* text = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("Test", text->v.text.text);
}

void tst_Parser::SingleComment()
{
    parse("<!-- comment -->");
    GumboNode* comment = GetChild(root_, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_COMMENT, comment->type);
    /*EXPECT_STREQ*/ QCOMPARE(" comment ", comment->v.text.text);
}

void tst_Parser::CommentInText()
{
    parse("Start <!-- comment --> end");
    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(3, GetChildCount(body));

    GumboNode* start = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, start->type);
    /*EXPECT_STREQ*/ QCOMPARE("Start ", start->v.text.text);

    GumboNode* comment = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_COMMENT, comment->type);
    /*EXPECT_EQ*/ QCOMPARE(body, comment->parent);
    /*EXPECT_EQ*/ QVERIFY(1 == comment->index_within_parent);
    /*EXPECT_STREQ*/ QCOMPARE(" comment ", comment->v.text.text);

    GumboNode* end = GetChild(body, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, end->type);
    /*EXPECT_STREQ*/ QCOMPARE(" end", end->v.text.text);
}

void tst_Parser::CommentBeforeNode()
{
    parse("<!--This is a comment-->\n<h1>hello world!</h1>");

    GumboNode* comment = GetChild(root_, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_COMMENT, comment->type);
    /*EXPECT_STREQ*/ QCOMPARE("This is a comment", comment->v.text.text);
    /*EXPECT_EQ*/ QVERIFY("<!--This is a comment-->" == ToString(comment->v.text.original_text));

    // Newline is ignored per the rules for "initial" insertion mode.

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* h1 = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, h1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_H1, h1->v.element.tag);
}

void tst_Parser::CommentInVerbatimMode()
{
    parse("<body> <div id='onegoogle'>Text</div>  </body><!-- comment \n\n-->");

    GumboNode* html = GetChild(root_, 0);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, html->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HTML, GetTag(html));
    /*EXPECT_EQ*/ QVERIFY((GUMBO_INSERTION_BY_PARSER | GUMBO_INSERTION_IMPLIED |
                          GUMBO_INSERTION_IMPLICIT_END_TAG) == html->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(3, GetChildCount(html));

    GumboNode* body = GetChild(html, 1);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, body->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BODY, GetTag(body));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, body->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(3, GetChildCount(body));

    GumboNode* comment = GetChild(html, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_COMMENT, comment->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, comment->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE(" comment \n\n", comment->v.text.text);
}

void tst_Parser::UnknownTag()
{
    parse("<foo>1<p>2</FOO>");
    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* foo = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, foo->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_UNKNOWN, GetTag(foo));
    /*EXPECT_EQ*/ QVERIFY("<foo>" == ToString(foo->v.element.original_tag));
    // According to the spec, the misplaced end tag is ignored, and so we return
    // an empty original_end_tag text.  We may want to extend our error-reporting
    // a bit so that we close off the tag that it *would have closed*, had the
    // HTML been correct, along with a parse flag that says the end tag was in the
    // wrong place.
    /*EXPECT_EQ*/ QVERIFY("" == ToString(foo->v.element.original_end_tag));
}

void tst_Parser::UnknownTag2()
{
    parse("<div><sarcasm><div></div></sarcasm></div>");
    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* div = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(div));
    GumboNode* sarcasm = GetChild(div, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, sarcasm->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_UNKNOWN, GetTag(sarcasm));
    /*EXPECT_EQ*/ QVERIFY("<sarcasm>" == ToString(sarcasm->v.element.original_tag));
    /*EXPECT_EQ*/ QVERIFY("</sarcasm>" == ToString(sarcasm->v.element.original_end_tag));
}

void tst_Parser::InvalidEndTag()
{
    parse("<a><img src=foo.jpg></img></a>");
    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* a = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, a->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_A, GetTag(a));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(a));

    GumboNode* img = GetChild(a, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, img->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_IMG, GetTag(img));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(img));
}

void tst_Parser::Tables()
{
    parse("<html><table>\n"
          "  <tr><br /></invalid-tag>\n"
          "    <th>One</th>\n"
          "    <td>Two</td>\n"
          "  </tr>\n"
          "  <iframe></iframe>"
          "</table><tr></tr><div></div></html>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(4, GetChildCount(body));

    GumboNode* br = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, br->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BR, GetTag(br));
    /*EXPECT_EQ*/ QCOMPARE(body, br->parent);
    /*EXPECT_EQ*/ QVERIFY(0 == br->index_within_parent);
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(br));

    GumboNode* iframe = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, iframe->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_IFRAME, GetTag(iframe));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(iframe));

    GumboNode* table = GetChild(body, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, GetTag(table));
    /*EXPECT_EQ*/ QCOMPARE(body, table->parent);
    /*EXPECT_EQ*/ QVERIFY(2 == table->index_within_parent);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(table));

    GumboNode* table_text = GetChild(table, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_WHITESPACE, table_text->type);
    /*EXPECT_STREQ*/ QCOMPARE("\n  ", table_text->v.text.text);

    GumboNode* tbody = GetChild(table, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tbody->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TBODY, GetTag(tbody));
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(tbody));
    // Second node is whitespace.

    GumboNode* tr = GetChild(tbody, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tr->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TR, GetTag(tr));
    /*ASSERT_EQ*/ QCOMPARE(5, GetChildCount(tr));  // Including whitespace.

    GumboNode* tr_text = GetChild(tr, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_WHITESPACE, tr_text->type);
    /*EXPECT_EQ*/ QCOMPARE(tr, tr_text->parent);
    /*EXPECT_EQ*/ QVERIFY(0 == tr_text->index_within_parent);
    /*EXPECT_STREQ*/ QCOMPARE("\n    ", tr_text->v.text.text);

    GumboNode* th = GetChild(tr, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, th->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TH, GetTag(th));
    /*EXPECT_EQ*/ QCOMPARE(tr, th->parent);
    /*EXPECT_EQ*/ QVERIFY(1 == th->index_within_parent);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(th));

    GumboNode* th_text = GetChild(th, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, th_text->type);
    /*EXPECT_STREQ*/ QCOMPARE("One", th_text->v.text.text);

    GumboNode* td = GetChild(tr, 3);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, td->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TD, GetTag(td));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(td));

    GumboNode* td_text = GetChild(td, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, td_text->type);
    /*EXPECT_STREQ*/ QCOMPARE("Two", td_text->v.text.text);

    GumboNode* td2_text = GetChild(td, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, td2_text->type);
    /*EXPECT_STREQ*/ QCOMPARE("Two", td2_text->v.text.text);

    GumboNode* div = GetChild(body, 3);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, div->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DIV, GetTag(div));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(div));
}

void tst_Parser::StartParagraphInTable()
{
    parse("<table><P></tr></td>foo</table>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* paragraph = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, paragraph->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_P, GetTag(paragraph));
    /*EXPECT_EQ*/ QCOMPARE(body, paragraph->parent);
    /*EXPECT_EQ*/ QVERIFY(0 == paragraph->index_within_parent);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(paragraph));

    GumboNode* text = GetChild(paragraph, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("foo", text->v.text.text);

    GumboNode* table = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, GetTag(table));
    /*EXPECT_EQ*/ QCOMPARE(body, table->parent);
    /*EXPECT_EQ*/ QVERIFY(1 == table->index_within_parent);
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(table));
}

void tst_Parser::EndParagraphInTable()
{
    parse("<table></p></table>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* paragraph = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, paragraph->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_P, GetTag(paragraph));
    /*EXPECT_EQ*/ QCOMPARE(body, paragraph->parent);
    /*EXPECT_EQ*/ QVERIFY(0 == paragraph->index_within_parent);
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(paragraph));

    GumboNode* table = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, GetTag(table));
    /*EXPECT_EQ*/ QCOMPARE(body, table->parent);
    /*EXPECT_EQ*/ QVERIFY(1 == table->index_within_parent);
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(table));
}

void tst_Parser::UnknownTagInTable()
{
    parse("<table><foo>bar</table>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* foo = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, foo->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_UNKNOWN, GetTag(foo));
    /*EXPECT_EQ*/ QVERIFY("<foo>" == ToString(foo->v.element.original_tag));
    /*EXPECT_EQ*/ QCOMPARE(body, foo->parent);
    /*EXPECT_EQ*/ QVERIFY(0 == foo->index_within_parent);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(foo));

    GumboNode* bar = GetChild(foo, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, bar->type);
    /*EXPECT_STREQ*/ QCOMPARE("bar", bar->v.text.text);

    GumboNode* table = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, GetTag(table));
    /*EXPECT_EQ*/ QCOMPARE(body, table->parent);
    /*EXPECT_EQ*/ QVERIFY(1 == table->index_within_parent);
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(table));
}

void tst_Parser::UnclosedTableTags()
{
    parse("<html><table>\n"
          "  <tr>\n"
          "    <td>One\n"
          "    <td>Two\n"
          "  <tr><td>Row2\n"
          "  <tr><td>Row3\n"
          "</table>\n"
          "</html>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* table = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, GetTag(table));
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(table));

    GumboNode* table_text = GetChild(table, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_WHITESPACE, table_text->type);
    /*EXPECT_STREQ*/ QCOMPARE("\n  ", table_text->v.text.text);

    GumboNode* tbody = GetChild(table, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tbody->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TBODY, GetTag(tbody));
    /*ASSERT_EQ*/ QCOMPARE(3, GetChildCount(tbody));

    GumboNode* tr = GetChild(tbody, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tr->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TR, GetTag(tr));
    /*ASSERT_EQ*/ QCOMPARE(3, GetChildCount(tr));

    GumboNode* tr_text = GetChild(tr, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_WHITESPACE, tr_text->type);
    /*EXPECT_STREQ*/ QCOMPARE("\n    ", tr_text->v.text.text);

    GumboNode* td1 = GetChild(tr, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, td1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TD, GetTag(td1));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(td1));

    GumboNode* td1_text = GetChild(td1, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, td1_text->type);
    /*EXPECT_STREQ*/ QCOMPARE("One\n    ", td1_text->v.text.text);

    GumboNode* td2 = GetChild(tr, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, td2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TD, GetTag(td2));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(td2));

    GumboNode* td2_text = GetChild(td2, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, td2_text->type);
    /*EXPECT_STREQ*/ QCOMPARE("Two\n  ", td2_text->v.text.text);

    GumboNode* tr3 = GetChild(tbody, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tr3->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TR, GetTag(tr3));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(tr3));

    GumboNode* body_text = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_WHITESPACE, body_text->type);
    /*EXPECT_STREQ*/ QCOMPARE("\n", body_text->v.text.text);
}

void tst_Parser::MisnestedTable()
{
    parse("<table><tr><div><td></div></table>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* div = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, div->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DIV, GetTag(div));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(div));

    GumboNode* table = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, GetTag(table));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(table));

    GumboNode* tbody = GetChild(table, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tbody->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TBODY, GetTag(tbody));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(tbody));

    GumboNode* tr = GetChild(tbody, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tr->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TR, GetTag(tr));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(tr));

    GumboNode* td = GetChild(tr, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, td->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TD, GetTag(td));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(td));
}

void tst_Parser::MisnestedTable2()
{
    parse("<table><td>Cell1<table><th>Cell2<tr>Cell3</table>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* table1 = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, GetTag(table1));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(table1));

    GumboNode* tbody1 = GetChild(table1, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tbody1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TBODY, GetTag(tbody1));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(tbody1));

    GumboNode* tr1 = GetChild(tbody1, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tr1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TR, GetTag(tr1));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(tr1));

    GumboNode* td1 = GetChild(tr1, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, td1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TD, GetTag(td1));
    /*ASSERT_EQ*/ QCOMPARE(3, GetChildCount(td1));

    GumboNode* cell1 = GetChild(td1, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, cell1->type);
    /*EXPECT_STREQ*/ QCOMPARE("Cell1", cell1->v.text.text);

    // Foster-parented out of the inner <tr>
    GumboNode* cell3 = GetChild(td1, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, cell3->type);
    /*EXPECT_STREQ*/ QCOMPARE("Cell3", cell3->v.text.text);

    GumboNode* table2 = GetChild(td1, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, GetTag(table2));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(table2));

    GumboNode* tbody2 = GetChild(table2, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tbody2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TBODY, GetTag(tbody2));
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(tbody2));

    GumboNode* tr2 = GetChild(tbody2, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tr2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TR, GetTag(tr2));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(tr2));

    GumboNode* th = GetChild(tr2, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, th->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TH, GetTag(th));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(th));

    GumboNode* cell2 = GetChild(th, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, cell2->type);
    /*EXPECT_STREQ*/ QCOMPARE("Cell2", cell2->v.text.text);

    GumboNode* tr3 = GetChild(tbody2, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tr3->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TR, GetTag(tr3));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(tr3));
}

void tst_Parser::Select()
{
    parse("<select><option>One<option>Two</select><div></div>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* select = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, select->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SELECT, GetTag(select));
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(select));

    GumboNode* option1 = GetChild(select, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, option1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_OPTION, GetTag(option1));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(option1));

    GumboNode* option2 = GetChild(select, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, option2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_OPTION, GetTag(option2));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(option2));

    GumboNode* div = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, div->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DIV, GetTag(div));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(div));
}

void tst_Parser::ComplicatedSelect()
{
    parse("<select><div class=foo></div><optgroup><option>Option"
          "</option><input></optgroup></select>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* select = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, select->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SELECT, GetTag(select));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(select));

    GumboNode* optgroup = GetChild(select, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, optgroup->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_OPTGROUP, GetTag(optgroup));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(optgroup));

    GumboNode* option = GetChild(optgroup, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, option->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_OPTION, GetTag(option));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(option));

    GumboNode* text = GetChild(option, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("Option", text->v.text.text);

    GumboNode* input = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, input->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_INPUT, GetTag(input));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(input));
}

void tst_Parser::DoubleSelect()
{
    parse("<select><select><div></div>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* select = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, select->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SELECT, GetTag(select));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(select));

    GumboNode* div = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, div->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DIV, GetTag(div));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(div));
}

void tst_Parser::InputInSelect()
{
    parse("<select><input /><div></div>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(3, GetChildCount(body));

    GumboNode* select = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, select->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SELECT, GetTag(select));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(select));

    GumboNode* input = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, input->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_INPUT, GetTag(input));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(input));

    GumboNode* div = GetChild(body, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, div->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DIV, GetTag(div));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(div));
}

void tst_Parser::SelectInTable()
{
    parse("<table><td><select><option value=1></table>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* table = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, GetTag(table));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(table));

    GumboNode* tbody = GetChild(table, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tbody->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TBODY, GetTag(tbody));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(tbody));

    GumboNode* tr = GetChild(tbody, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tr->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TR, GetTag(tr));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(tr));

    GumboNode* td = GetChild(tr, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, td->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TD, GetTag(td));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(td));

    GumboNode* select = GetChild(td, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, select->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SELECT, GetTag(select));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(select));

    GumboNode* option = GetChild(select, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, option->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_OPTION, GetTag(option));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(option));
}

void tst_Parser::ImplicitColgroup()
{
    parse("<table><col /><col /></table>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* table = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, GetTag(table));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(table));

    GumboNode* colgroup = GetChild(table, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, colgroup->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_COLGROUP, GetTag(colgroup));
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(colgroup));

    GumboNode* col1 = GetChild(colgroup, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, col1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_COL, GetTag(col1));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(col1));

    GumboNode* col2 = GetChild(colgroup, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, col2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_COL, GetTag(col2));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(col2));
}

void tst_Parser::Form()
{
    parse("<form><input type=hidden /><isindex /></form>After form");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* form = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, form->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_FORM, GetTag(form));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(form));

    GumboNode* input = GetChild(form, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, input->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_INPUT, GetTag(input));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(input));

    GumboNode* text = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("After form", text->v.text.text);
}

void tst_Parser::NestedForm()
{
    parse("<form><label>Label</label><form><input id=input2></form>After form");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* form = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, form->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_FORM, GetTag(form));
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(form));

    GumboNode* label = GetChild(form, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, label->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_LABEL, GetTag(label));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(label));

    GumboNode* input = GetChild(form, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, input->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_INPUT, GetTag(input));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(input));

    GumboNode* text = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("After form", text->v.text.text);
}

void tst_Parser::MisnestedFormInTable()
{
    // Parse of this is somewhat weird.  The first <form> is opened outside the
    // table, so when </form> checks to see if there's a form in scope, it stops
    // at the <table> boundary and returns null.  The form pointer is nulled out
    // anyway, though, which means that the second form (parsed in the table body
    // state) ends up creating an element.  It's immediately popped off
    // the stack, but the form element pointer remains set to that node (which is
    // not on the stack of open elements).  The final </form> tag triggers the
    // "does not have node in scope" clause and is ignored.  (Note that this is
    // different from "has a form element in scope" - the first form is still in
    // scope at that point, but the form pointer does not point to it.) Then the
    // original <form> element is closed implicitly when the table cell is closed.

    parse("<table><tr><td>"
          "<form><table><tr><td></td></tr></form>"
          "<form></tr></table></form>"
          "</td></tr></table");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* table1 = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, GetTag(table1));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(table1));

    GumboNode* tbody1 = GetChild(table1, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tbody1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TBODY, GetTag(tbody1));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(tbody1));

    GumboNode* tr1 = GetChild(tbody1, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tr1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TR, GetTag(tr1));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(tr1));

    GumboNode* td1 = GetChild(tr1, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, td1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TD, GetTag(td1));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(td1));

    GumboNode* form1 = GetChild(td1, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, form1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_FORM, GetTag(form1));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(form1));

    GumboNode* table2 = GetChild(form1, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, GetTag(table2));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(table2));

    GumboNode* tbody2 = GetChild(table2, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tbody2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TBODY, GetTag(tbody2));
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(tbody2));

    GumboNode* tr2 = GetChild(tbody2, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tr2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TR, GetTag(tr2));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(tr2));

    GumboNode* form2 = GetChild(tbody2, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, form2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_FORM, GetTag(form2));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(form2));
}

void tst_Parser::IsIndex()
{
    parse("<isindex id=form1 action='/action' prompt='Secret Message'>");
    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* form = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, form->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_FORM, GetTag(form));
    /*ASSERT_EQ*/ QCOMPARE(3, GetChildCount(form));

    GumboAttribute* action = GetAttribute(form, 0);
    /*EXPECT_STREQ*/ QCOMPARE("action", action->name);
    /*EXPECT_STREQ*/ QCOMPARE("/action", action->value);

    GumboNode* hr1 = GetChild(form, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, hr1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HR, GetTag(hr1));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(hr1));

    GumboNode* label = GetChild(form, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, label->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_LABEL, GetTag(label));
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(label));

    GumboNode* text = GetChild(label, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("Secret Message", text->v.text.text);

    GumboNode* input = GetChild(label, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, input->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_INPUT, GetTag(input));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(input));
    /*ASSERT_EQ*/ QCOMPARE(2, GetAttributeCount(input));

    GumboAttribute* id = GetAttribute(input, 0);
    /*EXPECT_STREQ*/ QCOMPARE("id", id->name);
    /*EXPECT_STREQ*/ QCOMPARE("form1", id->value);

    GumboAttribute* name = GetAttribute(input, 1);
    /*EXPECT_STREQ*/ QCOMPARE("name", name->name);
    /*EXPECT_STREQ*/ QCOMPARE("isindex", name->value);

    GumboNode* hr2 = GetChild(form, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, hr2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HR, GetTag(hr2));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(hr2));
}

void tst_Parser::IsIndexDuplicateAttribute()
{
    parse("<isindex name=foo>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* form = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, form->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_FORM, GetTag(form));
    /*ASSERT_EQ*/ QCOMPARE(3, GetChildCount(form));

    GumboNode* label = GetChild(form, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, label->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_LABEL, GetTag(label));
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(label));

    GumboNode* input = GetChild(label, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, input->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_INPUT, GetTag(input));
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(input));
    /*ASSERT_EQ*/ QCOMPARE(1, GetAttributeCount(input));

    GumboAttribute* name = GetAttribute(input, 0);
    /*EXPECT_STREQ*/ QCOMPARE("name", name->name);
    /*EXPECT_STREQ*/ QCOMPARE("isindex", name->value);
}

void tst_Parser::NestedRawtextTags()
{
    parse("<noscript><noscript jstag=false>"
          "<style>div{text-align:center}</style></noscript>");

    GumboNode* html = GetChild(root_, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, html->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HTML, GetTag(html));
    /*EXPECT_EQ*/ QVERIFY((GUMBO_INSERTION_BY_PARSER | GUMBO_INSERTION_IMPLICIT_END_TAG |
                          GUMBO_INSERTION_IMPLIED) == html->parse_flags);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(html));

    GumboNode* head = GetChild(html, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, head->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HEAD, GetTag(head));
    /*EXPECT_EQ*/ QVERIFY((GUMBO_INSERTION_BY_PARSER | GUMBO_INSERTION_IMPLICIT_END_TAG |
                          GUMBO_INSERTION_IMPLIED) == head->parse_flags);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(head));

    GumboNode* noscript = GetChild(head, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, noscript->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_NOSCRIPT, GetTag(noscript));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(noscript));

    GumboNode* style = GetChild(noscript, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, style->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_STYLE, GetTag(style));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(style));

    GumboNode* text = GetChild(style, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("div{text-align:center}", text->v.text.text);
}

void tst_Parser::RawtextInBody()
{
    parse("<body><noembed jsif=false></noembed>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* noembed = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, noembed->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_NOEMBED, GetTag(noembed));
    /*EXPECT_EQ*/ QCOMPARE(1, GetAttributeCount(noembed));
}

void tst_Parser::MetaBeforeHead()
{
    parse("<html><meta http-equiv='content-type' "
          "content='text/html; charset=UTF-8' /><head></head>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    // Testing for a memory leak here, but
    // TODO(jdtang): Flesh out structural asserts.
}

void tst_Parser::NoahsArkClause()
{
    parse("<p><font size=4><font color=red><font size=4><font size=4>"
          "<font size=4><font size=4><font size=4><font color=red><p>X");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* p1 = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, p1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_P, p1->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(p1));

    GumboNode* size1 = GetChild(p1, 0);
    GumboNode* red1 = GetChild(size1, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, red1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_FONT, red1->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetAttributeCount(red1));
    GumboAttribute* red1_attr = GetAttribute(red1, 0);
    /*EXPECT_STREQ*/ QCOMPARE("color", red1_attr->name);
    /*EXPECT_STREQ*/ QCOMPARE("red", red1_attr->value);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(red1));

    GumboNode* p2 = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, p2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_P, p2->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(p2));

    GumboNode* red2 = GetChild(p2, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, red2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_FONT, red2->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetAttributeCount(red2));
    GumboAttribute* red2_attr = GetAttribute(red2, 0);
    /*EXPECT_STREQ*/ QCOMPARE("color", red2_attr->name);
    /*EXPECT_STREQ*/ QCOMPARE("red", red2_attr->value);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(red2));
}

void tst_Parser::AdoptionAgency1()
{
    // http://www.whatwg.org/specs/web-apps/current-work/multipage/the-end.html#misnested-tags:-b-i-/b-/i
    parse("<p>1<b>2<i>3</b>4</i>5</p>");

    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(root_));

    GumboNode* html = GetChild(root_, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, html->type);
    /*EXPECT_EQ*/ QVERIFY((GUMBO_INSERTION_BY_PARSER | GUMBO_INSERTION_IMPLICIT_END_TAG |
                          GUMBO_INSERTION_IMPLIED) == html->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HTML, html->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(html));

    GumboNode* body = GetChild(html, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, body->type);
    /*EXPECT_EQ*/ QVERIFY((GUMBO_INSERTION_BY_PARSER | GUMBO_INSERTION_IMPLICIT_END_TAG |
                          GUMBO_INSERTION_IMPLIED) == body->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BODY, body->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* p = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, p->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, p->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_P, p->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(4, GetChildCount(p));

    GumboNode* text1 = GetChild(p, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text1->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("1", text1->v.text.text);

    GumboNode* b = GetChild(p, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, b->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, b->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_B, b->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(b));

    GumboNode* text2 = GetChild(b, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text2->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("2", text2->v.text.text);

    GumboNode* i = GetChild(b, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, i->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_IMPLICIT_END_TAG, i->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_I, i->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(i));

    GumboNode* text3 = GetChild(i, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text3->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text2->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("3", text3->v.text.text);

    GumboNode* i2 = GetChild(p, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, i2->type);
    /*EXPECT_EQ*/ QVERIFY((GUMBO_INSERTION_BY_PARSER |
                           GUMBO_INSERTION_RECONSTRUCTED_FORMATTING_ELEMENT) == i2->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_I, i2->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(i2));

    GumboNode* text4 = GetChild(i2, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text4->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text2->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("4", text4->v.text.text);

    GumboNode* text5 = GetChild(p, 3);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text5->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text2->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("5", text5->v.text.text);
}

void tst_Parser::AdoptionAgency2()
{
    // http://www.whatwg.org/specs/web-apps/current-work/multipage/the-end.html#misnested-tags:-b-p-/b-/p
    parse("<b>1<p>2</b>3</p>");
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(root_));

    GumboNode* html = GetChild(root_, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, html->type);
    /*EXPECT_EQ*/ QVERIFY((GUMBO_INSERTION_BY_PARSER | GUMBO_INSERTION_IMPLICIT_END_TAG |
                          GUMBO_INSERTION_IMPLIED) == html->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HTML, html->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(html));

    GumboNode* body = GetChild(html, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, body->type);
    /*EXPECT_EQ*/ QVERIFY((GUMBO_INSERTION_BY_PARSER | GUMBO_INSERTION_IMPLICIT_END_TAG |
                          GUMBO_INSERTION_IMPLIED) == body->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_BODY, body->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* b = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, b->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_IMPLICIT_END_TAG, b->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_B, b->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(b));

    GumboNode* text1 = GetChild(b, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text1->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("1", text1->v.text.text);

    GumboNode* p = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, p->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_ADOPTION_AGENCY_MOVED, p->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_P, p->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(p));

    GumboNode* b2 = GetChild(p, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, b2->type);
    /*EXPECT_EQ*/ QVERIFY((GUMBO_INSERTION_ADOPTION_AGENCY_CLONED |
                          GUMBO_INSERTION_BY_PARSER) == b2->parse_flags);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_B, b2->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(b2));

    GumboNode* text2 = GetChild(b2, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text2->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("2", text2->v.text.text);

    GumboNode* text3 = GetChild(p, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text3->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text2->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("3", text3->v.text.text);
}

void tst_Parser::AdoptionAgency3()
{
    parse("<div><a><b><u><i><code><div></a>");
}

void tst_Parser::ImplicitlyCloseLists()
{
    parse("<ul>\n"
          "  <li>First\n"
          "  <li>Second\n"
          "</ul>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* ul = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, ul->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_UL, GetTag(ul));
    /*ASSERT_EQ*/ QCOMPARE(3, GetChildCount(ul));

    GumboNode* text = GetChild(ul, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_WHITESPACE, text->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("\n  ", text->v.text.text);

    GumboNode* li1 = GetChild(ul, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, li1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_LI, GetTag(li1));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(li1));

    GumboNode* li2 = GetChild(ul, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, li2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_LI, GetTag(li2));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(li2));
}

void tst_Parser::CData()
{
    parse("<svg><![CDATA[this is text]]></svg>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* svg = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(svg));

    GumboNode* cdata = GetChild(svg, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_CDATA, cdata->type);
    /*EXPECT_STREQ*/ QCOMPARE("this is text", cdata->v.text.text);
}

void tst_Parser::CDataUnsafe()
{
    // Can't use Parse() because of the strlen
    output_ =
            gumbo_parse_with_options(&options_, "<svg><![CDATA[\0filler\0text\0]]>",
                                     sizeof("<svg><![CDATA[\0filler\0text\0]]>") - 1);
    root_ = output_->document;

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* svg = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(svg));

    GumboNode* cdata = GetChild(svg, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_CDATA, cdata->type);
    // \xEF\xBF\xBD = unicode replacement char
    /*EXPECT_STREQ*/ QCOMPARE(
                "\xEF\xBF\xBD"
                "filler\xEF\xBF\xBD"
                "text\xEF\xBF\xBD",
                cdata->v.text.text);
}

void tst_Parser::CDataInBody()
{
    parse("<div><![CDATA[this is text]]></div>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* div = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(div));

    GumboNode* cdata = GetChild(div, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_COMMENT, cdata->type);
    /*EXPECT_STREQ*/ QCOMPARE("[CDATA[this is text]]", cdata->v.text.text);
}

void tst_Parser::FormattingTagsInHeading()
{
    parse("<h2>This is <b>old</h2>text");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    GumboNode* h2 = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, h2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_H2, GetTag(h2));
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(h2));

    GumboNode* text1 = GetChild(h2, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text1->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("This is ", text1->v.text.text);

    GumboNode* b = GetChild(h2, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, b->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_B, GetTag(b));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_IMPLICIT_END_TAG, b->parse_flags);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(b));

    GumboNode* text2 = GetChild(b, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text2->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("old", text2->v.text.text);

    GumboNode* b2 = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, b2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_B, GetTag(b2));
    /*EXPECT_EQ*/ QVERIFY((GUMBO_INSERTION_IMPLICIT_END_TAG | GUMBO_INSERTION_BY_PARSER |
                          GUMBO_INSERTION_RECONSTRUCTED_FORMATTING_ELEMENT) == b2->parse_flags);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(b2));

    GumboNode* text3 = GetChild(b2, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text3->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text3->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("text", text3->v.text.text);
}

void tst_Parser::ExtraReconstruction()
{
    parse("<span><b></span></p>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(body));

    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SPAN, GetTag(GetChild(body, 0)));
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_P, GetTag(GetChild(body, 1)));
}

void tst_Parser::LinkifiedHeading()
{
    parse("<li><h3><a href=#foo>Text</a></h3><div>Summary</div>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* li = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, li->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_LI, GetTag(li));
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(li));

    GumboNode* h3 = GetChild(li, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, h3->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_H3, GetTag(h3));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(h3));

    GumboNode* anchor = GetChild(h3, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, anchor->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_A, GetTag(anchor));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(anchor));

    GumboNode* div = GetChild(li, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, div->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DIV, GetTag(div));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(div));
}

void tst_Parser::MisnestedHeading()
{
    parse("<h1>"
          "  <section>"
          "    <h2>"
          "      <dl><dt>List"
          "    </h1>"
          "  </section>"
          "  Heading1"
          "<h3>Heading3</h4>"
          "After</h3> text");

    // The parse of this is pretty weird: according to the spec, it should be:
    // <html>
    //   <head></head>
    //   <body>
    //     <h1>
    //       <section>
    //         <h2><dl><dt>List</dt></dl></h2>
    //       </section>
    //       Heading1
    //     </h1>
    //     <h3>Heading3</h3>
    //     After text
    //   </body>
    // </html>
    // Explanation:
    // <html>, <head>, and <body> tags are implied.  The opening <h1> and <section
    // tags function as expected.  Because the current node is <section>, the <h2>
    // does *not* close the existing <h1>, and then we enter a definition list.
    // The closing </h1>, even though it's misnested, causes the <dt> to be closed
    // implicitly, then also closes the <dl> and <h2> as a parse error.  <h1> is
    // still open, and so "Heading1" goes into it.  Because the current node is a
    // heading tag, <h3> closes it (as a parse error) and reopens a new <h3> node,
    // which is closed by the </h4> tag.  The remaining text goes straight into
    // the <body>; since no heading is open, the </h3> tag is ignored and the
    // second run is condensed into the first.
    // TODO(jdtang): Make sure that parse_flags are set appropriately for this.

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(3, GetChildCount(body));

    GumboNode* h1 = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, h1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_H1, GetTag(h1));
    /*ASSERT_EQ*/ QCOMPARE(3, GetChildCount(h1));
    // Child 1 is whitespace, as it is for many of these nodes.

    GumboNode* section = GetChild(h1, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, section->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SECTION, GetTag(section));
    /*ASSERT_EQ*/ QCOMPARE(3, GetChildCount(section));

    GumboNode* h2 = GetChild(section, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, h2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_H2, GetTag(h2));
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(h2));

    GumboNode* dl = GetChild(h2, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, dl->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DL, GetTag(dl));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(dl));

    GumboNode* dt = GetChild(dl, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, dt->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DT, GetTag(dt));
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(dt));

    GumboNode* text1 = GetChild(dt, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text1->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text1->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("List    ", text1->v.text.text);

    GumboNode* text2 = GetChild(h1, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text2->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text2->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("  Heading1", text2->v.text.text);

    GumboNode* h3 = GetChild(body, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, h3->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_H3, GetTag(h3));
    /*EXPECT_EQ*/ QCOMPARE(1, GetChildCount(h3));

    GumboNode* text3 = GetChild(h3, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text3->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text3->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("Heading3", text3->v.text.text);

    GumboNode* text4 = GetChild(body, 2);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text4->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text4->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("After text", text4->v.text.text);
}

void tst_Parser::DoubleBody()
{
    parse("<body class=first><body class=second id=merged>Text</body></body>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));
    /*ASSERT_EQ*/ QCOMPARE(2, GetAttributeCount(body));

    GumboAttribute* clas = GetAttribute(body, 0);
    /*EXPECT_STREQ*/ QCOMPARE("class", clas->name);
    /*EXPECT_STREQ*/ QCOMPARE("first", clas->value);

    GumboAttribute* id = GetAttribute(body, 1);
    /*EXPECT_STREQ*/ QCOMPARE("id", id->name);
    /*EXPECT_STREQ*/ QCOMPARE("merged", id->value);

    GumboNode* text = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_INSERTION_NORMAL, text->parse_flags);
    /*EXPECT_STREQ*/ QCOMPARE("Text", text->v.text.text);
}

void tst_Parser::ThInMathMl()
{
    parse("<math><th><mI><table></table><tr></table><div><tr>0");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* math = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, math->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_MATH, math->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NAMESPACE_MATHML, math->v.element.tag_namespace);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(math));

    GumboNode* th = GetChild(math, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, th->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TH, th->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NAMESPACE_MATHML, th->v.element.tag_namespace);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(th));

    GumboNode* mi = GetChild(th, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, mi->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_MI, mi->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NAMESPACE_MATHML, mi->v.element.tag_namespace);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(mi));

    GumboNode* table = GetChild(mi, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, table->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NAMESPACE_HTML, table->v.element.tag_namespace);
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(table));

    GumboNode* div = GetChild(mi, 1);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, div->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DIV, div->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NAMESPACE_HTML, div->v.element.tag_namespace);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(div));

    GumboNode* text = GetChild(div, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEXT, text->type);
    /*EXPECT_STREQ*/ QCOMPARE("0", text->v.text.text);
}

void tst_Parser::TdInMathml()
{
    parse("<table><th><math><td></tr>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(body));

    GumboNode* table = GetChild(body, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, table->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TABLE, table->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NAMESPACE_HTML, table->v.element.tag_namespace);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(table));

    GumboNode* tbody = GetChild(table, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tbody->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TBODY, tbody->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NAMESPACE_HTML, tbody->v.element.tag_namespace);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(tbody));

    GumboNode* tr = GetChild(tbody, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, tr->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TR, tr->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NAMESPACE_HTML, tr->v.element.tag_namespace);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(tr));

    GumboNode* th = GetChild(tr, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, th->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TH, th->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NAMESPACE_HTML, th->v.element.tag_namespace);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(th));

    GumboNode* math = GetChild(th, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, math->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_MATH, math->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NAMESPACE_MATHML, math->v.element.tag_namespace);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(math));

    GumboNode* td = GetChild(math, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, td->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TD, td->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NAMESPACE_MATHML, td->v.element.tag_namespace);
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(td));
}

void tst_Parser::SelectInForeignContent()
{
    parse("<svg><select><foreignobject><select><select><select>");
}

void tst_Parser::TemplateInForeignContent()
{
    parse("<template><svg><template>");

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*EXPECT_EQ*/ QCOMPARE(0, GetChildCount(body));

    GumboNode* html = GetChild(root_, 0);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(html));

    GumboNode* head = GetChild(html, 0);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(head));

    GumboNode* template_node = GetChild(head, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEMPLATE, template_node->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TEMPLATE, template_node->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(template_node));

    GumboNode* svg_node = GetChild(template_node, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, svg_node->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_SVG, svg_node->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NAMESPACE_SVG, svg_node->v.element.tag_namespace);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(svg_node));

    GumboNode* svg_template = GetChild(svg_node, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, svg_template->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TEMPLATE, svg_template->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_NAMESPACE_SVG, svg_template->v.element.tag_namespace);
    /*EXPECT_EQ*/ QCOMPARE(0, GetChildCount(svg_template));
}

void tst_Parser::TemplateNull()
{
    output_ = gumbo_parse_with_options(&options_, "<template>\0", sizeof("<template>\0") - 1);
    root_ = output_->document;

    GumboNode* body;
    GetAndAssertBody(root_, &body);
    /*EXPECT_EQ*/ QCOMPARE(0, GetChildCount(body));

    GumboNode* html = GetChild(root_, 0);
    /*ASSERT_EQ*/ QCOMPARE(2, GetChildCount(html));

    GumboNode* head = GetChild(html, 0);
    /*ASSERT_EQ*/ QCOMPARE(1, GetChildCount(head));

    GumboNode* template_node = GetChild(head, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_TEMPLATE, template_node->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_TEMPLATE, template_node->v.element.tag);
    /*ASSERT_EQ*/ QCOMPARE(0, GetChildCount(template_node));
}

void tst_Parser::FragmentWithNamespace()
{
    parseFragment("<div></div>", GUMBO_TAG_TITLE, GUMBO_NAMESPACE_SVG);

    /*EXPECT_EQ*/ QCOMPARE(1, GetChildCount(root_));
    GumboNode* html = GetChild(root_, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, html->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_HTML, html->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(1, GetChildCount(html));

    GumboNode* div = GetChild(html, 0);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, div->type);
    /*EXPECT_EQ*/ QCOMPARE(GUMBO_TAG_DIV, div->v.element.tag);
    /*EXPECT_EQ*/ QCOMPARE(0, GetChildCount(div));
}

/******************************************************************************
 ******************************************************************************/

QTEST_APPLESS_MAIN(tst_Parser)

#include "tst_parser.moc"
