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

#ifndef CORE_DOWNLOAD_ENGINE_H
#define CORE_DOWNLOAD_ENGINE_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

class AbstractDownloadItem;
class QTimer;

using DownloadRange = QList<AbstractDownloadItem *>;

class DownloadEngine : public QObject
{
    Q_OBJECT

public:
    explicit DownloadEngine(QObject *parent = nullptr);
    ~DownloadEngine() override;

    /* Queue Management */
    qsizetype count() const;
    void clear();

    virtual void append(const QList<AbstractDownloadItem *> &items, bool started = false);
    virtual void remove(const QList<AbstractDownloadItem *> &items);

    void removeItems(const QList<AbstractDownloadItem *> &items);
    void updateItems(const QList<AbstractDownloadItem *> &items);

    const AbstractDownloadItem* clientForRow(qsizetype row) const;

    int maxSimultaneousDownloads() const;
    void setMaxSimultaneousDownloads(int number);

    /* Statistics */
    QList<AbstractDownloadItem *> downloadItems() const;
    QList<AbstractDownloadItem *> completedJobs() const;
    QList<AbstractDownloadItem *> failedJobs() const;
    QList<AbstractDownloadItem *> runningJobs() const;

    qreal totalSpeed();

    /* Actions */
    void resume(AbstractDownloadItem *item);
    void pause(AbstractDownloadItem *item);
    void cancel(AbstractDownloadItem *item);

    /* Selection */
    void clearSelection();
    QList<AbstractDownloadItem *> selection() const;
    void setSelection(const QList<AbstractDownloadItem *> &selection);

    bool isSelected(AbstractDownloadItem *item) const;
    void setSelected(AbstractDownloadItem *item, bool isSelected);

    QString selectionToString() const;
    QString selectionToClipboard() const;

    void beginSelectionChange(); // BUGFIX
    void endSelectionChange();

    void moveCurrentTop();
    void moveCurrentUp();
    void moveCurrentDown();
    void moveCurrentBottom();

    /* Misc */
    void movetoTrash(const QList<AbstractDownloadItem *> &items);

    /* Utility */
    virtual AbstractDownloadItem* createFileItem(const QUrl &url);
    virtual AbstractDownloadItem* createTorrentItem(const QUrl &url);

signals:
    void jobAppended(DownloadRange range);
    void jobRemoved(DownloadRange range);
    void jobStateChanged(AbstractDownloadItem *item);
    void jobFinished(AbstractDownloadItem *item);
    void jobRenamed(QString oldName, QString newName, bool success);

    void selectionChanged();
    void sortChanged();

public slots:

private slots:
    void onChanged();
    void onFinished();
    void onRenamed(const QString &oldName, const QString &newName, bool success);
    void startNext(AbstractDownloadItem *item);

private slots:
    void onSpeedTimerTimeout();

private:
    QList<AbstractDownloadItem *> m_items = {};

    qreal m_previouSpeed = 0;
    QTimer* m_speedTimer = nullptr;

    // Pool
    int m_maxSimultaneousDownloads = 4;
    qsizetype downloadingCount() const;

    QList<AbstractDownloadItem *> m_selectedItems = {};
    bool m_selectionAboutToChange = false;

    void sortSelectionByIndex();
    void moveUpTo(qsizetype targetIndex);
    void moveDownTo(qsizetype targetIndex);
};

#endif // CORE_DOWNLOAD_ENGINE_H
