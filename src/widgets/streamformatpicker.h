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

#ifndef WIDGETS_STREAM_FORMAT_PICKER_H
#define WIDGETS_STREAM_FORMAT_PICKER_H

#include <Core/Stream>

#include <QtCore/QItemSelection>
#include <QtWidgets/QWidget>
#include <QtWidgets/QStyledItemDelegate>

class QComboBox;
class QStandardItemModel;

namespace Ui {
class StreamFormatPicker;
}

class StreamFormatPicker : public QWidget
{
    Q_OBJECT
public:
    enum ItemDataRole {
        FormatIdRole = Qt::UserRole + 1,
        CheckStateRole
    };

    explicit StreamFormatPicker(QWidget *parent);
    ~StreamFormatPicker() override;

    void clear();
    void setData(const StreamObject &streamObject);

    void select(const StreamFormatId &formatId);

    StreamFormatId selection() const;
    StreamObject::Config config() const;

signals:
    void selectionChanged(StreamFormatId formatId);
    void configChanged();

private slots:
    void onCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void onCurrentIndexChanged(int index);
    void onButtonBarClicked();

private:
    Ui::StreamFormatPicker *ui;
    QStandardItemModel *m_model;

    void propagateIcons();
    void updateButtonBar();

    void populateSimple(const QList<StreamFormat> &formats);
    void populateComboBox(const QList<StreamFormat> &formats, QComboBox *comboBox);

    QModelIndex find(const StreamFormatId &id) const;

    void setCurrentSimple(const StreamFormatId &id);
    void setCurrentAudio(const StreamFormatId &id);
    void setCurrentVideo(const StreamFormatId &id);

    StreamFormatId currentSimple() const;
    StreamFormatId currentAudio() const;
    StreamFormatId currentVideo() const;
};

/******************************************************************************
 ******************************************************************************/
class RadioItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit RadioItemDelegate(QObject *parent = nullptr);

    // painting
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // editing
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

#endif // WIDGETS_STREAM_FORMAT_PICKER_H
