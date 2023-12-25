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

#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>

constexpr int firstGroupPosition     = 1; // 0 is reseved to full string
constexpr int secondGroupPosition    = 2;


struct Capture
{
    QString capture;
    int pos{0};
    int len{0};
    QStringList interpretedCapture;
};

static QList<Capture> capture(const QString &str)
{
    QList<Capture> captures;
    static QRegularExpression reBatch("([\\[\\(]\\d+[:\\-\\s]\\d+[\\]\\)])");
    QRegularExpressionMatchIterator i = reBatch.globalMatch(str);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        Capture cap;
        cap.capture = match.captured(0);
        cap.pos = match.capturedStart();
        cap.len = cap.capture.length();
        captures << cap;
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
        static QRegularExpression reGroup("[\\[\\(](\\d+)[:\\-\\s](\\d+)[\\]\\)]");
        QRegularExpressionMatch match = reGroup.match(cap.capture);
        if (!match.hasMatch()) {
            Q_ASSERT(false);
        } else {

            auto strBegin = match.captured(firstGroupPosition);
            auto strEnd = match.captured(secondGroupPosition);

            int fieldWidth = strBegin.length();
            int begin = strBegin.toInt();
            int end = strEnd.toInt();

            if (begin > end) {
                int t = end;
                end = begin;
                begin = t;
                fieldWidth = match.captured(secondGroupPosition).length();
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

QStringList Regex::getCaptures(const QString &str)
{
    QList<Capture> captures = capture(str);
    QStringList ret;
    foreach (auto capture, captures) {
        ret.append(capture.capture);
    }
    return ret;
}
