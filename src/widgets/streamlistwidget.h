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
    void setMessageError(const QString &errorMessage);

    void setStreamObjects(const StreamObject &streamObject);
    void setStreamObjects(const QList<StreamObject> &streamObjects);

    QList<StreamObject> selection() const;

protected slots:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onSelectionChanged(const QItemSelection &selected,
                            const QItemSelection &deselected);
    void onStreamObjectChanged(const StreamObject &streamObject);
    void onCheckStateChanged(const QModelIndex &index, bool checked);
    void onTrackNumberChecked(int state);

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

    void setStreamObjects(const QList<StreamObject> &streamObjects);
    void enableTrackNumberPrefix(bool enable);

    StreamObject itemAt(int row) const;
    void setItemAt(int row, const StreamObject &streamObject);

    QList<StreamObject> selection() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

private:
    QStringList m_headers;
    QList<StreamObject> m_items;
    QString filenameOrErrorMessage(const StreamObject &streamObject) const;
};

#endif // WIDGETS_STREAM_LIST_WIDGET_H
