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
#include <Core/ResourceItem>
#include <Core/ResourceModel>

#include <QtCore/QModelIndex>
#include <QtGui/QClipboard>
#include <QtGui/QDesktopServices>
#include <QtGui/QIcon>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QAction>
#include <QtWidgets/QItemDelegate>
#include <QtWidgets/QMenu>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QTableView>
#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

#define C_CHECKBOX_WIDTH 12   /* check_ok_16x16.png */
#define C_CHECKBOX_COLUMN_WIDTH 16
#define C_COLUMN_DEFAULT_WIDTH 100

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
        painter->fillRect(option.rect, QColor(255, 255, 179)); // light yellow
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

QSize LinkWidgetItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

bool LinkWidgetItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                         const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress && index.column() == 0) {
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

/******************************************************************************
 ******************************************************************************/
void LinkWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    resize();
}

/******************************************************************************
 ******************************************************************************/
void LinkWidget::keyPressEvent(QKeyEvent *event)
{
    QKeySequence sequence(event->key() | event->modifiers());
    if (sequence == QKeySequence(QKeySequence::SelectAll)) {
        selectAll();

    } else if (sequence == QKeySequence(Qt::CTRL + Qt::Key_F)) {
        selectFiltered();

    } else if (sequence == QKeySequence(Qt::CTRL + Qt::Key_I)) {
        invertSelection();

    } else if (sequence == QKeySequence(QKeySequence::Copy)) {
        copyLinks();

    } else if (sequence == QKeySequence(QKeySequence::Open)) {
        open();

    } else {
        QWidget::keyPressEvent(event); // important!
    }
}

/******************************************************************************
 ******************************************************************************/
void LinkWidget::setup(QTableView *view)
{
    view->setShowGrid(false);

    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);

    view->setItemDelegate(new LinkWidgetItemDelegate(view));
    QHeaderView *verticalHeader = view->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(22);
    verticalHeader->setVisible(false);

    QHeaderView *horizontalHeader = view->horizontalHeader();
    horizontalHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    horizontalHeader->setHighlightSections(false);

    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));
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
QList<int> LinkWidget::columnWidths() const
{
    QAbstractItemModel *model = ui->linkTableView->model();
    Q_ASSERT(model);
    QList<int> widths;
    if (model) {
        for (int column = 0; column < model->columnCount(); ++column) {
            const int width = ui->linkTableView->columnWidth(column);
           widths.append(width);
        }
    }
    return widths;
}

void LinkWidget::setColumnWidths(const QList<int> &widths)
{
    QAbstractItemModel *model = ui->linkTableView->model();
    Q_ASSERT(model);
    if (model) {
        for (int column = 0; column < model->columnCount(); ++column) {
            if (column == 0) {
                ui->linkTableView->setColumnWidth(column, C_CHECKBOX_COLUMN_WIDTH);
            } else if (column > 0 && column < widths.count()) {
                const int width = widths.at(column);
                ui->linkTableView->setColumnWidth(column, width);
            } else {
                ui->linkTableView->setColumnWidth(column, C_COLUMN_DEFAULT_WIDTH);
            }
        }
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

/******************************************************************************
 ******************************************************************************/
void LinkWidget::showContextMenu(const QPoint &/*pos*/)
{
    QMenu *contextMenu = new QMenu(this);

    QAction actionCheckSelected(tr("Check Selected Items"), contextMenu);
    actionCheckSelected.setIcon(QIcon(":/icons/menu/check_ok_16x16.png"));
    connect(&actionCheckSelected, SIGNAL(triggered()), this, SLOT(checkSelected()));

    QAction actionUncheckSelected(tr("Uncheck Selected Items"), contextMenu);
    actionUncheckSelected.setIcon(QIcon(":/icons/menu/check_nok_16x16.png"));
    connect(&actionUncheckSelected, SIGNAL(triggered()), this, SLOT(uncheckSelected()));

    QAction actionToggleCheck(tr("Toggle Check for Selected Items"), contextMenu);
    actionToggleCheck.setIcon(QIcon(":/icons/menu/check_progress_16x16.png"));
    connect(&actionToggleCheck, SIGNAL(triggered()), this, SLOT(toggleCheck()));
    // --
    QAction actionMask(tr("Mask..."), contextMenu);
    actionMask.setIcon(QIcon(":/icons/menu/icon_mask_16x16.png"));
    connect(&actionMask, SIGNAL(triggered()), this, SLOT(customizeMask()));
    // --
    QAction actionSelectAll(tr("Select All"), contextMenu);
    actionSelectAll.setIcon(QIcon(":/icons/menu/select_all_32x32.png"));
    actionSelectAll.setShortcut(QKeySequence::SelectAll);
    connect(&actionSelectAll, SIGNAL(triggered()), this, SLOT(selectAll()));

    QAction actionSelectFiltered(tr("Select Filtered"), contextMenu);
    actionSelectFiltered.setIcon(QIcon(":/icons/menu/select_completed_32x32.png"));
    actionSelectFiltered.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
    connect(&actionSelectFiltered, SIGNAL(triggered()), this, SLOT(selectFiltered()));

    QAction actionInvertSelection(tr("Invert Selection "), contextMenu);
    actionInvertSelection.setIcon(QIcon(":/icons/menu/select_invert_32x32.png"));
    actionInvertSelection.setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
    connect(&actionInvertSelection, SIGNAL(triggered()), this, SLOT(invertSelection()));
    // --
    QAction actionCopyLinks(tr("Copy Links"), contextMenu);
    actionCopyLinks.setShortcut(QKeySequence::Copy);
    connect(&actionCopyLinks, SIGNAL(triggered()), this, SLOT(copyLinks()));
    // --
    QAction actionOpen(textForOpenAction(), contextMenu);
    actionOpen.setIcon(QIcon(":/icons/menu/icon_open_file_16x16.png"));
    actionOpen.setShortcut(QKeySequence::Open);
    connect(&actionOpen, SIGNAL(triggered()), this, SLOT(open()));

    contextMenu->addAction(&actionCheckSelected);
    contextMenu->addAction(&actionUncheckSelected);
    contextMenu->addAction(&actionToggleCheck);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionMask);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionSelectAll);
    contextMenu->addAction(&actionSelectFiltered);
    contextMenu->addAction(&actionInvertSelection);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionCopyLinks);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionOpen);

    contextMenu->exec(QCursor::pos());
    contextMenu->deleteLater();
}

