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

#ifndef FAKE_DOWNLOAD_MANAGER_H
#define FAKE_DOWNLOAD_MANAGER_H

#include <Core/DownloadEngine>

class FakeDownloadManager : public DownloadEngine
{
    Q_OBJECT

public:
    explicit FakeDownloadManager(QObject *parent = nullptr);
    ~FakeDownloadManager();

    virtual IDownloadItem* createItem(const QUrl &url) override;

    /* Utility */
    void createFakeJobs(int count = 100);
    void appendFakeJob(const QUrl &url);

};

#endif // FAKE_DOWNLOAD_MANAGER_H
