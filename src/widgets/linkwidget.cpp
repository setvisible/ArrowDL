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

#include "linkwidget.h"
#include "ui_linkwidget.h"

#include <Core/MimeDatabase>
#include <Core/Model>
#include <Core/ResourceItem>
#include <Core/ResourceModel>

#include <QtCore/QDebug>
#include <QtCore/QModelIndex>
#include <QtGui/QClipboard>
#include <QtGui/QDesktopServices>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtWidgets/QAction>
#include <QtWidgets/QItemDelegate>
#include <QtWidgets/QMenu>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QTableView>


#define C_COLUMN_DEFAULT_WIDTH    100
#define C_COLUMN_MAX_WIDTH       1000

#define C_CHECKBOX_SIZE            12
#define C_CHECKBOX_WIDTH           16

#define C_ELIDE_CHAR_COUNT         30

static const QColor s_black         = QColor(0, 0, 0);
static const QColor s_darkYellow    = QColor(210, 210, 100);
static const QColor s_lightBlue     = QColor(205, 232, 255);
static const QColor s_lightGreen    = QColor(236, 255, 179);
static const QColor s_lightYellow   = QColor(255, 255, 179);


/*!
 * LinkWidgetItemDelegate is used to draw the check icons.
 */
class LinkWidgetItemDelegate : public QStyledItemDelegate
{
    /*
     * Remark:
     * If use Q_OBJECT, signals and slots in nested classes, add
     * <code>
     * #include "linkwidget.moc"
     * </code>
     * at this file's end.
     */
    Q_OBJECT

public:
    inline LinkWidgetItemDelegate(QObject *parent = Q_NULLPTR);

    ~LinkWidgetItemDelegate() Q_DECL_OVERRIDE {}

    // painting
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

    // editing
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) Q_DECL_OVERRIDE;
private:
    QIcon m_checkIcon;
};

LinkWidgetItemDelegate::LinkWidgetItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    m_checkIcon.addPixmap(QPixmap(":/icons/menu/check_ok_16x16.png"), QIcon::Normal, QIcon::On);
    m_checkIcon.addPixmap(QPixmap(":/icons/menu/check_progress_16x16.png"), QIcon::Disabled, QIcon::On);
}

void LinkWidgetItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    initStyleOption(&myOption, index);

    const bool selected = index.model()->data(index, ResourceModel::IsSelectedRole).toBool();

    if (selected) {
        painter->fillRect(option.rect, s_lightYellow);
        myOption.palette.setColor(QPalette::All, QPalette::Highlight, s_darkYellow);
    } else {
        myOption.palette.setColor(QPalette::All, QPalette::Highlight, s_lightBlue);
    }

    if (myOption.state & QStyle::State_Selected) {
        myOption.font.setBold(true);
    }

    myOption.palette.setColor(QPalette::All, QPalette::HighlightedText, s_black);

    if (index.column() == 0) {
        QStyleOptionButton button;
        button.rect = myOption.rect;
        button.palette = myOption.palette;
        button.iconSize = QSize(C_CHECKBOX_SIZE, C_CHECKBOX_SIZE);
        button.icon = m_checkIcon;
        button.features |= QStyleOptionButton::Flat;
        button.state |= selected ? QStyle::State_Enabled : QStyle::State_None;
        QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);

    } else {
        if (index.column() == 1) {
            const QUrl url(myOption.text);
            const QPixmap pixmap = MimeDatabase::fileIcon(url, 16);

            myOption.icon.addPixmap(pixmap);
            myOption.decorationAlignment = Qt::AlignHCenter |Qt::AlignVCenter;
            myOption.decorationPosition = QStyleOptionViewItem::Left;
            myOption.features |= QStyleOptionViewItem::HasDecoration;
        }
        QStyledItemDelegate::paint(painter, myOption, index);
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
        const bool selected = index.model()->data(index, ResourceModel::IsSelectedRole).toBool();
        model->setData(index, !selected, ResourceModel::IsSelectedRole);
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
static inline QKeySequence toKeySequence(QKeyEvent *event)
{
    QString modifier;
    QString key;

    if (event->modifiers() & Qt::ShiftModifier)
        modifier += "Shift+";
    if (event->modifiers() & Qt::ControlModifier)
        modifier += "Ctrl+";
    if (event->modifiers() & Qt::AltModifier)
        modifier += "Alt+";
    if (event->modifiers() & Qt::MetaModifier)
        modifier += "Meta+";

    key = QKeySequence(event->key()).toString();

    return QKeySequence(modifier + key);
}

void LinkWidget::keyPressEvent(QKeyEvent *event)
{
    QKeySequence sequence(toKeySequence(event));
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

    view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);

    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->setAlternatingRowColors(false);
    view->setMidLineWidth(3);

    view->setItemDelegate(new LinkWidgetItemDelegate(view));
    QHeaderView *verticalHeader = view->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(22);
    verticalHeader->setVisible(false);

    QHeaderView *horizontalHeader = view->horizontalHeader();
    horizontalHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    horizontalHeader->setHighlightSections(false);

    connect(view->horizontalHeader(), SIGNAL(sectionCountChanged(int,int)),
            this, SLOT(onSectionCountChanged(int,int)));

    connect(view->horizontalHeader(), SIGNAL(sectionResized(int,int,int)),
            this, SLOT(onSectionResized(int,int,int)));

    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));
}