/******************************************************************************
 ******************************************************************************/
void LinkWidget::checkSelected()
{
    foreach (auto index, selectedIndexesAtColumn(0)) {
        QAbstractItemModel *model = const_cast<QAbstractItemModel*>(index.model());
        model->setData(index, true, Qt::UserRole);
    }
}

void LinkWidget::uncheckSelected()
{
    foreach (auto index, selectedIndexesAtColumn(0)) {
        QAbstractItemModel *model = const_cast<QAbstractItemModel*>(index.model());
        model->setData(index, false, Qt::UserRole);
    }
}

void LinkWidget::toggleCheck()
{
    foreach (auto index, selectedIndexesAtColumn(0)) {
        const bool selected = index.model()->data(index, Qt::UserRole).toBool();
        QAbstractItemModel *model = const_cast<QAbstractItemModel*>(index.model());
        model->setData(index, !selected, Qt::UserRole);
    }
}

void LinkWidget::customizeMask()
{
    /// \todo
}

void LinkWidget::selectAll()
{
    currentTableView()->selectAll();
}

void LinkWidget::selectFiltered()
{
    currentTableView()->selectionModel()->clearSelection();

    const int rowCount = currentTableView()->model()->rowCount();
    const int colCount = currentTableView()->model()->columnCount();
    for (int i = 0; i < rowCount; ++i) {

        const QModelIndex &index = currentTableView()->model()->index(i, 0);
        const bool selected = index.model()->data(index, Qt::UserRole).toBool();

        if (selected) {
            for (int j = 0; j < colCount; ++j) {
                const QModelIndex &selectedIndex = currentTableView()->model()->index(i, j);
                currentTableView()->selectionModel()->select(selectedIndex, QItemSelectionModel::Select);
            }
        }
    }
}

void LinkWidget::invertSelection()
{
    const int rowCount = currentTableView()->model()->rowCount();
    const int colCount = currentTableView()->model()->columnCount();
    for (int i = 0; i < rowCount; ++i) {
        for (int j = 0; j < colCount; ++j) {
            const QModelIndex &index = currentTableView()->model()->index(i, j);
            currentTableView()->selectionModel()->select(index, QItemSelectionModel::Toggle);
        }
    }
}

void LinkWidget::copyLinks()
{
    QString input;
    foreach (auto index, selectedIndexesAtColumn(1)) {
        const QString text = index.model()->data(index, Qt::DisplayRole).toString();
        input.append(text);
        input.append('\n');
    }
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(input);
}

void LinkWidget::open()
{
    foreach (auto index, selectedIndexesAtColumn(1)) {
        const QString text = index.model()->data(index, Qt::DisplayRole).toString();
        QUrl url(text);
        QDesktopServices::openUrl(url);
    }
}

/******************************************************************************
 ******************************************************************************/
inline QString LinkWidget::textForOpenAction() const
{
    const QModelIndexList indexes = currentTableView()->selectionModel()->selectedIndexes();
    QModelIndexList urlIndexes;
    foreach (auto index, indexes) {
        if (index.column() == 1) {
            urlIndexes.append(index);
        }
    }
    if (urlIndexes.count() == 0) {
        return tr("-");

    } else if (urlIndexes.count() == 1) {
        const QModelIndex urlIndex = urlIndexes.first();
        const QString text = urlIndex.model()->data(urlIndex, Qt::DisplayRole).toString();
        return tr("Open %0").arg(text);
    } else {
        return tr("Open %0 Links").arg(urlIndexes.count());
    }
}

inline QModelIndexList LinkWidget::selectedIndexesAtColumn(int column)
{
    QModelIndexList indexes;
    foreach (auto index, currentTableView()->selectionModel()->selectedIndexes()) {
        if (index.column() == column) {
            indexes.append(index);
        }
    }
    return indexes;
}

inline QTableView* LinkWidget::currentTableView() const
{
    if (ui->tabWidget->currentIndex() == 0) {
        return ui->linkTableView;
    } else {
        return ui->contentTableView;
    }
}

#include "linkwidget.moc"
