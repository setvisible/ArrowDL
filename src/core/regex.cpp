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

#include "regex.h"

#include <QtCore/QRegExp>
#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

struct Capture
{
    QString capture;
    int pos;
    int len;
    QStringList interpretedCapture;
};

const QStringList Regex::interpret(const QString &str)
{
    QStringList list;
    QList<Capture> captures;

    {
        const QRegExp rx("(\\[\\d+[:-]\\d+\\])");

        int pos = 0;
        while ((pos = rx.indexIn(str, pos)) != -1) {
            Capture cap;
            cap.capture = rx.cap(1);
            cap.pos = pos;
            cap.len = cap.capture.length();

            captures << cap;
            pos += rx.matchedLength();
        }
    }

    for (int index = 0; index < captures.count(); ++index) {
        Capture &cap  = captures[index];

        const QRegExp rx("\\[(\\d+)[:-](\\d+)\\]");
        int start = rx.indexIn(cap.capture, 0);
        if (start != 0) {
            Q_ASSERT(false);
        } else {

            int fieldWidth = rx.cap(1).length();
            int begin = rx.cap(1).toInt();
            int end = rx.cap(2).toInt();

            if (begin > end) {
                int t = end;
                end = begin;
                begin = t;
            }

            for (int i = begin; i <= end; ++i) {
                cap.interpretedCapture.append(QString("%0").arg(i, fieldWidth, 10, QChar('0')));
            }
        }
    }

    if (captures.isEmpty()) {
        list << str;
    } else {
        foreach (auto cap, captures) {
            foreach (auto interpretedCapture, cap.interpretedCapture) {
                QString interpreted = str;
                interpreted.replace(cap.pos, cap.len, interpretedCapture);
                list << interpreted;
            }
        }
    }
    return list;
}
