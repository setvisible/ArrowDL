/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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

#include <QtWidgets/QWidget>

class QTableView;

class IDownloadItem;
class DownloadTorrentItem;

namespace Ui {
class TorrentWidget;
}

class TorrentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TorrentWidget(QWidget *parent);
    ~TorrentWidget() Q_DECL_OVERRIDE;

    void clear();
    bool isEmpty() const;

    IDownloadItem* downloadItem() const;
    void setDownloadItem(IDownloadItem *item);

    QByteArray saveState(int version = 0) const;
    bool restoreState(const QByteArray &state, int version = 0);

protected slots:
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onChanged();

private:
    Ui::TorrentWidget *ui;
    DownloadTorrentItem *m_item;
    QList<int> m_fileColumnsWidths;
    QList<int> m_peerColumnsWidths;
    QList<int> m_trackerColumnsWidths;

    void resetUi();
    void retranslateUi();

    void setupUiTableView(QTableView *view);

    void getColumnWidths(QTableView *view, QList<int> *widths);
    void setColumnWidths(QTableView *view, const QList<int> &widths);

    void updateWidget();
    void updateProgressBar();
    void updateTorrentPage();

    static inline QString text(int value, bool showInfiniteSymbol = false);
    static inline QString text(const QString &text);
    static inline QString text(const QDateTime &datetime);
};

#endif // WIDGETS_TORRENT_WIDGET_H
