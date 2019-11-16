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

#ifndef GUMBO_TEST_UTILS_H
#define GUMBO_TEST_UTILS_H

#include <assert.h>
#include <stdint.h>
#include <string>

#include "gumbo.h"
#include "parser.h"

#include <QtCore/QObject>

inline std::string ToString(const GumboStringPiece& str) {
    return std::string(str.data, str.length);
}
inline QString toQString(const GumboStringPiece& str) {
    return QString::fromStdString(std::string(str.data, str.length));
}

unsigned int GetChildCount(GumboNode* node);
GumboTag GetTag(GumboNode* node);
GumboNode* GetChild(GumboNode* parent, int index);
int GetAttributeCount(GumboNode* node);
GumboAttribute* GetAttribute(GumboNode* node, int index);

// Convenience function to do some basic assertions on the structure of the
// document (nodes are elements, nodes have the right tags) and then return
// the body node.
void GetAndAssertBody(GumboNode* root, GumboNode** body);
void SanityCheckPointers(
        const char* input, size_t input_length, const GumboNode* node, int depth);

// Custom allocator machinery to sanity check for memory leaks.  Normally we can
// use heapcheck/valgrind/ASAN for this, but they only give the
// results when the program terminates.  This means that if the parser is run in
// a loop (say, a MapReduce) and there's a leak, it may end up exhausting memory
// before it can catch the particular document responsible for the leak.  These
// allocators let us check each document individually for leaks.

typedef struct {
    uint64_t bytes_allocated;
    uint64_t objects_allocated;
    uint64_t objects_freed;
} MallocStats;

void InitLeakDetection(GumboOptions* options, MallocStats* stats);

// Base class for Gumbo tests.  This provides an GumboParser object that's
// been initialized to sane values, as normally happens in the beginning of
// gumbo_parse, and then a destructor that cleans up after it.

class GumboTest : public QObject
{
    Q_OBJECT

protected:
    GumboTest();
    virtual ~GumboTest();

protected slots:
    void init();
    void cleanup();

protected:
    MallocStats malloc_stats_;
    GumboOptions options_;
    GumboParser parser_;
    bool errors_are_expected_;
    const char* text_;
};

#endif // GUMBO_TEST_UTILS_H
