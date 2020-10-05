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

#ifndef WIDGETS_TORRENT_WIDGET_H
#define WIDGETS_TORRENT_WIDGET_H

#include <Core/Torrent>

#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QWidget>

class TorrentBaseContext;
class Torrent;

class QAction;
class QLabel;
class QTableView;

namespace Ui {
class TorrentWidget;
}

class TorrentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TorrentWidget(QWidget *parent);
    ~TorrentWidget() Q_DECL_OVERRIDE;

    TorrentBaseContext* torrentContext() const;
    void setTorrentContext(TorrentBaseContext *torrentContext);

    void clear();
    bool isEmpty() const;

    Torrent* torrent() const;
    void setTorrent(Torrent *torrent);

    QByteArray saveState(int version = 0) const;
    bool restoreState(const QByteArray &state, int version = 0);

protected slots:
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onChanged();

    void onSectionClicked(int logicalIndex);

    void showContextMenuFileTable(const QPoint &pos);
    void showContextMenuPeerTable(const QPoint &pos);
    void showContextMenuTrackerTable(const QPoint &pos);

    void setPriorityHigh();
    void setPriorityNormal();
    void setPriorityLow();
    void setPrioritySkip();
    void setPriorityByFileOrder();

    void copy();

    void addPeer();
    void copyPeerList();
    void removeUnconnected();

    void addTracker();
    void removeTracker();
    void copyTrackerList();

private:
    Ui::TorrentWidget *ui;
    TorrentBaseContext *m_torrentContext;
    Torrent *m_torrent;

    QList<int> m_fileColumnsWidths;
    QList<int> m_peerColumnsWidths;
    QList<int> m_trackerColumnsWidths;

    void resetUi();
    void retranslateUi();

    void setupUiTableView(QTableView *view);
    void setupInfoCopy();
    void setupInfoCopy(QLabel *label, QFrame *buddy);
    void setupContextMenus();

    void setPriority(TorrentFileInfo::Priority priority);
    TorrentFileInfo::Priority assessPriority(int row, int count);

    void getColumnWidths(QTableView *view, QList<int> *widths);
    void setColumnWidths(QTableView *view, const QList<int> &widths);

    void updateWidget();
    void updateProgressBar();
    void updateTorrentPage();

    static inline QString text(int value, bool showInfiniteSymbol = false);
    static inline QString text(const QString &text);
    static inline QString text(const QDateTime &datetime);
};

/******************************************************************************
 ******************************************************************************/
/*!
 * FileTableViewItemDelegate is used to draw the progress bar.
 */
class FileTableViewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FileTableViewItemDelegate(QObject *parent = Q_NULLPTR);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index ) const Q_DECL_OVERRIDE;
};

/******************************************************************************
 ******************************************************************************/
/*!
 * PeerTableViewItemDelegate is used to draw the pieces owned by the peer.
 */
class PeerTableViewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit PeerTableViewItemDelegate(QObject *parent = Q_NULLPTR);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index ) const Q_DECL_OVERRIDE;
};

/******************************************************************************
 ******************************************************************************/
class TrackerTableViewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TrackerTableViewItemDelegate(QObject *parent = Q_NULLPTR);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index ) const Q_DECL_OVERRIDE;
};

#endif // WIDGETS_TORRENT_WIDGET_H