/******************************************************************************
 ******************************************************************************/
void LinkWidget::onSectionCountChanged(int /*oldCount*/, int newCount)
{
    auto header = qobject_cast<QHeaderView *>(sender());
    if (newCount > 0) {
        header->setSectionResizeMode(0, QHeaderView::Fixed);
        auto parent = qobject_cast<QTableView *>(header->parent());
        if (parent) {
            parent->setColumnWidth(0, C_CHECKBOX_WIDTH);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
/*!
 * \brief Synchronize the column resize event.
 * All the QTableView have the same column sizes.
 */
void LinkWidget::onSectionResized(int logicalIndex, int /*oldSize*/, int newSize)
{
    resizeSection(ui->linkTableView, logicalIndex, newSize);
    resizeSection(ui->contentTableView, logicalIndex, newSize);
}

void LinkWidget::resizeSection(QTableView *view, int logicalIndex, int newSize)
{
    QHeaderView *header = view->horizontalHeader();
    const bool isblocked =  header->blockSignals(true);
    header->resizeSection(logicalIndex, newSize);
    header->blockSignals(isblocked);
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
                ui->linkTableView->setColumnWidth(column, C_CHECKBOX_WIDTH);
            } else if (column > 0 && column < widths.count()) {
                int width = widths.at(column);
                if (width < 0 || width > C_COLUMN_MAX_WIDTH) {
                    width =  C_COLUMN_DEFAULT_WIDTH;
                }
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
    auto contextMenu = new QMenu(this);

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

    QAction actionInvertSelection(tr("Invert Selection"), contextMenu);
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
        auto model = const_cast<QAbstractItemModel*>(index.model());
        model->setData(index, true, ResourceModel::IsSelectedRole);
    }
}

void LinkWidget::uncheckSelected()
{
    foreach (auto index, selectedIndexesAtColumn(0)) {
        auto model = const_cast<QAbstractItemModel*>(index.model());
        model->setData(index, false, ResourceModel::IsSelectedRole);
    }
}

void LinkWidget::toggleCheck()
{
    foreach (auto index, selectedIndexesAtColumn(0)) {
        const bool selected = index.model()->data(index, ResourceModel::IsSelectedRole).toBool();
        auto model = const_cast<QAbstractItemModel*>(index.model());
        model->setData(index, !selected, ResourceModel::IsSelectedRole);
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
        const bool selected = index.model()->data(index, ResourceModel::IsSelectedRole).toBool();

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
static inline QString elide(const QString &text)
{
    if (text.length() > 2 * C_ELIDE_CHAR_COUNT) {
        return QString("%0...%1").arg(text.left(C_ELIDE_CHAR_COUNT)).arg(text.right(C_ELIDE_CHAR_COUNT));
    }
    return text;
}

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
        return QLatin1String("-");
    }
    if (urlIndexes.count() == 1) {
        const QModelIndex urlIndex = urlIndexes.first();
        const QString text = urlIndex.model()->data(urlIndex, Qt::DisplayRole).toString();
        return tr("Open %0").arg(elide(text));
    }
    return tr("Open %0 Links").arg(urlIndexes.count());
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
    }
    return ui->contentTableView;
}

#include "linkwidget.moc"
