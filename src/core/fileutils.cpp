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

#include "fileutils.h"

#include <Constants>

#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>


const char *S_FORBIDDEN_SUB_STRINGS[] = {".."};
const int S_FORBIDDEN_SUB_STRINGS_COUNT = sizeof(S_FORBIDDEN_SUB_STRINGS)/sizeof(const char *);


/******************************************************************************
 ******************************************************************************/
/*!
 * Validate a file base name.
 */
QString FileUtils::validateFileName(const QString &name, bool allowSubDir)
{
    QString fixedName;

    const char *forbiddenChars = allowSubDir
            ? S_FORBIDDEN_CHARS_SUB_DIR
            : S_FORBIDDEN_CHARS_NO_SUB_DIR;

    // Characters
    for (auto ch : name) {
        if (ch < QLatin1Char(32)) {
            // Non-printable characters
            // ASCII control characters not allowed on Windows
            fixedName += S_SUBSTITUTE_CHAR;

        } else if (ch < QLatin1Char(127)) {
            // Printable ASCII characters
            bool passed = true;
            for (const char *c = forbiddenChars; *c; ++c) {
                if (ch == QLatin1Char(*c)) {
                    fixedName += S_SUBSTITUTE_CHAR;
                    passed = false;
                    break;
                }
            }
            if (passed) {
                fixedName += ch;
            }

        } else if (ch == QLatin1Char(127)) {
            // DEL char
            fixedName += S_SUBSTITUTE_CHAR;
        } else {
            // allow all other chars in filename
            fixedName += ch;
        }
    }

    // Substrings
    for (auto s = 0; s < S_FORBIDDEN_SUB_STRINGS_COUNT; ++s) {
        const QLatin1StringView notAllowedSubString(S_FORBIDDEN_SUB_STRINGS[s]);
        fixedName = fixedName.replace(notAllowedSubString, S_SUBSTITUTE_CHAR);
    }

    // Windows devices
    static QRegularExpression regex(
                QString("^")
                // *********************
                // captured group #0
                + (allowSubDir
                   ? QString("(?<subdir>(.*[") + QRegularExpression::escape("/\\") + "])?)"
                   : QString())
                // *********************
                // captured group #1
                // rem: [^A] means anything not A
                + "(?<filename>" + WINDOWS_RESERVED_DEVICE_NAMES + ")"
                // *********************
                // captured group #2
                + "(?<extension>(..*)?)"
                // *********************
                + "$",
                QRegularExpression::CaseInsensitiveOption
                );
    Q_ASSERT(regex.isValid());
    QRegularExpressionMatch match = regex.match(fixedName);
    if (match.hasMatch()) {
        QString subdir = allowSubDir ? match.captured("subdir") : QString();
        QString extension = match.captured("extension");
        fixedName = subdir + S_SUBSTITUTE_FILE_NAME + extension;
    }

    // Windows end char rule
    if ( fixedName.endsWith(QLatin1Char(' ')) ||
         fixedName.endsWith(QLatin1Char('.'))) {
        fixedName += S_SUBSTITUTE_CHAR;
    }

    return fixedName;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Replace "RUFUS THOMAS, THE FUNKY CHICKEN-LIVE"
 *  to title-capitalized "Rufus Thomas, The Funky Chicken-Live"
 */
static QString capitalize(const QString &sentence)
{
    auto capitalized = sentence.toLower();
    for (auto sep: {" ", ",", "-", "_", "\"", "'", "@"}) {
        auto words = capitalized.split(sep, Qt::KeepEmptyParts);
        for (int i = 0; i < words.size(); ++i) {
            if (words[i].count() > 0) {
                words[i].replace(0, 1, words[i][0].toUpper());
            }
        }
        capitalized = words.join(sep);
    }
    return capitalized;
}

QString FileUtils::cleanFileName(const QString &fileName)
{
    auto ret = fileName.simplified();
    ret = ret.remove(QRegularExpression("official music video", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("official video", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("official visualizer", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("official audio", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("video", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("audio", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("radio edit", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("\\(+\\)+", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("\\[+\\]+", QRegularExpression::CaseInsensitiveOption));
    ret = capitalize(ret);
    ret = ret.simplified();

    QString::iterator it;
    for (it = ret.begin(); it != ret.end(); ++it){
        const QChar c = (*it).unicode();
        if (c.isLetterOrNumber() || C_LEGAL_CHARS.contains(c)) {
            continue;
        }
        if (c == QChar('"')) {
            *it = QChar('\'');
        } else {
            *it = QChar('-');
        }
    }
    ret = ret.replace(QRegularExpression("-+"), QLatin1String("-"));
    return ret.simplified();
}
