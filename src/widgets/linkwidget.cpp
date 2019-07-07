/* - DownZemAll! - Copyright (C) 2019 Sebastien Vavassori
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

#include "linkwidget.h"
#include "ui_linkwidget.h"

#include <Core/Model>
#include <Core/ResourceModel>

#include <QtCore/QModelIndex>
#include <QtGui/QPainter>
#include <QtWidgets/QItemDelegate>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QTableView>
#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

#define C_CHECKBOX_WIDTH 16   /* check_ok_16x16.png */

/*!
 * LinkWidgetItemDelegate is used to draw the check icons.
 */
class LinkWidgetItemDelegate : public QStyledItemDelegate
{
    /*
     * If use Q_OBJECT, signals and slots in nested classes, add
     * <code>
     * #include "linkwidget.moc"
     * </code>
     * at this file's end.
     */
    Q_OBJECT

public:
    inline LinkWidgetItemDelegate(QObject *parent = Q_NULLPTR);

    ~LinkWidgetItemDelegate() {}

    // painting
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

    // editing
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) Q_DECL_OVERRIDE;
private:
    QIcon m_icon;
};

LinkWidgetItemDelegate::LinkWidgetItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    m_icon.addPixmap(QPixmap(":/icons/menu/check_ok_16x16.png"), QIcon::Normal, QIcon::On);
    m_icon.addPixmap(QPixmap(":/icons/menu/check_progress_16x16.png"), QIcon::Disabled, QIcon::On);
}

void LinkWidgetItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    const bool selected = index.model()->data(index, Qt::UserRole).toBool();

    if (selected) {
        painter->fillRect(option.rect, QColor(255,255,179)); // light yellow
    }
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
        QFont font = painter->font();
        font.setBold(true);
        painter->setFont(font);
    }

    QStyledItemDelegate::paint(painter, option, index);

    if (index.column() == 0) {
        QStyleOptionButton button;
        button.rect = option.rect;
        button.iconSize = QSize(C_CHECKBOX_WIDTH, C_CHECKBOX_WIDTH);
        button.icon = m_icon;
        button.features |= QStyleOptionButton::Flat;
        button.state |= selected ? QStyle::State_Enabled : QStyle::State_None;
        QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
    }
}

QSize LinkWidgetItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(C_CHECKBOX_WIDTH,C_CHECKBOX_WIDTH);
}

bool LinkWidgetItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                         const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress &&  index.column() == 0) {
        const bool selected = index.model()->data(index, Qt::UserRole).toBool();
        model->setData(index, !selected, Qt::UserRole);
        return true;
    }
    return QStyledItemDelegate::editorEvent(event,model,option, index);
}

/******************************************************************************
 ******************************************************************************/

LinkWidget::LinkWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::LinkWidget)
  , m_model(Q_NULLPTR)
{
    ui->setupUi(this);

    setup(ui->linkTableView);
    setup(ui->contentTableView);

    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged(int)));
}

LinkWidget::~LinkWidget()
{
    delete ui;
}

void LinkWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    resize();
}

/******************************************************************************
 ******************************************************************************/
void LinkWidget::setup(QTableView *view)
{
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Item Delegate
    view->setItemDelegate(new LinkWidgetItemDelegate(view));
}

/******************************************************************************
 ******************************************************************************/
void LinkWidget::resize()
{
    ui->tabWidget->setCurrentIndex(0);
    resize(ui->linkTableView);
    ui->tabWidget->setCurrentIndex(1);
    resize(ui->contentTableView);
    ui->tabWidget->setCurrentIndex(0);
}

void LinkWidget::resize(QTableView *view)
{
    view->setColumnWidth(0, C_CHECKBOX_WIDTH);
    const int width = view->width() - C_CHECKBOX_WIDTH;
    view->setColumnWidth(1, width*0.5);
    view->setColumnWidth(2, width*0.2);
    view->setColumnWidth(3, width*0.2);
    view->setColumnWidth(4, width*0.1);
}

/******************************************************************************
 ******************************************************************************/
Model* LinkWidget::model() const
{
    return m_model;
}

void LinkWidget::setModel(Model *model)
{
    if (m_model) {
        disconnect(m_model->linkModel(), SIGNAL(resourceChanged()), this, SLOT(onResourceChanged()));
        disconnect(m_model->contentModel(), SIGNAL(resourceChanged()), this, SLOT(onResourceChanged()));

        ui->linkTableView->setModel(Q_NULLPTR);
        ui->contentTableView->setModel(Q_NULLPTR);
    }
    m_model = model;
    if (m_model) {
        connect(m_model->linkModel(), SIGNAL(resourceChanged()), this, SLOT(onResourceChanged()));
        connect(m_model->contentModel(), SIGNAL(resourceChanged()), this, SLOT(onResourceChanged()));

        ui->linkTableView->setModel(m_model->linkModel());
        ui->contentTableView->setModel(m_model->contentModel());
    }
}

/******************************************************************************
 ******************************************************************************/
void LinkWidget::onCurrentTabChanged(int index)
{
    if (index == 0) {
        m_model->setCurrentTab(Model::LINK);
    } else {
        m_model->setCurrentTab(Model::CONTENT);
    }
}

void LinkWidget::onResourceChanged()
{
    ui->tabWidget->setTabText(0, tr("Links (%0)").arg(m_model->linkModel()->rowCount()));
    ui->tabWidget->setTabText(1, tr("Pictures and Media (%0)").arg(m_model->contentModel()->rowCount()));
}

#include "linkwidget.moc"
