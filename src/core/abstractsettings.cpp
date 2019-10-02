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

#include "abstractsettings.h"

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QDebug>

/*
 * Remark:
 * Characters '<' and '>' are unlikely to be used as value for data or directory path.
 * If a collision appears, the only risk is to reset the faulty parameter
 * to its default value.
 */
static const QLatin1String UNDEFINED ("<UNDEFINED>");
static const QLatin1String VALUE_TRUE ("<TRUE>");
static const QLatin1String VALUE_FALSE ("<FALSE>");

/*
 * Helper methods
 */
static const QString toString(bool b) { return b ? VALUE_TRUE : VALUE_FALSE; }
static bool toBool(const QString &str) { return str == VALUE_TRUE ? true : false; }


struct AbstractSettings::SettingsItem
{
    AbstractSettings::KeyType keyType;
    QString key;
    QString value;
    QString defaultValue;
};

AbstractSettings::AbstractSettings(QObject *parent) : QObject(parent)
  , m_default(false)
{
}

AbstractSettings::~AbstractSettings()
{
    qDeleteAll(m_items);
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Restore the default settings
 */
void AbstractSettings::beginRestoreDefault()
{
    m_default = true;
}

void AbstractSettings::endRestoreDefault()
{
    m_default = false;
}

void AbstractSettings::readSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("Preference"));
    foreach (auto item, m_items) {
        const QString value = settings.value(_q_unique_name(item), UNDEFINED).toString();
        item->value = (value != UNDEFINED) ? value : item->defaultValue;
    }
    settings.endGroup();
    emit changed();
}

void AbstractSettings::writeSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("Preference"));
    foreach (auto item, m_items) {
        const QString name = _q_unique_name(item);
        if (item->value != item->defaultValue || settings.contains(name)) {
            settings.setValue(name, item->value);
        }
    }
    settings.endGroup();
}

/******************************************************************************
 ******************************************************************************/
void AbstractSettings::addDefaultSetting(const QString &key, bool defaultValue)
{
    _q_addDefaultSetting(key, toString(defaultValue), KeyType::BOOL);
}

bool AbstractSettings::getSettingBool(const QString &key) const
{
    return toBool(_q_getSetting(key, KeyType::BOOL));
}

void AbstractSettings::setSetting(const QString &key, bool value)
{
    _q_setSetting(key, toString(value), KeyType::BOOL);
}


/******************************************************************************
 ******************************************************************************/
void AbstractSettings::addDefaultSetting(const QString &key, const QString &defaultValue)
{
    _q_addDefaultSetting(key, defaultValue, KeyType::STRING);
}

QString AbstractSettings::getSetting(const QString &key) const
{
    return _q_getSetting(key, KeyType::STRING);
}


void AbstractSettings::setSetting(const QString &key, const QString &value)
{
    _q_setSetting(key, value, KeyType::STRING);
}

/******************************************************************************
 ******************************************************************************/
QStringList AbstractSettings::getListSetting(const QString &key) const
{
    QStringList ret;
    for (int i = 0; i < m_items.count(); ++i) {
        const QString subkey = QString("%0%1").arg(key).arg(i);
        foreach (auto item, m_items) {
            if (item->key == subkey) {
                ret << (m_default ? item->defaultValue : item->value);
            }
        }
    }
    return ret;
}

void AbstractSettings::addDefaultListSetting(const QString &key, const QStringList &defaultValue)
{
    for (int i = 0; i < defaultValue.count(); ++i) {
        const QString subkey = QString("%0%1").arg(key).arg(i);
        const QString subvalue = defaultValue.at(i);
        addDefaultSetting(subkey, subvalue);
    }
}

void AbstractSettings::setListSetting(const QString &key, const QStringList &value)
{
    for (int i = 0; i < value.count(); ++i) {
        const QString subkey = QString("%0%1").arg(key).arg(i);
        const QString subvalue = value.at(i);
        setSetting(subkey, subvalue);
    }
}

/******************************************************************************
 ******************************************************************************/
void AbstractSettings::_q_addDefaultSetting(const QString &key,
                                            const QString &defaultValue,
                                            KeyType keyType)
{
    if (defaultValue == UNDEFINED) {
        throw IllegalValueException();
    }
    foreach (auto item, m_items) {
        if (item->keyType == keyType && item->key == key) {
            item->defaultValue = defaultValue;
            return;
        }
    }
    SettingsItem *item = new SettingsItem();
    item->key = key;
    item->keyType = keyType;
    item->defaultValue = defaultValue;
    m_items.append(item);
}

QString AbstractSettings::_q_getSetting(const QString &key, KeyType keyType) const
{
    foreach (auto item, m_items) {
        if (item->keyType == keyType && item->key == key) {
            return m_default ? item->defaultValue : item->value;
        }
    }
    throw MissingKeyException();
}

void AbstractSettings::_q_setSetting(const QString &key,
                                     const QString &value,
                                     KeyType keyType)
{
    if (value == UNDEFINED) {
        throw IllegalValueException();
    }
    foreach (auto item, m_items) {
        if (item->keyType == keyType && item->key == key) {
            if (item->value != value) {
                item->value = value;
                emit changed();
            }
            return;
        }
    }
    throw MissingKeyException();
}

QString AbstractSettings::_q_unique_name(const SettingsItem *item) const
{
    Q_ASSERT(item != Q_NULLPTR);
    return QString("%0%1")
            .arg(item->key)
            .arg(item->keyType == KeyType::BOOL ? QLatin1String("_bool") : QString());
}

