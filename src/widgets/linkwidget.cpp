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

#include <Core/CheckableTableModel>
#include <Core/MimeDatabase>
#include <Core/Model>
#include <Core/ResourceItem>
#include <Core/ResourceModel>
#include <Widgets/CheckableItemDelegate>
#include <Widgets/CheckableTableView>

#include <QtCore/QDebug>
#include <QtCore/QModelIndex>
#include <QtGui/QClipboard>
#include <QtGui/QDesktopServices>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>

#define C_ELIDE_CHAR_COUNT         30


/******************************************************************************
 ******************************************************************************/
/*!
 * LinkWidgetItemDelegate is used to draw the mimetype icon.
 */
class LinkWidgetItemDelegate : public CheckableItemDelegate
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
    explicit LinkWidgetItemDelegate(QObject *parent = Q_NULLPTR)
        : CheckableItemDelegate(parent)
    {}

    ~LinkWidgetItemDelegate() Q_DECL_OVERRIDE {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;
};

void LinkWidgetItemDelegate::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    initStyleOption(&myOption, index);

    if (index.column() == 1) {
        const QUrl url(myOption.text);
        const QPixmap pixmap = MimeDatabase::fileIcon(url, thumbnailHint());
        myOption.icon.addPixmap(pixmap);
        myOption.decorationAlignment = Qt::AlignHCenter |Qt::AlignVCenter;
        myOption.decorationPosition = QStyleOptionViewItem::Left;
        myOption.features |= QStyleOptionViewItem::HasDecoration;
    }
    CheckableItemDelegate::paint(painter, myOption, index);
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
    if (event->modifiers() & Qt::ShiftModifier)    modifier += "Shift+";
    if (event->modifiers() & Qt::ControlModifier)  modifier += "Ctrl+";
    if (event->modifiers() & Qt::AltModifier)      modifier += "Alt+";
    if (event->modifiers() & Qt::MetaModifier)     modifier += "Meta+";
    key = QKeySequence(event->key()).toString();
    return QKeySequence(modifier + key);
}

void LinkWidget::keyPressEvent(QKeyEvent *event)
{
    QKeySequence sequence(toKeySequence(event));
    auto view = currentTableView();
    if (sequence == QKeySequence(QKeySequence::SelectAll)) {      view->selectAll();
    } else if (sequence == QKeySequence(Qt::CTRL + Qt::Key_F)) {  view->selectFiltered();
    } else if (sequence == QKeySequence(Qt::CTRL + Qt::Key_I)) {  view->invertSelection();
    } else if (sequence == QKeySequence(QKeySequence::Copy)) {    copyLinks();
    } else if (sequence == QKeySequence(QKeySequence::Open)) {    open();
    } else {
        QWidget::keyPressEvent(event); // important!
    }
}

/******************************************************************************
 ******************************************************************************/
void LinkWidget::setup(CheckableTableView *view)
{
    using namespace std::placeholders; // adds visibility of _1, _2, ...

    auto fp = std::bind(&LinkWidget::contextMenuCallback, this, _1);
    view->setContextMenuCallback(fp);

    view->setItemDelegate(new LinkWidgetItemDelegate(view));

    connect(view->horizontalHeader(), SIGNAL(sectionResized(int,int,int)),
            this, SLOT(onSectionResized(int,int,int)));
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

void LinkWidget::resizeSection(CheckableTableView *view, int logicalIndex, int newSize)
{
    QHeaderView *header = view->horizontalHeader();
    QSignalBlocker blocker(header);
    header->resizeSection(logicalIndex, newSize);
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
    /*
     * Note: ui->linkTableView and ui->contentTableView are synchronized
     */
    return ui->linkTableView->columnWidths();
}

void LinkWidget::setColumnWidths(const QList<int> &widths)
{
    /*
     * Note: ui->linkTableView and ui->contentTableView are synchronized
     */
    ui->linkTableView->setColumnWidths(widths);
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
void LinkWidget::contextMenuCallback(QMenu *contextMenu)
{
    if (!contextMenu) {
        return;
    }
    QAction *actionMask = new QAction(tr("Mask..."), contextMenu);
    actionMask->setIcon(QIcon(":/icons/menu/icon_mask_16x16.png"));
    connect(actionMask, SIGNAL(triggered()), this, SLOT(customizeMask()));

    QAction *actionCopyLinks = new QAction(tr("Copy Links"), contextMenu);
    actionCopyLinks->setShortcut(QKeySequence::Copy);
    connect(actionCopyLinks, SIGNAL(triggered()), this, SLOT(copyLinks()));

    QAction *actionOpen = new QAction(textForOpenAction(), contextMenu);
    actionOpen->setIcon(QIcon(":/icons/menu/icon_open_file_16x16.png"));
    actionOpen->setShortcut(QKeySequence::Open);
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));

    contextMenu->addSeparator();
    contextMenu->addAction(actionMask);
    contextMenu->addSeparator();
    contextMenu->addAction(actionCopyLinks);
    contextMenu->addSeparator();
    contextMenu->addAction(actionOpen);
}

void LinkWidget::customizeMask()
{
    /// \todo
}

void LinkWidget::copyLinks()
{
    QString input;
    foreach (auto index, currentTableView()->selectedIndexesAtColumn(1)) {
        const QString text = index.model()->data(index, Qt::DisplayRole).toString();
        input.append(text);
        input.append('\n');
    }
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(input);
}

void LinkWidget::open()
{
    foreach (auto index, currentTableView()->selectedIndexesAtColumn(1)) {
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

inline CheckableTableView* LinkWidget::currentTableView() const
{
    return (ui->tabWidget->currentIndex() == 0) ? ui->linkTableView : ui->contentTableView;
}

#include "linkwidget.moc"
