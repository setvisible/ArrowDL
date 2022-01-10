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

#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>

// ********************************************************
// Inpired by: QtCreator source code
// Utils::FileNameValidatingLineEdit::validateFileName
// ********************************************************
/*
 * Naming a file like a device name will break on Windows,
 * even if it is "com1.txt".
 * Since we are cross-platform, we generally disallow such file names.
 */
#define WINDOWS_RESERVED_DEVICE_NAMES "CON|PRN|AUX|NUL" \
    "|COM1|COM2|COM3|COM4|COM5|COM6|COM7|COM8|COM9" \
    "|LPT1|LPT2|LPT3|LPT4|LPT5|LPT6|LPT7|LPT8|LPT9"

/*
 * Validate a file base name, check for forbidden characters/strings.
 */
#define PRINTABLE_ASCII_CHARS "<>:\"|?*"
#define SLASHES "/\\"

static const char s_forbidden_chars_sub_dir[] = PRINTABLE_ASCII_CHARS;
static const char s_forbidden_chars_no_sub_dir[] = PRINTABLE_ASCII_CHARS SLASHES;

static const char *s_forbidden_sub_strings[] = {".."};
static const int s_forbidden_sub_strings_count = sizeof(s_forbidden_sub_strings)/sizeof(const char *);

static const QString s_substitute_char('_');
static const QString s_substitute_file_name("file");

/*
 * This list of legal characters for filenames is limited to avoid injections
 * of special or invisible characters that could be not supported by the OS.
 */
static const QString C_LEGAL_CHARS = QLatin1String("-+' @()[]{}°#,.&");


/******************************************************************************
 ******************************************************************************/
/*!
 * Validate a file base name.
 */
QString FileUtils::validateFileName(const QString &name, bool allowSubDir)
{
    QString fixedName;

    const char *forbiddenChars = allowSubDir
            ? s_forbidden_chars_sub_dir
            : s_forbidden_chars_no_sub_dir;

    // Characters
    for (auto ch : name) {
        if (ch < 32) {
            // Non-printable characters
            // ASCII control characters not allowed on Windows
            fixedName += s_substitute_char;

        } else if (ch < 127) {
            // Printable ASCII characters
            bool passed = true;
            for (const char *c = forbiddenChars; *c; ++c) {
                if (ch == QLatin1Char(*c)) {
                    fixedName += s_substitute_char;
                    passed = false;
                    break;
                }
            }
            if (passed) {
                fixedName += ch;
            }

        } else if (ch == 127) {
            // DEL char
            fixedName += s_substitute_char;
        } else {
            // allow all other chars in filename
            fixedName += ch;
        }
    }

    // Substrings
    for (int s = 0; s < s_forbidden_sub_strings_count; ++s) {
        const QLatin1String notAllowedSubString(s_forbidden_sub_strings[s]);
        fixedName = fixedName.replace(notAllowedSubString, s_substitute_char);
    }

    // Windows devices
    QRegularExpression regex(
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
        fixedName = subdir + s_substitute_file_name + extension;
    }

    // Windows end char rule
    if ( fixedName.endsWith(QLatin1Char(' ')) ||
         fixedName.endsWith(QLatin1Char('.'))) {
        fixedName += s_substitute_char;
    }

    return fixedName;
}

/******************************************************************************
 ******************************************************************************/
QString FileUtils::cleanFileName(const QString &fileName)
{
    QString ret = fileName.simplified();
    ret = ret.remove(QRegularExpression("official music video", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("official video", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("official audio", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("video", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("audio", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("\\(\\)", QRegularExpression::CaseInsensitiveOption));
    ret = ret.remove(QRegularExpression("\\[\\]", QRegularExpression::CaseInsensitiveOption));
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
