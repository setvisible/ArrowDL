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

#include "downloadqueueview_p.h"

#include <Constants>
#include <Core/AbstractDownloadItem>
#include <Core/DownloadManager>
#include <Core/Format>
#include <Core/MimeDatabase>
#include <Widgets/CustomStyle>
#include <Widgets/CustomStyleOptionProgressBar>
#include <Widgets/Globals>

#include <QtCore/QDebug>
#include <QtCore/QMimeData>
#include <QtCore/QFileInfo>
#include <QtGui/QDrag>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>
#include <QtWidgets/QStyledItemDelegate>


QueueView::QueueView(QWidget *parent)
    : QTreeWidget(parent)
{
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // To enable the user to move the items around within the view,
    // we must set the list widget's dragDropMode:
    setDragDropMode(QAbstractItemView::DragOnly);
}

void QueueView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->pos();
    }
    QTreeWidget::mousePressEvent(event);
}

void QueueView::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }
    if ((event->pos() - dragStartPosition).manhattanLength()
            < QApplication::startDragDistance()) {
        return;
    }
    auto queueItems = toQueueItem(selectedItems());

    QPixmap pixmap;
    QList<QUrl> urls;
    for (auto queueItem : queueItems) {
        auto url = urlFrom(queueItem);
        if (!url.isEmpty()) {
            if (pixmap.isNull()) {
                pixmap = MimeDatabase::fileIcon(url);
            }
            urls << url;
        }
    }
    if (urls.isEmpty())
        return;

    auto drag = new QDrag(this);
    auto mimeData = new QMimeData;
    mimeData->setUrls(urls);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()/2));

    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
    if (dropAction == Qt::MoveAction) {
        for (auto queueItem : queueItems) {
            emit dropped(queueItem);
        }
    }
}

QList<QueueItem*> QueueView::toQueueItem(const QList<QTreeWidgetItem *> &items) const
{
    QList<QueueItem*> queueItems;
    for (auto item : items) {
        auto queueItem = dynamic_cast<QueueItem*>(item);
        if (queueItem)
            queueItems << queueItem;
    }
    return queueItems;
}

QUrl QueueView::urlFrom(const QueueItem *queueItem) const
{
    if (!queueItem)
        return {};

    const AbstractDownloadItem* downloadItem = queueItem->downloadItem();
    if (!downloadItem)
        return {};

    const QFileInfo fi(downloadItem->localFullFileName());
    if (!fi.exists())
        return {};

    return QUrl::fromLocalFile(downloadItem->localFullFileName());
}

/******************************************************************************
 ******************************************************************************/
/*!
 * QueueViewItemDelegate is used to draw the progress bars and edit the filename.
 */
class QueueViewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit QueueViewItemDelegate(QObject *parent = nullptr);

    // painting
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // editing
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void restylizeUi();

private:
    QIcon m_idleIcon = {};
    QIcon m_resumeIcon = {};
    QIcon m_pauseIcon = {};
    QIcon m_stopIcon = {};
    QIcon m_completedIcon = {};

    QColor stateColor(AbstractDownloadItem::State state) const;
    QIcon stateIcon(AbstractDownloadItem::State state) const;
};

QueueViewItemDelegate::QueueViewItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    restylizeUi();
}

void QueueViewItemDelegate::restylizeUi()
{
    m_idleIcon = {};
    m_resumeIcon = {};
    m_pauseIcon = {};
    m_stopIcon = {};
    m_completedIcon = {};

    m_idleIcon.addPixmap(QIcon::fromTheme("queue-idle").pixmap(16), QIcon::Normal, QIcon::On);
    m_resumeIcon.addPixmap(QIcon::fromTheme("queue-play").pixmap(16), QIcon::Normal, QIcon::On);
    m_pauseIcon.addPixmap(QIcon::fromTheme("queue-paused").pixmap(16), QIcon::Normal, QIcon::On);
    m_stopIcon.addPixmap(QIcon::fromTheme("queue-stop").pixmap(16), QIcon::Normal, QIcon::On);
    m_completedIcon.addPixmap(QIcon::fromTheme("queue-completed").pixmap(16), QIcon::Normal, QIcon::On);
}

void QueueViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    QStyleOptionViewItem myOption = option;
    initStyleOption(&myOption, index);

    if (myOption.state & QStyle::State_Selected) {
        myOption.font.setBold(true);
    }

    auto palette = qApp->palette();
    myOption.palette.setColor(QPalette::All, QPalette::Window, palette.color(QPalette::Base));
    myOption.palette.setColor(QPalette::All, QPalette::WindowText, palette.color(QPalette::WindowText));
    myOption.palette.setColor(QPalette::All, QPalette::Highlight, palette.color(QPalette::Highlight));
    myOption.palette.setColor(QPalette::All, QPalette::HighlightedText, palette.color(QPalette::HighlightedText));

    if (index.column() == COL_0_FILE_NAME) {

        const QUrl url(myOption.text);
        auto pixmap = MimeDatabase::fileIcon(url, 16);

        myOption.icon.addPixmap(pixmap);
        myOption.decorationAlignment = Qt::AlignHCenter | Qt::AlignVCenter;
        myOption.decorationPosition = QStyleOptionViewItem::Left;
        myOption.features = myOption.features | QStyleOptionViewItem::HasDecoration;

        QStyledItemDelegate::paint(painter, myOption, index);

    } else if (index.column() == COL_2_PROGRESS_BAR) {

        auto progress = index.data(QueueItem::ProgressRole).toInt();
        auto state = static_cast<AbstractDownloadItem::State>(index.data(QueueItem::StateRole).toInt());

        CustomStyleOptionProgressBar progressBarOption;
        progressBarOption.state = myOption.state;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = myOption.rect;
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = MIN_PROGRESS;
        progressBarOption.maximum = MAX_PROGRESS;
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.textVisible = false;
        progressBarOption.palette = myOption.palette;
        progressBarOption.progress = progress;
        progressBarOption.color = stateColor(state);
        progressBarOption.icon = stateIcon(state);

        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    } else {
        QStyledItemDelegate::paint(painter, myOption, index);
    }
}

QWidget* QueueViewItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    if (!index.isValid())
        return nullptr;

    if (index.column() != COL_0_FILE_NAME)
        return nullptr;

    auto editor = new QLineEdit(parent);
    editor->setAutoFillBackground(true);
    editor->setFocusPolicy(Qt::StrongFocus);
    return editor;
}

void QueueViewItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) {
        QString text = index.data(Qt::EditRole).toString();
        lineEdit->setText(text);
    }
}

void QueueViewItemDelegate::updateEditorGeometry(
    QWidget *editor,
    const QStyleOptionViewItem &option,
    const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect);
}

QColor QueueViewItemDelegate::stateColor(AbstractDownloadItem::State state) const
{
    switch (state) {
    case AbstractDownloadItem::Idle:
        return s_darkGrey;

    case AbstractDownloadItem::Paused:
        return s_orange;

    case AbstractDownloadItem::Preparing:
    case AbstractDownloadItem::Connecting:
    case AbstractDownloadItem::DownloadingMetadata:
    case AbstractDownloadItem::Downloading:
    case AbstractDownloadItem::Endgame:
        return s_green;

    case AbstractDownloadItem::Completed:
    case AbstractDownloadItem::Seeding:
        return s_darkGreen;

    case AbstractDownloadItem::Stopped:
    case AbstractDownloadItem::Skipped:
    case AbstractDownloadItem::NetworkError:
    case AbstractDownloadItem::FileError:
        return s_darkRed;

    default:
        Q_UNREACHABLE();
        break;
    }
    return Qt::black;
}

QIcon QueueViewItemDelegate::stateIcon(AbstractDownloadItem::State state) const
{
    switch (state) {
    case AbstractDownloadItem::Idle:
        return m_idleIcon;

    case AbstractDownloadItem::Paused:
        return m_pauseIcon;

    case AbstractDownloadItem::Preparing:
    case AbstractDownloadItem::Connecting:
    case AbstractDownloadItem::DownloadingMetadata:
    case AbstractDownloadItem::Downloading:
    case AbstractDownloadItem::Endgame:
        return m_resumeIcon;

    case AbstractDownloadItem::Completed:
    case AbstractDownloadItem::Seeding:
        return m_completedIcon;

    case AbstractDownloadItem::Stopped:
    case AbstractDownloadItem::Skipped:
    case AbstractDownloadItem::NetworkError:
    case AbstractDownloadItem::FileError:
        return m_stopIcon;

    default:
        Q_UNREACHABLE();
        break;
    }
    return {};
}

/******************************************************************************
 ******************************************************************************/
QueueItem::QueueItem(AbstractDownloadItem *downloadItem, QTreeWidget *view)
    : QObject(view)
    , QTreeWidgetItem(view, QTreeWidgetItem::UserType)
    , m_downloadItem(downloadItem)
{
    this->setSizeHint(COL_2_PROGRESS_BAR, QSize(COLUMN_DEFAULT_WIDTH, ROW_DEFAULT_HEIGHT));
    this->setFlags(Qt::ItemIsEditable | flags());

    connect(m_downloadItem, SIGNAL(changed()), this, SLOT(updateItem()));

    updateItem();
}

