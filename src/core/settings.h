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

#ifndef CORE_SETTINGS_H
#define CORE_SETTINGS_H

#include <Core/AbstractSettings>

struct Filter
{
    QString title;
    QString regexp;
};

/*!
 * User preferences are edited with the 'Preferences' dialog.
 */
class Settings : public AbstractSettings
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent);
    virtual ~Settings();

    // Tab General

    // Tab Interface
    bool isStartMinimizedEnabled() const;
    void setStartMinimizedEnabled(bool enabled);

    bool isConfirmRemovalEnabled() const;
    void setConfirmRemovalEnabled(bool enabled);

    // Tab Network
    int maxSimultaneousDownloads() const;
    void setMaxSimultaneousDownloads(int number);


    // Tab Privacy
    QString database() const;
    void setDatabase(const QString &value);

    // Tab Filters
    QList<Filter> filters() const;
    void setFilters(const QList<Filter> &filters);

    // Tab Schedule

    // Tab Advanced

};

#endif // CORE_SETTINGS_H
