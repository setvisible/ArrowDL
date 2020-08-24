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

#ifndef WIDGETS_STREAM_LIST_WIDGET_H
#define WIDGETS_STREAM_LIST_WIDGET_H

#include <Core/Stream>
#include <Core/CheckableTableModel>

#include <QtWidgets/QWidget>
#include <QtCore/QItemSelection>

class StreamTableModel;

namespace Ui {
class StreamListWidget;
}

class StreamListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StreamListWidget(QWidget *parent);
    ~StreamListWidget() Q_DECL_OVERRIDE;

    void retranslateUi();

    QList<int> columnWidths() const;
    void setColumnWidths(const QList<int> &widths);

    bool isValid() const;

    void setMessageEmpty();
    void setMessageWait();
    void setMessageError(QString errorMessage);

    void setStreamInfoList(StreamInfo streamInfo);
    void setStreamInfoList(QList<StreamInfo> streamInfoList);

    QList<StreamInfo> selection() const;

protected slots:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onSelectionChanged(const QItemSelection &selected,
                            const QItemSelection &deselected);
    void onStreamInfoChanged(StreamInfo streamInfo);
    void onCheckStateChanged(QModelIndex index, bool checked);

private:
    Ui::StreamListWidget *ui;
    StreamTableModel *m_playlistModel;

    enum State {
        Empty,
        Downloading,
        Normal,
        Error
    };
    State m_state = Empty;

    State state() const;
    void setState(State state);

    QList<int> selectedRows() const;
};

/******************************************************************************
 ******************************************************************************/
class StreamTableModel: public CheckableTableModel
{
    Q_OBJECT

public:
    explicit StreamTableModel(QObject *parent);
    ~StreamTableModel() Q_DECL_OVERRIDE = default;

    void clear() Q_DECL_OVERRIDE;

    void retranslateUi();

    void setStreamInfoList(QList<StreamInfo> streamInfoList);

    StreamInfo itemAt(int row) const;
    void setItemAt(int row, const StreamInfo &streamInfo);

    QList<StreamInfo> selection() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    QStringList m_headers;
    QList<StreamInfo> m_items;
    QString filenameOrErrorMessage(const StreamInfo &streamInfo) const;
};

#endif // WIDGETS_STREAM_LIST_WIDGET_H