static QString estimatedTime(AbstractDownloadItem *downloadItem)
{
    switch (downloadItem->state()) {
    case AbstractDownloadItem::Downloading:
        return Format::timeToString(downloadItem->remainingTime());
    case AbstractDownloadItem::NetworkError:
    case AbstractDownloadItem::FileError:
        return downloadItem->errorMessage();
    default:
        return downloadItem->stateToString();
    }
}

void QueueItem::updateItem()
{
    QString size;
    if (m_downloadItem->bytesTotal() > 0) {
        size = tr("%0 of %1").arg(
                    Format::fileSizeToString(m_downloadItem->bytesReceived()),
                    Format::fileSizeToString(m_downloadItem->bytesTotal()));
    } else {
        size = tr("Unknown");
    }

    QString speed = Format::currentSpeedToString(m_downloadItem->speed());

    this->setText(COL_0_FILE_NAME      , m_downloadItem->localFileName());
    this->setText(COL_1_WEBSITE_DOMAIN , m_downloadItem->sourceUrl().host()); /// \todo domain only
    this->setData(COL_2_PROGRESS_BAR   , StateRole, m_downloadItem->state());
    this->setData(COL_2_PROGRESS_BAR   , ProgressRole, m_downloadItem->progress());
    this->setText(COL_3_PERCENT        , QString("%0%").arg(qMax(0, m_downloadItem->progress())));
    this->setText(COL_4_SIZE           , size);
    this->setText(COL_5_ESTIMATED_TIME , estimatedTime(m_downloadItem));
    this->setText(COL_6_SPEED          , speed);

    //item->setText(C_COL_7_SEGMENTS, "Unknown");
    // todo etc...
}

/******************************************************************************
 ******************************************************************************/
DownloadQueueView::DownloadQueueView(QWidget *parent) : QWidget(parent)
  , m_queueView(new QueueView(this))
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    // Main queue list
    m_queueView->setItemDelegate(new QueueViewItemDelegate(this));
    m_queueView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_queueView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_queueView->setAlternatingRowColors(false);
    m_queueView->setRootIsDecorated(false);
    m_queueView->setMidLineWidth(3);

    setColumnWidths(QList<int>());

    // Edit with second click
    m_queueView->setEditTriggers(QAbstractItemView::SelectedClicked);

    connect(m_queueView, SIGNAL(itemSelectionChanged()), this, SLOT(onQueueViewItemSelectionChanged()));
    connect(m_queueView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onQueueViewDoubleClicked(QModelIndex)));

    connect(m_queueView->itemDelegate(), SIGNAL(commitData(QWidget*)), this, SLOT(onQueueItemCommitData(QWidget*)));

    // Drag-n-Drop
    connect(m_queueView, SIGNAL(dropped(QueueItem*)), this, SLOT(onQueueItemDropped(QueueItem*)));

    auto layout = new QGridLayout(this);
    layout->addWidget(m_queueView);
    layout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(layout);

    retranslateUi();
}

/******************************************************************************
 ******************************************************************************/
QSize DownloadQueueView::sizeHint() const
{
    auto header = m_queueView->header();

    // Add up the sizes of all header sections. The last section is
    // stretched, so its size is relative to the size of the width;
    // instead of counting it, we count the size of its largest value.
    int width = 200;
    // int width = fontMetrics().horizontalAdvance(tr("Downloading") + "  ");
    for (auto i = 0; i < header->count() - 1; ++i) {
        width += header->sectionSize(i);
    }

    return QSize(width, QWidget::sizeHint().height());
}

/******************************************************************************
 ******************************************************************************/
QByteArray DownloadQueueView::saveState(int version) const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << VERSION_MARKER;
    stream << version;
    stream << columnWidths();
    return data;
}

bool DownloadQueueView::restoreState(const QByteArray &state, int version)
{
    if (state.isEmpty()) {
        return false;
    }
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    int marker;
    int v;
    stream >> marker;
    stream >> v;
    if (stream.status() != QDataStream::Ok || marker != VERSION_MARKER || v != version) {
        return false;
    }
    QList<int> widths;
    stream >> widths;
    setColumnWidths(widths);
    bool restored = true;
    return restored;
}

/******************************************************************************
 ******************************************************************************/
