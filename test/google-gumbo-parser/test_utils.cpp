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

#include "test_utils.h"

#include "error.h"
#include "util.h"

#include <QtTest/QtTest>
#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

int GetChildCount(GumboNode* node) {
    if (node->type == GUMBO_NODE_DOCUMENT) {
        return node->v.document.children.length;
    } else {
        return node->v.element.children.length;
    }
}

GumboTag GetTag(GumboNode* node) { return node->v.element.tag; }

GumboNode* GetChild(GumboNode* parent, int index) {
    if (parent->type == GUMBO_NODE_DOCUMENT) {
        return static_cast<GumboNode*>(parent->v.document.children.data[index]);
    } else {
        return static_cast<GumboNode*>(parent->v.element.children.data[index]);
    }
}

int GetAttributeCount(GumboNode* node) {
    return node->v.element.attributes.length;
}

GumboAttribute* GetAttribute(GumboNode* node, int index) {
    return static_cast<GumboAttribute*>(node->v.element.attributes.data[index]);
}

// Convenience function to do some basic assertions on the structure of the
// document (nodes are elements, nodes have the right tags) and then return
// the body node.
void GetAndAssertBody(GumboNode* root, GumboNode** body) {
    GumboNode* html = NULL;
    for (int i = 0; i < GetChildCount(root); ++i) {
        GumboNode* child = GetChild(root, i);
        if (child->type != GUMBO_NODE_ELEMENT) {
            /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_COMMENT, child->type);
            continue;
        }
        /*ASSERT_TRUE*/ QVERIFY(html == NULL);
        html = child;
    }
    /*ASSERT_TRUE*/ QVERIFY(html != NULL);
    /*ASSERT_EQ*/ QCOMPARE(GUMBO_NODE_ELEMENT, html->type);
    /*EXPECT_EQ*/ QVERIFY(GUMBO_TAG_HTML == GetTag(html));

    // There may be comment/whitespace nodes; this walks through the children of
    // <html> and assigns head/body based on them, or assert-fails if there are
    // fewer/more than 2 such nodes.
    GumboNode* head = NULL;
    *body = NULL;
    for (int i = 0; i < GetChildCount(html); ++i) {
        GumboNode* child = GetChild(html, i);
        if (child->type != GUMBO_NODE_ELEMENT) {
            continue;
        }

        if (!head) {
            head = child;
            /*EXPECT_EQ*/ QVERIFY(GUMBO_TAG_HEAD == GetTag(head));
        } else if (!(*body)) {
            *body = child;
            /*EXPECT_EQ*/ QVERIFY(GUMBO_TAG_BODY == GetTag(*body));
        } else {
            /*ASSERT_TRUE*/ QVERIFY("More than two elements found inside <html>" != NULL);
        }
    }
    /*EXPECT_TRUE*/ QVERIFY(head != NULL);
    /*ASSERT_TRUE*/ QVERIFY(*body != NULL);
}

