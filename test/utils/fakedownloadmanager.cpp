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

#include "fakedownloadmanager.h"

#include "fakedownloaditem.h"

FakeDownloadManager::FakeDownloadManager(QObject *parent) : DownloadEngine(parent)
{
}

FakeDownloadManager::~FakeDownloadManager()
{
}

IDownloadItem* FakeDownloadManager::createItem(const QUrl &url)
{
    FakeDownloadItem *item = new FakeDownloadItem(this);
    item->setSourceUrl(url);
    return item;
}

void FakeDownloadManager::createFakeJobs(int count)
{
    QList<IDownloadItem*> items;
    for (auto i = 0; i < count; ++i) {
        auto item = new FakeDownloadItem(this);
        items.append(item);
    }
    DownloadEngine::append(items, false);
}

void FakeDownloadManager::appendFakeJob(const QUrl &url)
{
    IDownloadItem *item = createItem(url);

    QList<IDownloadItem*> items;
    items.append(item);
    DownloadEngine::append(items, false);
}
