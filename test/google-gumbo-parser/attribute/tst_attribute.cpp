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

#include "attribute.h"
#include "gumbo.h"
#include "parser.h"
#include "vector.h"

#include <QtCore/QDebug>
#include <QtTest/QtTest>

class tst_Attribute : public GumboTest
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void AddAttribute();

private:
    GumboVector vector_;
};

/******************************************************************************
 ******************************************************************************/
void tst_Attribute::init()
{
    gumbo_vector_init(&parser_, 2, &vector_);
}

void tst_Attribute::cleanup()
{
    gumbo_vector_destroy(&parser_, &vector_);
}

/******************************************************************************
 ******************************************************************************/
void tst_Attribute::AddAttribute()
{
    GumboAttribute attr1;
    GumboAttribute attr2;
    attr1.name = "";
    attr2.name = "foo";

    gumbo_vector_add(&parser_, &attr1, &vector_);
    gumbo_vector_add(&parser_, &attr2, &vector_);

    QCOMPARE(&attr2, gumbo_get_attribute(&vector_, "foo"));
    QVERIFY(NULL == gumbo_get_attribute(&vector_, "bar"));
}

/******************************************************************************
 ******************************************************************************/

QTEST_APPLESS_MAIN(tst_Attribute)

#include "tst_attribute.moc"
