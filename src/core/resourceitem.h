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

#ifndef CORE_RESOURCE_ITEM_H
#define CORE_RESOURCE_ITEM_H

#include <QtCore/QString>
#include <QtCore/QUrl>

class ResourceItem
{    
public:
    ResourceItem();
    ~ResourceItem();

    /* Source */
    QString url() const;
    void setUrl(const QString &url);
    QUrl distantFileUrl() const;

    /* Destination */
    QString destination() const;
    void setDestination(const QString &destination);

    QString mask() const;
    void setMask(const QString &mask);

    QString customFileName() const;
    void setCustomFileName(const QString &customFileName);

    /* Local file URL, once the file is downloaded */
    QUrl localFileUrl() const;
    QString fileName() const;

    /* Options */
    QString referringPage() const;
    void setReferringPage(const QString &referringPage);

    QString description() const;
    void setDescription(const QString &description);

    QString checkSum() const;
    void setCheckSum(const QString &checkSum);


    bool isSelected() const; // OBSOLETE
    void setSelected(const bool isSelected); // OBSOLETE

private:
    QString m_url;              // QUrl ?
    QString m_destination;      // QDir ?
    QString m_mask;             // Mask ?
    QString m_customFileName;   // QFileInfo ?

    QString m_referringPage;
    QString m_description;
    QString m_checkSum;

    bool m_isSelected;
};

#endif // CORE_RESOURCE_ITEM_H
