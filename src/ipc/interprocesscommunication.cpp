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

#include "interprocesscommunication.h"
#include "constants.h"

#include <Core/Model>
#include <Core/ResourceItem>
#include <Core/ResourceModel>

#include <QtCore/QDebug>
#include <QtCore/QSharedMemory>
#include <QtCore/QString>


QString InterProcessCommunication::readMessageFromLauncher()
{
    QString message;
    message += C_PACKET_BEGIN;
    message += QChar::Space;

    QSharedMemory sharedMemory;
    sharedMemory.setKey(C_SHARED_MEMORY_KEY);

    if (sharedMemory.attach(QSharedMemory::ReadWrite)) {

        // Read message from Launcher
        message += shm_read(&sharedMemory);
        message += QChar::Space;

        // Write Acknowledge to Launcher
        shm_write(&sharedMemory, C_SHARED_MEMORY_ACK_REPLY);

        sharedMemory.detach();

    } else {
        /*
         * Unable to attach to the shared memory segment.
         *
         * Use sharedMemory.error() and sharedMemory.errorString()
         * to investigate the error.
         */
        message = C_PACKET_ERROR;
        message += QChar::Space;
    }
    message += C_PACKET_END;
    message += QChar::Space;

    return message;
}


QString InterProcessCommunication::clean(const QString &message)
{
    QString cleaned = message;
    cleaned.replace('\"', "", Qt::CaseInsensitive);
    return cleaned.trimmed();
}

bool InterProcessCommunication::isSingleUrl(const QString &message)
{
    return !message.trimmed().contains(QChar::Space, Qt::CaseInsensitive);
}

bool InterProcessCommunication::isCommandOpenManager(const QString &message)
{
    return message.contains(C_KEYWORD_MANAGER, Qt::CaseInsensitive);
}

bool InterProcessCommunication::isCommandShowPreferences(const QString &message)
{
    return message.contains(C_KEYWORD_PREFS, Qt::CaseInsensitive);
}

bool InterProcessCommunication::isCommandOpenUrl(const QString &message)
{
    return message.contains(C_KEYWORD_OPEN_URL, Qt::CaseInsensitive);
}

QString InterProcessCommunication::getCurrentUrl(const QString &message)
{
    auto resources = message.split(QChar::Space, Qt::SkipEmptyParts);
    for (auto i = 0; i < resources.count() - 1; ++i) {
        if (resources.at(i).trimmed() == C_KEYWORD_OPEN_URL) {
            return resources.at(i+1).trimmed();
        }
    }
    return {};
}

bool InterProcessCommunication::isCommandDownloadLink(const QString &message)
{
    return message.contains(C_KEYWORD_DOWNLOAD_LINK, Qt::CaseInsensitive);
}

QString InterProcessCommunication::getDownloadLink(const QString &message)
{
    const QStringList resources = message.split(QChar::Space, Qt::SkipEmptyParts);
    for (int i = 0; i < resources.count() - 1; ++i) {
        if (resources.at(i).trimmed() == C_KEYWORD_DOWNLOAD_LINK) {
            return resources.at(i+1).trimmed();
        }
    }
    return {};
}


void InterProcessCommunication::parseMessage(const QString &message, Model *model,
                                        InterProcessCommunication::Options *options)
{
    if (model == nullptr) {
        return;
    }
    if (message.contains(C_PACKET_ERROR, Qt::CaseInsensitive)) {
        return;
    }

    Mode mode = None;

    auto resources = message.split(QChar::Space, Qt::SkipEmptyParts);

    for (auto resource : resources) {
        auto trimmed = resource.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }

        if (trimmed == C_KEYWORD_CURRENT_URL) {
            mode = None;

        } else if (trimmed == C_KEYWORD_LINKS) {
            mode = Link;

        } else if (trimmed == C_KEYWORD_MEDIA) {
            mode = Media;

        } else if (trimmed.contains('[')) {
            // C_PACKET_BEGIN
            // C_PACKET_END
            // C_KEYWORD_OPEN_URL
            // ...
            mode = Other;

            if (options) {
                if (trimmed == C_KEYWORD_QUICK_LINKS) {
                    *options |= QuickLinks;
                } else if (trimmed == C_KEYWORD_QUICK_MEDIA) {
                    *options |= QuickMedia;
                } else if (trimmed == C_KEYWORD_STARTED_PAUSED) {
                    *options |= StartPaused;
                }
            }

        } else {
            if (mode == Link) {
                auto item = new ResourceItem();
                item->setUrl(resource);
                model->linkModel()->add(item);

            } else if (mode == Media) {
                auto item = new ResourceItem();
                item->setUrl(resource);
                model->contentModel()->add(item);
            }
        }
    }
}
