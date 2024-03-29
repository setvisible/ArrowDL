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

#include "locale.h"

#include <QtGlobal>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QTranslator>
#include <QtCore/QDir>


static QTranslator *s_translator = nullptr;
static QList<QLocale> s_locales;

static QString languageDirectory()
{
    return QString("%0/locale").arg(qApp->applicationDirPath());
}

static QString translationFileName(const QString &localeName)
{
    return QString("%0/arrowdl_%1.qm").arg(languageDirectory(), localeName);
}

static void populateLocales()
{
    // List all *.qm
    const QDir dir(languageDirectory());
    auto allDotQMs = dir.entryList({ QLatin1String("*.qm") }, QDir::Files, QDir::Name);

    // Get locale from the ISO 639 language code
    QList<QLocale> locales;
    for (const auto &dotQM : allDotQMs) {
        auto code = dotQM;
        code.replace("arrowdl_", "");
        code.replace(".qm", "");
        locales << QLocale(code); // Ex: "en_US"
    }
    s_locales = locales;
}

/******************************************************************************
 ******************************************************************************/
QStringList Locale::availableLanguages()
{
    if (s_locales.isEmpty()) {
        populateLocales();
    }
    QStringList languageNames;
    for (const auto &locale : s_locales) {
        languageNames << locale.nativeLanguageName();
    }
    return languageNames;
}

QString Locale::toLanguage(qsizetype index)
{
    if (index >= 0 && index < s_locales.count()) {
        auto locale = s_locales.at(index);
        return locale.name();
    }
    return QLatin1String(""); // Must be an empty string, not a null QString
}

qsizetype Locale::fromLanguage(QString language)
{
    if (language.isEmpty()) {
        language = QLocale::system().name();
    }
    for (auto index = 0; index < s_locales.count(); ++index) {
        auto locale = s_locales.at(index);
        auto localeLanguage = locale.name();
        if (localeLanguage.compare(language, Qt::CaseInsensitive) == 0) {
            return index;
        }
    }
    return 0;
}

/******************************************************************************
 ******************************************************************************/
void Locale::applyLanguage(const QString &language)
{
    if (s_translator) {
        qApp->removeTranslator(s_translator);
        delete s_translator;
        s_translator = nullptr;
    }
    auto locale = !language.isEmpty() ? QLocale(language) : QLocale::system();
    auto localeName = locale.name();
    auto localeFilename = translationFileName(localeName);
    auto localeInfo = QObject::tr("translation '%0', locale '%1': %2")
            .arg(language, localeName, localeFilename);

    s_translator = new QTranslator();
    if (s_translator->load(localeFilename)) {
        qApp->installTranslator(s_translator);
        // qInfo() << QObject::tr("Loaded %0").arg(localeInfo);
    } else {
        delete s_translator;
        s_translator = nullptr;
        qWarning() << QObject::tr("Can't load %0").arg(localeInfo);
    }
}
