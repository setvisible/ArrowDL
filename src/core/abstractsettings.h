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

#ifndef CORE_ABSTRACT_SETTINGS_H
#define CORE_ABSTRACT_SETTINGS_H

#include <QtCore/QException>
#include <QtCore/QObject>
#include <QtCore/QString>

/*!
 * \class AbstractSettings
 * \brief The AbstractSettings class manages the persistence of the User preferences
 *
 */
class AbstractSettings : public QObject
{
    Q_OBJECT
    enum KeyType { BOOL, INTEGER, STRING };
    struct SettingsItem;

public:
    explicit AbstractSettings(QObject *parent = nullptr);
    ~AbstractSettings() override;

    void beginRestoreDefault();
    void endRestoreDefault();

    void readSettings();
    void writeSettings();

    class IllegalKeyException : public QException {
    public:
        void raise() const override { throw *this; }
        IllegalKeyException *clone() const override { return new IllegalKeyException(*this); }
    };

    class IllegalValueException : public QException {
    public:
        void raise() const override { throw *this; }
        IllegalValueException *clone() const override { return new IllegalValueException(*this); }
    };

    class MissingKeyException : public QException {
    public:
        void raise() const override { throw *this; }
        MissingKeyException *clone() const override { return new MissingKeyException(*this); }
    };

    class WrongTypeException : public QException {
    public:
        void raise() const override { throw *this; }
        WrongTypeException *clone() const override { return new WrongTypeException(*this); }
    };

signals:
    void changed();

protected:
    void addDefaultSettingBool(const QString &key, bool defaultValue);
    void setSettingBool(const QString &key, bool value);
    bool getSettingBool(const QString &key) const;

    void addDefaultSettingInt(const QString &key, int defaultValue);
    void setSettingInt(const QString &key, int value);
    int getSettingInt(const QString &key) const;

    void addDefaultSettingString(const QString &key, const QString &defaultValue);
    void setSettingString(const QString &key, const QString &value);
    QString getSettingString(const QString &key) const;

    void addDefaultSettingStringList(const QString &key, const QStringList &defaultValue);
    void setSettingStringList(const QString &key, const QStringList &value);
    QStringList getSettingStringList(const QString &key) const;

    QString serialize(const QMap<QString, QVariant> &map) const;
    QMap<QString, QVariant> deserialize(const QString &str) const;

private:
    QList<SettingsItem*> m_items = {};
    bool m_default = false;

    void _q_addDefaultSetting(const QString &key, const QString &defaultValue, KeyType keyType);
    void _q_setSetting(const QString &key, const QString &defaultValue, KeyType keyType);
    QString _q_getSetting(const QString &key, KeyType keyType) const;

    QString uniqueRegisterKey(const AbstractSettings::SettingsItem *item) const;
};

#endif // CORE_ABSTRACT_SETTINGS_H