QList<int> DownloadQueueView::columnWidths() const
{
    QList<int> widths;
    for (int column = 0, count = m_queueView->columnCount(); column < count; ++column) {
        auto width = m_queueView->columnWidth(column);
        widths.append(width);
    }
    return widths;
}

static int defaultColumnWidth(int index)
{
    return index == 0 ? COLUMN_0_DEFAULT_WIDTH : COLUMN_DEFAULT_WIDTH;
}

void DownloadQueueView::setColumnWidths(const QList<int> &widths)
{
    for (int column = 0, count = m_queueView->columnCount(); column < count; ++column) {
        if (column < widths.count()) {
            auto width = widths.at(column);
            m_queueView->setColumnWidth(column, width);
        } else {
            m_queueView->setColumnWidth(column, defaultColumnWidth(column));
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::rename()
{
    if (!m_queueView->selectedItems().isEmpty()) {
        auto treeItem = m_queueView->selectedItems().first();
        m_queueView->setCurrentItem(treeItem, COL_0_FILE_NAME);
        m_queueView->editItem(treeItem, COL_0_FILE_NAME);
    }
}

/******************************************************************************
 ******************************************************************************/
DownloadManager *DownloadQueueView::engine() const
{
    return m_downloadManager;
}

void DownloadQueueView::setEngine(DownloadManager *downloadManager)
{
    struct Cx {
        const char *signal;
        const char *slot;
    };
    static const Cx connections[] = {
        { SIGNAL(jobAppended(DownloadRange)),
          SLOT(onJobAdded(DownloadRange)) },
        { SIGNAL(jobRemoved(DownloadRange)),
          SLOT(onJobRemoved(DownloadRange)) },
        { SIGNAL(jobStateChanged(AbstractDownloadItem*)),
          SLOT(onJobStateChanged(AbstractDownloadItem*)) },
        { SIGNAL(selectionChanged()),
          SLOT(onSelectionChanged()) },
        { SIGNAL(sortChanged()),
          SLOT(onSortChanged()) },
        { 0, 0 }
    };

    if (m_downloadManager == downloadManager) {
        return;
    }

    if (m_downloadManager) {
        for (auto cx = &connections[0]; cx->signal; cx++) {
            QObject::disconnect(m_downloadManager, cx->signal, this, cx->slot);
        }
    }
    m_downloadManager = downloadManager;
    if (m_downloadManager) {
        for (auto cx = &connections[0]; cx->signal; cx++) {
            QObject::connect(m_downloadManager, cx->signal, this, cx->slot);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    } else if (event->type() == QEvent::StyleChange) {
        restylizeUi();
    }
    QWidget::changeEvent(event);
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::retranslateUi()
{
    Q_ASSERT(m_queueView);
    QStringList headers;
    headers << tr("Download/Name")
            << tr("Domain")
            << tr("Progress")
            << tr("Percent")
            << tr("Size")
            << tr("Est. time")      /* Hidden by default */
            << tr("Speed")          /* Hidden by default */
               ;
    m_queueView->setHeaderLabels(headers);

    for (auto index = 0; index < m_queueView->topLevelItemCount(); ++index) {
        auto treeItem = m_queueView->topLevelItem(index);
        auto queueItem = dynamic_cast<QueueItem *>(treeItem);
        if (queueItem) {
            queueItem->updateItem();
        }
    }
}

void DownloadQueueView::restylizeUi()
{
    auto itemDelegate = static_cast<QueueViewItemDelegate*>(m_queueView->itemDelegate());
    if (itemDelegate) {
        itemDelegate->restylizeUi();
        m_queueView->update();
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::onJobAdded(const DownloadRange &range)
{
    for (auto item : range) {
        auto downloadItem = dynamic_cast<AbstractDownloadItem*>(item);
        auto queueItem = new QueueItem(downloadItem, m_queueView);
        m_queueView->addTopLevelItem(queueItem);
    }
}

void DownloadQueueView::onJobRemoved(const DownloadRange &range)
{
    for (auto item : range) {
        auto index = getIndex(item);
        if (index >= 0) {
            auto treeItem = m_queueView->takeTopLevelItem(index);
            auto queueItem = dynamic_cast<QueueItem*>(treeItem);
            Q_ASSERT(queueItem);
            if (queueItem) {
                queueItem->deleteLater();
            }
        }
    }
}

void DownloadQueueView::onJobStateChanged(AbstractDownloadItem *item)
{
    auto queueItem = getQueueItem(item);
    if (queueItem) {
        queueItem->updateItem();
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::onSelectionChanged()
{
    const QSignalBlocker blocker(m_downloadManager);
    m_downloadManager->beginSelectionChange();

    auto selection = m_downloadManager->selection();
    for (auto index = 0; index < m_queueView->topLevelItemCount(); ++index) {
        auto treeItem = m_queueView->topLevelItem(index);
        auto queueItem = dynamic_cast<const QueueItem *>(treeItem);
        auto isSelected = selection.contains(queueItem->downloadItem());
        treeItem->setSelected(isSelected);
    }

    m_downloadManager->endSelectionChange();
}

void DownloadQueueView::onSortChanged()
{
    // Save selection and current item
    auto currentItem = m_queueView->currentItem();
    auto selection = m_downloadManager->selection();

    auto items = m_downloadManager->downloadItems();
    for (auto i = 0; i < items.size(); ++i) {
        auto downloadItem = items.at(i);
        auto index = getIndex(downloadItem);
        if (index != -1) {
            // Rem: takeTopLevelItem() changes the selection
            auto treeItem =  m_queueView->takeTopLevelItem(index);
            if (treeItem) {
                m_queueView->insertTopLevelItem(static_cast<int>(i), treeItem);
            }
        }
    }
    // Restore selection and current item
    m_queueView->setCurrentItem(currentItem);
    m_downloadManager->setSelection(selection);
}

/******************************************************************************
 ******************************************************************************/
int DownloadQueueView::getIndex(AbstractDownloadItem *downloadItem) const
{
    for (auto index = 0; index < m_queueView->topLevelItemCount(); ++index) {
        auto treeItem = m_queueView->topLevelItem(index);
        if (treeItem->type() == QTreeWidgetItem::UserType) {
            auto queueItem = dynamic_cast<const QueueItem *>(treeItem);
            if (queueItem && downloadItem && queueItem->downloadItem() == downloadItem) {
                return index;
            }
        }
    }
    return -1;
}

QueueItem* DownloadQueueView::getQueueItem(AbstractDownloadItem *downloadItem)
{
    auto index = getIndex(downloadItem);
    if (index >= 0) {
        auto treeItem = m_queueView->topLevelItem(index);
        if (treeItem->type() == QTreeWidgetItem::UserType) {
            auto queueItem = dynamic_cast<QueueItem *>(treeItem);
            if (queueItem && queueItem->downloadItem() == downloadItem) {
                return queueItem;
            }
        }
    }
    return nullptr;
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::onQueueViewDoubleClicked(const QModelIndex &index)
{
    auto treeItem = m_queueView->itemFromIndex(index);
    auto queueItem = dynamic_cast<const QueueItem *>(treeItem);
    emit doubleClicked(queueItem->downloadItem());
}

/*!
 * Synchronize with the selection in the Engine.
 */
void DownloadQueueView::onQueueViewItemSelectionChanged()
{
    QList<AbstractDownloadItem *> selection;
    for (auto treeItem : m_queueView->selectedItems()) {
        auto queueItem = dynamic_cast<const QueueItem *>(treeItem);
        selection << queueItem->downloadItem();
    }
    m_downloadManager->setSelection(selection);
}

/*!
 * Update the Engine data.
 */
void DownloadQueueView::onQueueItemCommitData(QWidget *editor)
{
    auto lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) {
        auto newName = lineEdit->text();

        // remove extension from base name
        auto pos = newName.lastIndexOf('.');
        if (pos != -1) {
            newName = newName.left(pos);
        }

        auto treeItem = m_queueView->currentItem();
        auto queueItem = dynamic_cast<QueueItem *>(treeItem);
        auto downloadItem = queueItem->downloadItem();

        downloadItem->rename(newName);
        queueItem->updateItem();
    }
}

void DownloadQueueView::onQueueItemDropped(QueueItem *queueItem)
{
    if (queueItem) {
        QList<AbstractDownloadItem*> items;
        items << queueItem->downloadItem();
        m_downloadManager->remove(items);
    }
}

/******************************************************************************
 ******************************************************************************/
QMenu* DownloadQueueView::contextMenu() const
{
    return m_contextMenu;
}

void DownloadQueueView::setContextMenu(QMenu *contextMenu)
{
    m_contextMenu = contextMenu;
}

void DownloadQueueView::showContextMenu(const QPoint &pos)
{
    if (m_contextMenu) {
        m_contextMenu->exec(mapToGlobal(pos));
    }
}

/* Required to build the nested class QueueViewItemDelegate */
#include "downloadqueueview.moc"