void SanityCheckPointers(
        const char* input, size_t input_length, const GumboNode* node, int depth) {
    /*ASSERT_GE*/ QVERIFY(input_length >= (size_t) 0);
    /*ASSERT_TRUE*/ QVERIFY(node != NULL);
    // There are some truly pathological HTML documents out there - the
    // integration tests for this include one where the DOM "tree" is actually a
    // linked list 27,500 nodes deep - and so we need a limit on the recursion
    // depth here to avoid blowing the stack. Alternatively, we could externalize
    // the stack and use an iterative algorithm, but that gets us very little for
    // the additional programming complexity.
    if (node->type == GUMBO_NODE_DOCUMENT || depth > 400) {
        // Don't sanity-check the document as well...we start with the root.
        return;
    }
    if (node->type == GUMBO_NODE_ELEMENT) {
        const GumboElement* element = &node->v.element;
        // Sanity checks on original* pointers, making sure they fall within the
        // original input.
        if (element->original_tag.data && element->original_tag.length) {
            /*EXPECT_GE*/ QVERIFY(element->original_tag.data >= input);
            /*EXPECT_LT*/ QVERIFY(element->original_tag.data < input + input_length);
            /*EXPECT_LE*/ QVERIFY(element->original_tag.length <= input_length);
        }
        if (element->original_end_tag.data && element->original_tag.length) {
            /*EXPECT_GE*/ QVERIFY(element->original_end_tag.data >= input);
            /*EXPECT_LT*/ QVERIFY(element->original_end_tag.data < input + input_length);
            /*EXPECT_LE*/ QVERIFY(element->original_end_tag.length <= input_length);
        }
        /*EXPECT_GE*/ QVERIFY(element->start_pos.offset >= 0);
        /*EXPECT_LE*/ QVERIFY(element->start_pos.offset <= input_length);
        /*EXPECT_GE*/ QVERIFY(element->end_pos.offset >= 0);
        /*EXPECT_LE*/ QVERIFY(element->end_pos.offset <= input_length);

        const GumboVector* children = &element->children;
        for (int i = 0; i < children->length; ++i) {
            const GumboNode* child = static_cast<const GumboNode*>(children->data[i]);
            // Checks on parent/child links.
            /*ASSERT_TRUE*/ QVERIFY(child != NULL);
            /*EXPECT_EQ*/ QVERIFY(node == child->parent);
            /*EXPECT_EQ*/ QVERIFY(i == child->index_within_parent);
            SanityCheckPointers(input, input_length, child, depth + 1);
        }
    } else {
        const GumboText* text = &node->v.text;
        /*EXPECT_GE*/ QVERIFY(text->original_text.data >= input);
        /*EXPECT_LT*/ QVERIFY(text->original_text.data < input + input_length);
        /*EXPECT_LE*/ QVERIFY(text->original_text.length <= input_length);
        /*EXPECT_GE*/ QVERIFY(text->start_pos.offset >= 0);
        /*EXPECT_LT*/ QVERIFY(text->start_pos.offset < input_length);
    }
}

// Custom allocator machinery to sanity check for memory leaks.  Normally we can
// use heapcheck/valgrind/ASAN for this, but they only give the
// results when the program terminates.  This means that if the parser is run in
// a loop (say, a MapReduce) and there's a leak, it may end up exhausting memory
// before it can catch the particular document responsible for the leak.  These
// allocators let us check each document individually for leaks.

static void* LeakDetectingMalloc(void* userdata, size_t size) {
    MallocStats* stats = static_cast<MallocStats*>(userdata);
    stats->bytes_allocated += size;
    ++stats->objects_allocated;
    // Arbitrary limit of 2G on allocation; parsing any reasonable document
    // shouldn't take more than that.
    assert(stats->bytes_allocated < (1 << 31));
    void* obj = malloc(size);
    // gumbo_debug("Allocated %u bytes at %x.\n", size, obj);
    return obj;
}

static void LeakDetectingFree(void* userdata, void* ptr) {
    MallocStats* stats = static_cast<MallocStats*>(userdata);
    if (ptr) {
        ++stats->objects_freed;
        // gumbo_debug("Freed %x.\n");
        free(ptr);
    }
}

void InitLeakDetection(GumboOptions* options, MallocStats* stats) {
    stats->bytes_allocated = 0;
    stats->objects_allocated = 0;
    stats->objects_freed = 0;

    options->allocator = LeakDetectingMalloc;
    options->deallocator = LeakDetectingFree;
    options->userdata = stats;
}

void GumboTest::init()
{
    options_ = kGumboDefaultOptions;
    errors_are_expected_ = false;
    text_ = "";;

    InitLeakDetection(&options_, &malloc_stats_);
    options_.max_errors = 100;
    parser_._options = &options_;
    parser_._output = static_cast<GumboOutput*>(gumbo_parser_allocate(&parser_, sizeof(GumboOutput)));
    gumbo_init_errors(&parser_);
}

void GumboTest::cleanup()
{
    if (!errors_are_expected_) {
        // TODO(jdtang): A googlemock matcher may be a more appropriate solution for
        // this; we only want to pretty-print errors that are not an expected
        // output of the test.
        for (unsigned int i = 0; i < parser_._output->errors.length && i < 1; ++i) {
            gumbo_print_caret_diagnostic(&parser_, static_cast<GumboError*>(parser_._output->errors.data[i]), text_);
        }
    }
    gumbo_destroy_errors(&parser_);
    gumbo_parser_deallocate(&parser_, parser_._output);
    /*EXPECT_EQ*/ QCOMPARE(malloc_stats_.objects_allocated, malloc_stats_.objects_freed);
}
