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

private slots:
    void onChanged();

private:
    Ui::TorrentWidget *ui;
    DownloadTorrentItem *m_item;

    void resetUi();

    void setupUiTableView(QTableView *view);
    void setColumnWidths(QTableView *view, const QList<int> &widths);

    void updateProgressBar();
    void updateInfoTabPage();

    static inline QString text(int value, bool showInfiniteSymbol = false);
    static inline QString text(const QString &text);
    static inline QString text(const QDateTime &datetime);
};

#endif // WIDGETS_TORRENT_WIDGET_H
