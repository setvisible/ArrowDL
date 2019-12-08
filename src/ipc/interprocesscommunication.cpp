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

        QByteArray bytes;

        if (sharedMemory.lock()) {
            // Reads the shared memory
            const char* ptr = static_cast<const char*>(sharedMemory.constData());
            uint n = static_cast<uint>(sharedMemory.size());
            bytes.setRawData(ptr, n);
            sharedMemory.unlock();
        }

        message += QString(bytes);
        message += QChar::Space;

        if (sharedMemory.lock()) {
            // Replies the ACK message
            bytes = C_SHARED_MEMORY_ACK_REPLY.toUtf8();

            const char *from = bytes.constData();
            void *to = sharedMemory.data();
            size_t size = static_cast<size_t>(qMin(bytes.size() + 1, sharedMemory.size()));
            memcpy(to, from, size);

            char *d_ptr = static_cast<char*>(sharedMemory.data());
            d_ptr[sharedMemory.size() - 1] = '\0';

            sharedMemory.unlock();
        }

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

bool InterProcessCommunication::isUrl(const QString &message)
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
    const QStringList resources = message.split(QChar::Space, QString::SkipEmptyParts);
    for (int i = 0; i < resources.count() - 1; ++i) {
        if (resources.at(i).trimmed() == C_KEYWORD_OPEN_URL) {
            return resources.at(i+1).trimmed();
        }
    }
    return QString();
}

bool InterProcessCommunication::isCommandDownloadLink(const QString &message)
{
    return message.contains(C_KEYWORD_DOWNLOAD_LINK, Qt::CaseInsensitive);
}

QString InterProcessCommunication::getDownloadLink(const QString &message)
{
    const QStringList resources = message.split(QChar::Space, QString::SkipEmptyParts);
    for (int i = 0; i < resources.count() - 1; ++i) {
        if (resources.at(i).trimmed() == C_KEYWORD_DOWNLOAD_LINK) {
            return resources.at(i+1).trimmed();
        }
    }
    return QString();
}

void InterProcessCommunication::parseMessage(const QString &message, Model *model)
{
    if (model == nullptr) {
        return;
    }
    if (message.contains(C_PACKET_ERROR, Qt::CaseInsensitive)) {
        return;
    }

    const QStringList resources = message.split(QChar::Space, QString::SkipEmptyParts);

    int mode = -1;
    foreach (auto resource, resources) {
        auto url = resource.trimmed();
        if (url.isEmpty()) {
            continue;
        }

        if (url == C_KEYWORD_CURRENT_URL) {
            mode = 0;

        } else if (url == C_KEYWORD_LINKS) {
            mode = 1;

        } else if (url == C_KEYWORD_MEDIA) {
            mode = 2;

        } else if (url.contains('[')) {
            // C_PACKET_BEGIN
            // C_PACKET_END
            // C_KEYWORD_OPEN_URL
            // ...
            mode = -1;

        } else {
            if (mode == 1) {
                ResourceItem *item = new ResourceItem();
                item->setUrl(resource);
                model->linkModel()->addResource(item);

            } else if (mode == 2) {
                ResourceItem *item = new ResourceItem();
                item->setUrl(resource);
                model->contentModel()->addResource(item);
            }
        }
    }
}
