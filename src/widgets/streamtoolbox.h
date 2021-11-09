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

#ifndef WIDGETS_STREAM_TOOl_BOX_H
#define WIDGETS_STREAM_TOOl_BOX_H

#include <Core/Stream>

//#include <QtCore/QAbstractTableModel>
#include <QtWidgets/QToolBox>

class QAbstractButton;

namespace Ui {
class StreamToolBox;
}

//class StreamSubtitleTableModel : public QAbstractTableModel
//{
//    Q_OBJECT
//public:
//    explicit StreamSubtitleTableModel(QObject *parent = nullptr);
//    ~StreamSubtitleTableModel() Q_DECL_OVERRIDE;

//    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
//    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

//    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
//    virtual QVariant headerData(int section, Qt::Orientation orientation,
//                                int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
//};
//class ButtonPressEater : public QObject
//{
//    Q_OBJECT
////public:
////    ButtonPressEater() = default;
////        ~ButtonPressEater() = default;
////        explicit ButtonPressEater(QObject *parent=nullptr) = default;

//protected:
//    bool eventFilter(QObject *obj, QEvent *event) override;
//};


#define HACK_CLOSABLE_PAGES


class StreamToolBox : public QToolBox
{
    Q_OBJECT
public:
    explicit StreamToolBox(QWidget *parent, Qt::WindowFlags f = Qt::WindowFlags());
    ~StreamToolBox() Q_DECL_OVERRIDE;

    void clear();
    void setConfig(const StreamObject::Config &config);

signals:
    void configChanged(StreamObject::Config config);


private slots:
    void onCheckBoxToggled(bool checked);
    void onChanged();

private:
    Ui::StreamToolBox *ui;
    //    StreamSubtitleTableModel *m_subtitleModel;

    void propagateIcons();


#ifdef HACK_CLOSABLE_PAGES
signals:
    void _q_closeToolButton();
private slots:
    void _q_onToolButtonReleased();
    void _q_onToolButtonClosed();
private:
    QList<QAbstractButton *> m_buttons;
#endif
};

/******************************************************************************
 ******************************************************************************/
//class RadioItemDelegate : public QStyledItemDelegate
//{
//    Q_OBJECT
//public:
//    explicit RadioItemDelegate(QObject *parent = Q_NULLPTR);

//    // painting
//    void paint(QPainter *painter, const QStyleOptionViewItem &option,
//               const QModelIndex &index) const Q_DECL_OVERRIDE;

//    // editing
//    bool editorEvent(QEvent *event, QAbstractItemModel *model,
//                     const QStyleOptionViewItem &option,
//                     const QModelIndex &index) Q_DECL_OVERRIDE;
//};

#endif // WIDGETS_STREAM_TOOl_BOX_H
