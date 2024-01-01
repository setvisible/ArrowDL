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

#include "vector.h"

#include <QtCore/QDebug>
#include <QtTest/QtTest>


class tst_Vector : public GumboTest
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void InitializeTheTest();
    void InitZeroCapacity();
    void Add();
    void AddMultiple();
    void Realloc();
    void Pop();
    void PopEmpty();
    void InsertAtFirst();
    void InsertAtLast();
    void Remove();
    void RemoveAt();

private:    
    GumboVector vector_;

    // dummy ints that we can use to take addresses of.
    int one_;
    int two_;
    int three_;

    // Counters for testing a custom allocator.
    int num_allocations_;
    size_t bytes_allocated_;
    int num_deallocations_;

    void* custom_gumbo_vector_allocator(void* userdata, size_t num_bytes);
    void custom_gumbo_vector_deallocator(void* userdata, void* ptr);
};

/******************************************************************************
 ******************************************************************************/
void tst_Vector::init()
{
    one_ = 1;
    two_ = 2;
    three_ = 3;
    num_allocations_ = 0;
    bytes_allocated_ = 0;
    num_deallocations_ = 0;

    gumbo_vector_init(&parser_, 2, &vector_);
}

void tst_Vector::cleanup()
{
    gumbo_vector_destroy(&parser_, &vector_);
}

/******************************************************************************
 ******************************************************************************/
void* tst_Vector::custom_gumbo_vector_allocator(void* userdata, size_t num_bytes)
{
    tst_Vector* test = static_cast<tst_Vector*>(userdata);
    ++test->num_allocations_;
    test->bytes_allocated_ += num_bytes;
    return malloc(num_bytes);
}

void tst_Vector::custom_gumbo_vector_deallocator(void* userdata, void* ptr)
{
    tst_Vector* test = static_cast<tst_Vector*>(userdata);
    ++test->num_deallocations_;
    free(ptr);
}

/******************************************************************************
 ******************************************************************************/
void tst_Vector::InitializeTheTest()
{
    /*EXPECT_EQ*/ QVERIFY(0 == vector_.length);
    /*EXPECT_EQ*/ QVERIFY(2 == vector_.capacity);
}

void tst_Vector::InitZeroCapacity()
{
    gumbo_vector_destroy(&parser_, &vector_);
    gumbo_vector_init(&parser_, 0, &vector_);
    gumbo_vector_add(&parser_, &one_, &vector_);
    /*EXPECT_EQ*/ QVERIFY(1 == vector_.length);
    /*EXPECT_EQ*/ QCOMPARE(1, *(static_cast<int*>(vector_.data[0])));
}

void tst_Vector::Add()
{
    gumbo_vector_add(&parser_, &one_, &vector_);
    /*EXPECT_EQ*/ QVERIFY(1 == vector_.length);
    /*EXPECT_EQ*/ QCOMPARE(1, *(static_cast<int*>(vector_.data[0])));
    /*EXPECT_EQ*/ QCOMPARE(0, gumbo_vector_index_of(&vector_, &one_));
    /*EXPECT_EQ*/ QCOMPARE(-1, gumbo_vector_index_of(&vector_, &two_));
}

void tst_Vector::AddMultiple()
{
    gumbo_vector_add(&parser_, &one_, &vector_);
    gumbo_vector_add(&parser_, &two_, &vector_);
    /*EXPECT_EQ*/ QVERIFY(2 == vector_.length);
    /*EXPECT_EQ*/ QCOMPARE(2, *(static_cast<int*>(vector_.data[1])));
    /*EXPECT_EQ*/ QCOMPARE(1, gumbo_vector_index_of(&vector_, &two_));
}

void tst_Vector::Realloc()
{
    gumbo_vector_add(&parser_, &one_, &vector_);
    gumbo_vector_add(&parser_, &two_, &vector_);
    gumbo_vector_add(&parser_, &three_, &vector_);
    /*EXPECT_EQ*/ QVERIFY(3 == vector_.length);
    /*EXPECT_EQ*/ QVERIFY(4 == vector_.capacity);
    /*EXPECT_EQ*/ QCOMPARE(3, *(static_cast<int*>(vector_.data[2])));
}

void tst_Vector::Pop()
{
    gumbo_vector_add(&parser_, &one_, &vector_);
    int result = *static_cast<int*>(gumbo_vector_pop(&parser_, &vector_));
    /*EXPECT_EQ*/ QCOMPARE(1, result);
    /*EXPECT_EQ*/ QVERIFY(0 == vector_.length);
}

void tst_Vector::PopEmpty()
{
    /*EXPECT_EQ*/ QVERIFY(NULL == gumbo_vector_pop(&parser_, &vector_));
}

void tst_Vector::InsertAtFirst()
{
    gumbo_vector_add(&parser_, &one_, &vector_);
    gumbo_vector_add(&parser_, &two_, &vector_);
    gumbo_vector_insert_at(&parser_, &three_, 0, &vector_);
    /*EXPECT_EQ*/ QVERIFY(3 == vector_.length);
    int result = *static_cast<int*>(vector_.data[0]);
    /*EXPECT_EQ*/ QCOMPARE(3, result);
}

void tst_Vector::InsertAtLast()
{
    gumbo_vector_add(&parser_, &one_, &vector_);
    gumbo_vector_add(&parser_, &two_, &vector_);
    gumbo_vector_insert_at(&parser_, &three_, 2, &vector_);
    /*EXPECT_EQ*/ QVERIFY(3 == vector_.length);
    int result = *static_cast<int*>(vector_.data[2]);
    /*EXPECT_EQ*/ QCOMPARE(3, result);
}

void tst_Vector::Remove()
{
    gumbo_vector_add(&parser_, &one_, &vector_);
    gumbo_vector_add(&parser_, &two_, &vector_);
    gumbo_vector_add(&parser_, &three_, &vector_);
    gumbo_vector_remove(&parser_, &two_, &vector_);
    /*EXPECT_EQ*/ QVERIFY(2 == vector_.length);
    int three = *static_cast<int*>(vector_.data[1]);
    /*EXPECT_EQ*/ QCOMPARE(3, three);
}

void tst_Vector::RemoveAt()
{
    gumbo_vector_add(&parser_, &one_, &vector_);
    gumbo_vector_add(&parser_, &two_, &vector_);
    gumbo_vector_add(&parser_, &three_, &vector_);
    int result = *static_cast<int*>(gumbo_vector_remove_at(&parser_, 1, &vector_));
    /*EXPECT_EQ*/ QCOMPARE(2, result);
    /*EXPECT_EQ*/ QVERIFY(2 == vector_.length);
    int three = *static_cast<int*>(vector_.data[1]);
    /*EXPECT_EQ*/ QCOMPARE(3, three);
}

/******************************************************************************
 ******************************************************************************/

QTEST_APPLESS_MAIN(tst_Vector)

#include "tst_vector.moc"
