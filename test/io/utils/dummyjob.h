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

#ifndef DUMMY_JOB_H
#define DUMMY_JOB_H

#include <Core/AbstractJob>

class DummyJob : public AbstractJob
{
    Q_OBJECT

public:
    explicit DummyJob(QObject *parent, ResourceItem *resource);

    virtual QUrl sourceUrl() const override;
    virtual void setSourceUrl(const QUrl &resourceUrl) override;

    virtual QString localFileName() const override;

    virtual QString localFullFileName() const override;
    virtual QString localFilePath() const override;

    virtual QUrl localFileUrl() const override;
    virtual QUrl localDirUrl() const override;

private:
    QUrl m_resourceUrl;
    QString m_resourceLocalFileName;
};

#endif // DUMMY_JOB_H
