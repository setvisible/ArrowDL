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

#ifndef WIDGETS_QUEUE_VIEW_H
#define WIDGETS_QUEUE_VIEW_H

#include <QtCore/QItemSelection>
#include <QtCore/QModelIndex>
#include <QtWidgets/QTableView>

class AbstractJob;

class QMenu;

class QueueView: public QTableView
{
    Q_OBJECT

public:
    explicit QueueView(QWidget *parent);
    ~QueueView() override = default;

    void setModel(QAbstractItemModel *model) override;

    QMenu* contextMenu() const;
    void setContextMenu(QMenu *contextMenu);

    QSize sizeHint() const override;

    QByteArray saveState(int version = 0) const;
    bool restoreState(const QByteArray &state, int version = 0);

    QList<AbstractJob *> selectedItems() const;
    QString selectionToString() const;
    QString selectionToClipboard() const;

public slots:
    void rename();

    void selectAll() override;
    void selectNone();
    void invertSelection();
    void selectCompleted();

    void removeCompleted();
    void removeSelected();
    void removeAll();
    void moveSelectionToTrash();

    void moveUp();
    void moveTop();
    void moveDown();
    void moveBottom();

signals:
    void dataChanged();
    void selectionChanged();
    void doubleClicked(AbstractJob *item);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

protected slots:
    void changeEvent(QEvent *event) override;

private slots:
    void onRowsInserted(const QModelIndex &parent, int first, int last);
    void onRowsRemoved(const QModelIndex &parent, int first, int last);
    void onRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                     const QModelIndex &destinationParent, int destinationRow);
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                       const QList<int> &roles = QList<int>());
    void onModelReset();

    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void onDoubleClicked(const QModelIndex &index);
    void onCommitData(QWidget *editor);

    void showContextMenu(const QPoint &pos) ;

private:
    QMenu *m_contextMenu = nullptr;
    QPoint dragStartPosition = {};

    void retranslateUi();
    void restylizeUi();

    QUrl urlFrom(const AbstractJob *item) const;

    QList<int> columnWidths() const;
    void setColumnWidths(const QList<int> &widths);

    enum Direction { Up, Top, Down, Bottom };
    void move(Direction direction);
    AbstractJob *getItemAtRow(const int row) const;
};

#endif // WIDGETS_QUEUE_VIEW_H
