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

#include "regex.h"

#include <QtCore/QRegExp>
#include <QtCore/QDebug>


static QString pattern            = R"([\[\(]\d+[:-\s]\d+[\]\)])";
static QString patternWithGroups  = R"([\[\(](\d+)[:-\s](\d+)[\]\)])";

//static QString pattern            = "[\\[\\(]\\d+[:-\\s]\\d+[\\]\\)]";
//static QString patternWithGroups  = "[\\[\\(](\\d+)[:-\\s](\\d+)[\\]\\)]";
static int firstGroupPosition     = 1; // 0 is reseved to full string
static int secondGroupPosition    = 2;


struct Capture
{
    QString capture;
    int pos{};
    int len{};
    QStringList interpretedCapture;
};

static const QList<Capture> capture(const QString &str)
{
    QList<Capture> captures;
    const QRegExp rx(pattern);
    int pos = 0;
    while ((pos = rx.indexIn(str, pos)) != -1) {
        Capture cap;
        cap.capture = rx.cap(0);
        cap.pos = pos;
        cap.len = cap.capture.length();

        captures << cap;
        pos += rx.matchedLength();
    }
    return captures;
}

bool Regex::hasBatchDescriptors(const QString &str)
{
    const QList<Capture> captures = capture(str);
    return !captures.isEmpty();
}

QStringList Regex::interpret(const QUrl &url)
{
    return interpret(url.toString());
}

QStringList Regex::interpret(const QString &str)
{
    /*
     * First, we detect and capture each batch.
     * Indeed, the incoming string can contains more than 1 batch:
     * Ex:
     * "https://www.mysite.com/pic/[2015:2019]/[01:12]/DSC_[001:999].jpg"
     *                              ^^^^^^^^^   ^^^^^       ^^^^^^^
     *                               batch 1    batch2      batch 3
     */
    QList<Capture> captures = capture(str);

    /* If no batch, return the incoming string. */
    if (captures.isEmpty()) {
        QStringList list;
        list << str;
        return list;
    }

    /*
     * For each batch, we interpret the batch:
     * Ex:
     * "[2015:2019]" is interpreted {2015 2016 2017 2018 2019}
     * "[001:003]" is interpreted {001 002 003}
     * "[8:11]" is interpreted {8 9 10 11}
     *
     */
    for (int index = 0; index < captures.count(); ++index) {
        Capture &cap = captures[index];

        const QRegExp rx(patternWithGroups);
        int start = rx.indexIn(cap.capture, 0);
        if (start != 0) {
            Q_ASSERT(false);
        } else {

            int fieldWidth = rx.cap(firstGroupPosition).length();
            int begin = rx.cap(firstGroupPosition).toInt();
            int end = rx.cap(secondGroupPosition).toInt();

            if (begin > end) {
                int t = end;
                end = begin;
                begin = t;
                fieldWidth = rx.cap(secondGroupPosition).length();
            }

            for (int i = begin; i <= end; ++i) {
                cap.interpretedCapture.append(QString("%0").arg(i, fieldWidth, 10, QChar('0')));
            }
        }
    }

    /*
     * Finally, we join each interpreted capture recursively.
     */
    QStringList list;
    list << str;
    for (int i = captures.count() - 1; i >= 0; --i) {
        auto capture = captures.at(i);
        QStringList buffer;
        foreach (auto interpretedCapture, capture.interpretedCapture) {
            foreach (auto interpreted, list) {
                interpreted.replace(capture.pos, capture.len, interpretedCapture);
                buffer << interpreted;
            }
        }
        list.clear();
        list.append(buffer);
    }
    return list;
}
