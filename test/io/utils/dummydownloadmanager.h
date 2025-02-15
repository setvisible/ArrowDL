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

#ifndef DUMMY_DOWNLOAD_MANAGER_H
#define DUMMY_DOWNLOAD_MANAGER_H

#include <Core/IDownloadManager>

#include <QtCore/QObject>

class DummyDownloadManager : public QObject, public IDownloadManager
{
    Q_OBJECT
public:
    explicit DummyDownloadManager(QObject *parent = nullptr);
    ~DummyDownloadManager() override = default;

    void append(const QList<AbstractDownloadItem *> &items, bool started = false) override;
    void remove(const QList<AbstractDownloadItem *> &items) override;

    QList<AbstractDownloadItem *> downloadItems() const override;

    AbstractDownloadItem* createFileItem(const QUrl &url) override;
    AbstractDownloadItem* createTorrentItem(const QUrl &url) override;

    // Utility
    void createFakeJobs(int count = 100);
    void appendFakeJob(const QUrl &url);

private:
    QList<AbstractDownloadItem *> m_items;
};

#endif // DUMMY_DOWNLOAD_MANAGER_H
