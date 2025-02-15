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

#include "dummydownloaditem.h"
#include "dummydownloadmanager.h"

#include <Core/AbstractDownloadItem>

DummyDownloadManager::DummyDownloadManager(QObject *parent) : QObject(parent)
{
}

void DummyDownloadManager::append(const QList<AbstractDownloadItem *> &items, bool started)
{
    m_items.append(items);
}

void DummyDownloadManager::remove(const QList<AbstractDownloadItem *> &items)
{
    for (auto item : items) {
        m_items.removeAll(item);
    }
}

QList<AbstractDownloadItem *> DummyDownloadManager::downloadItems() const
{
    return m_items;
}

AbstractDownloadItem* DummyDownloadManager::createFileItem(const QUrl &url)
{
    auto item = new DummyDownloadItem(this);
    item->setSourceUrl(url);
    return item;
}

AbstractDownloadItem* DummyDownloadManager::createTorrentItem(const QUrl &url)
{
    Q_UNUSED(url);
    return nullptr;
}


void DummyDownloadManager::createFakeJobs(int count)
{
    QList<AbstractDownloadItem*> items;
    for (auto i = 0; i < count; ++i) {
        auto item = new DummyDownloadItem(this);
        items.append(item);
    }
    append(items, false);
}

void DummyDownloadManager::appendFakeJob(const QUrl &url)
{
    AbstractDownloadItem *item = createFileItem(url);

    QList<AbstractDownloadItem*> items;
    items.append(item);
    append(items, false);
}
