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

#include <Core/AbstractSettings>

#include <QtTest/QtTest>
#include <QtCore/QDebug>

#define FORBIDDEN_VALUE     QLatin1String("<UNDEFINED>")
#define DUMMY_KEY           QLatin1String("MyBooleanSetting")


class tst_AbstractSettings : public QObject
{
    Q_OBJECT

private slots:
    void addDefaultSettingWithIllegalValue();
    void setSettingWithIllegalValue();
    void setSettingWithBoolean();
};

class FriendlyAbstractSettings : public AbstractSettings { friend class tst_AbstractSettings; };

/******************************************************************************
 ******************************************************************************/
void tst_AbstractSettings::addDefaultSettingWithIllegalValue()
{
    FriendlyAbstractSettings target;
    QVERIFY_EXCEPTION_THROWN(target.addDefaultSetting(DUMMY_KEY, FORBIDDEN_VALUE), AbstractSettings::IllegalValueException);
}

/******************************************************************************
 ******************************************************************************/
void tst_AbstractSettings::setSettingWithIllegalValue()
{
    FriendlyAbstractSettings target;
    target.addDefaultSetting(DUMMY_KEY, false);

    QVERIFY_EXCEPTION_THROWN(target.setSetting(DUMMY_KEY, FORBIDDEN_VALUE), AbstractSettings::IllegalValueException);
}

/******************************************************************************
 ******************************************************************************/
void tst_AbstractSettings::setSettingWithBoolean()
{
    FriendlyAbstractSettings target;
    target.addDefaultSetting(DUMMY_KEY, false);
    target.setSetting(DUMMY_KEY, false);

    QVERIFY_EXCEPTION_THROWN(target.getSetting(DUMMY_KEY), AbstractSettings::MissingKeyException);

    const bool actual = target.getSettingBool(DUMMY_KEY);
    QCOMPARE(actual, false);
}

/******************************************************************************
 ******************************************************************************/

QTEST_APPLESS_MAIN(tst_AbstractSettings)

#include "tst_abstractsettings.moc"
