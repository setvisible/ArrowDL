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

#ifndef CORE_RESOURCE_ITEM_H
#define CORE_RESOURCE_ITEM_H

#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QVariant>

class ResourceItem
{    
public:
    ResourceItem();
    ~ResourceItem() = default;

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
    QString localFileFullPath(const QString &customFileName = QString()) const;

    /* Options */
    QString referringPage() const;
    void setReferringPage(const QString &referringPage);

    QString description() const;
    void setDescription(const QString &description);

    QString checkSum() const;
    void setCheckSum(const QString &checkSum);

    bool isStreamEnabled() const;
    void setStreamEnabled(bool enabled);

    QString streamFileName() const;
    void setStreamFileName(const QString &streamFileName);

    QString streamFormatId() const;
    void setStreamFormatId(const QString &streamFormatId);

    qint64 streamFileSize() const;
    void setStreamFileSize(qint64 streamFileSize);

    bool isTorrentEnabled() const;
    void setTorrentEnabled(bool enabled);

    QString torrentPreferredFilePriorities() const;
    void setTorrentPreferredFilePriorities(const QString &priorities);

    bool isSelected() const; // OBSOLETE
    void setSelected(bool isSelected); // OBSOLETE

private:
    QString m_url;              // QUrl ?
    QString m_destination;      // QDir ?
    QString m_mask;             // Mask ?
    QString m_customFileName;   // QFileInfo ?

    QString m_referringPage;
    QString m_description;
    QString m_checkSum;

    bool m_isStreamEnabled; // true=download the stream; false=download the page
    QString m_streamFileName;
    QString m_streamFormatId;
    qint64 m_streamFileSize;

    bool m_isTorrentEnabled;
    QString m_torrentPreferredFilePriorities;

    bool m_isSelected = false;

    inline QString localFilePath(const QString &customFileName) const;
    inline QString localStreamFile(const QString &customFileName) const;
    inline QString localMagnetFile(const QString &customFileName) const;

    inline QString parseMagnetUrl(const QString &url) const;

    inline static QString localFile(const QString &destination, const QUrl &url,
                                    const QString &customFileName, const QString &mask);
};

#endif // CORE_RESOURCE_ITEM_H
