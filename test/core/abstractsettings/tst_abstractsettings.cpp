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

#include <Core/AbstractSettings>

#include <QtTest/QtTest>
#include <QtCore/QDebug>

/* Protected Keywords */
#define UNDEF    QLatin1String("<UNDEFINED>")


class tst_AbstractSettings : public QObject
{
    Q_OBJECT

private slots:
    void addSettingWithString_data();
    void addSettingWithString();

    void addDefaultSettingWithIllegalKey();
    void setSettingMissingDefaultValue();
    void addDefaultSettingWithIllegalValue();
    void setSettingStringWithIllegalValue();
    void getSettingStringWithIllegalValue();

    void setSettingWithWrongType();
    void getSettingWithWrongType();
};

class FriendlyAbstractSettings : public AbstractSettings
{
    friend class tst_AbstractSettings;
};

/******************************************************************************
 ******************************************************************************/
void tst_AbstractSettings::addSettingWithString_data()
{
    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("defaultValue");
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("trivial") << "Color" << "red" << "blue" << "blue";
    QTest::newRow("key -") << "-" << "red" << "" << "";
    QTest::newRow("key .") << "..." << "red" << "10" << "10";
    QTest::newRow("simple") << "integer" << "red" << "10" << "10";

    QTest::newRow("empty") << "Color" << "red" << "" << "";

}

void tst_AbstractSettings::addSettingWithString()
{
    QFETCH(QString, key);
    QFETCH(QString, defaultValue);
    QFETCH(QString, input);
    QFETCH(QString, expected);

    FriendlyAbstractSettings target;
    target.addDefaultSettingString(key, defaultValue);
    target.setSettingString(key, input);

    QString actual = target.getSettingString(key);
    QCOMPARE(actual, expected);
}

/******************************************************************************
 ******************************************************************************/
void tst_AbstractSettings::addDefaultSettingWithIllegalKey()
{
    FriendlyAbstractSettings target;
    QVERIFY_EXCEPTION_THROWN( target.addDefaultSettingString(QString(), "Hello"), AbstractSettings::IllegalKeyException);
    QVERIFY_EXCEPTION_THROWN( target.addDefaultSettingString("", "Hello"), AbstractSettings::IllegalKeyException);
    QVERIFY_EXCEPTION_THROWN( target.addDefaultSettingString(UNDEF, "Hello"), AbstractSettings::IllegalKeyException);
}

void tst_AbstractSettings::setSettingMissingDefaultValue()
{
    FriendlyAbstractSettings target;
    /* Must call addDefaultSetting() before */
    QVERIFY_EXCEPTION_THROWN( target.setSettingString("Color", "yellow"), AbstractSettings::MissingKeyException);
}

void tst_AbstractSettings::addDefaultSettingWithIllegalValue()
{
    FriendlyAbstractSettings target;
    QVERIFY_EXCEPTION_THROWN( target.addDefaultSettingString("Color", QString()), AbstractSettings::IllegalValueException);
    QVERIFY_EXCEPTION_THROWN( target.addDefaultSettingString("Color", UNDEF), AbstractSettings::IllegalValueException);
}

void tst_AbstractSettings::setSettingStringWithIllegalValue()
{
    FriendlyAbstractSettings target;
    target.addDefaultSettingString("Color", "red");
    QVERIFY_EXCEPTION_THROWN( target.setSettingString("Color", QString()), AbstractSettings::IllegalValueException);
    QVERIFY_EXCEPTION_THROWN( target.setSettingString("Color", UNDEF), AbstractSettings::IllegalValueException);
}

void tst_AbstractSettings::getSettingStringWithIllegalValue()
{
    FriendlyAbstractSettings target;
    target.addDefaultSettingString("Color", "red");

    QVERIFY_EXCEPTION_THROWN( target.getSettingString(QString()), AbstractSettings::IllegalKeyException);
    QVERIFY_EXCEPTION_THROWN( target.getSettingString(""), AbstractSettings::IllegalKeyException);
    QVERIFY_EXCEPTION_THROWN( target.getSettingString(UNDEF), AbstractSettings::IllegalKeyException);
}

/******************************************************************************
 ******************************************************************************/
void tst_AbstractSettings::setSettingWithWrongType()
{
    FriendlyAbstractSettings target;

    // "Color" is a QString here...
    target.addDefaultSettingString("Color", "red");

    // ...but other types expected there
    QVERIFY_EXCEPTION_THROWN( target.setSettingBool("Color", true), AbstractSettings::WrongTypeException);
    QVERIFY_EXCEPTION_THROWN( target.setSettingInt("Color", 123), AbstractSettings::WrongTypeException);

    /* Other types */
    target.addDefaultSettingBool("IsEnabled", true);
    QVERIFY_EXCEPTION_THROWN( target.setSettingString("IsEnabled", "red"), AbstractSettings::WrongTypeException);
    QVERIFY_EXCEPTION_THROWN( target.setSettingInt("IsEnabled", 123), AbstractSettings::WrongTypeException);

    target.addDefaultSettingInt("Value", 99);
    QVERIFY_EXCEPTION_THROWN( target.setSettingBool("Value", true), AbstractSettings::WrongTypeException);
    QVERIFY_EXCEPTION_THROWN( target.setSettingString("Value", "red"), AbstractSettings::WrongTypeException);
}

void tst_AbstractSettings::getSettingWithWrongType()
{
    FriendlyAbstractSettings target;

    // "Color" is a QString here...
    target.addDefaultSettingString("Color", "red");

    // ...but other types expected there
    QVERIFY_EXCEPTION_THROWN( target.getSettingBool("Color"), AbstractSettings::WrongTypeException);
    QVERIFY_EXCEPTION_THROWN( target.getSettingInt("Color"), AbstractSettings::WrongTypeException);

    /* Other types */
    target.addDefaultSettingBool("IsEnabled", true);
    QVERIFY_EXCEPTION_THROWN( target.getSettingInt("IsEnabled"), AbstractSettings::WrongTypeException);
    QVERIFY_EXCEPTION_THROWN( target.getSettingString("IsEnabled"), AbstractSettings::WrongTypeException);

    target.addDefaultSettingInt("Value", 99);
    QVERIFY_EXCEPTION_THROWN( target.getSettingBool("Value"), AbstractSettings::WrongTypeException);
    QVERIFY_EXCEPTION_THROWN( target.getSettingString("Value"), AbstractSettings::WrongTypeException);
}

/******************************************************************************
 ******************************************************************************/

QTEST_APPLESS_MAIN(tst_AbstractSettings)

#include "tst_abstractsettings.moc"
