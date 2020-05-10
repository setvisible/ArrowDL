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

#include "downloadqueueview_p.h"

#include <Core/AbstractDownloadItem>
#include <Core/DownloadEngine>
#include <Core/Format>
#include <Core/MimeDatabase>
#include <Widgets/CustomStyle>
#include <Widgets/CustomStyleOptionProgressBar>

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


#define C_COL_0_FILE_NAME          0
#define C_COL_1_WEBSITE_DOMAIN     1
#define C_COL_2_PROGRESS_BAR       2
#define C_COL_3_PERCENT            3
#define C_COL_4_SIZE               4
#define C_COL_5_ESTIMATED_TIME     5
#define C_COL_6_SPEED              6
#define C_COL_7_SEGMENTS           7  /* hidden */
#define C_COL_8_MASK               8  /* hidden */
#define C_COL_9_SAVE_PATH          9  /* hidden */
#define C_COL_10_CHECKSUM         10  /* hidden */

#define C_COLUMN_DEFAULT_WIDTH   100
#define C_COLUMN_0_DEFAULT_WIDTH 300
#define C_ROW_DEFAULT_HEIGHT      22

/* Constant */
static const QColor s_black         = QColor(0, 0, 0);
static const QColor s_lightBlue     = QColor(205, 232, 255);
static const QColor s_darkGrey      = QColor(160, 160, 160);
static const QColor s_green         = QColor(170, 224, 97);
static const QColor s_darkGreen     = QColor(0, 143, 0);
static const QColor s_darkYellow    = QColor(255, 204, 0);
static const QColor s_darkRed       = QColor(177, 40, 1);

static inline QString stateToString(IDownloadItem::State state)
{
    QString stateString;
    switch (state) {
    case IDownloadItem::Idle:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Idle");
        break;
    case IDownloadItem::Paused:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Paused");
        break;
    case IDownloadItem::Stopped:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Canceled");
        break;
    case IDownloadItem::Preparing:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Preparing");
        break;
    case IDownloadItem::Connecting:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Connecting");
        break;
    case IDownloadItem::DownloadingMetadata:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Downloading Metadata");
        break;
    case IDownloadItem::Downloading:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Downloading");
        break;
    case IDownloadItem::Endgame:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Finishing");
        break;
    case IDownloadItem::Completed:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Complete");
        break;
    case IDownloadItem::Seeding:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Seeding");
        break;
    case IDownloadItem::Skipped:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Skipped");
        break;
    case IDownloadItem::NetworkError:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Server error");
        break;
    case IDownloadItem::FileError:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "File error");
        break;
    default:
        Q_UNREACHABLE();
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "????");
        break;
    }
    return stateString ;
}

enum ProgressBar {
    StateRole = Qt::UserRole + 1,
    ProgressRole
};

/******************************************************************************
 ******************************************************************************/
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
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
            < QApplication::startDragDistance())
        return;

    const QList<QTreeWidgetItem*> items = selectedItems();
    if (items.isEmpty())
        return;

    auto queueItem = dynamic_cast<QueueItem*>(items.first());
    if (!queueItem)
        return;

    AbstractDownloadItem* downloadItem = queueItem->downloadItem();
    if (!downloadItem)
        return;

    QFileInfo fi(downloadItem->localFullFileName());
    if (!fi.exists())
        return;

    const QUrl url = QUrl::fromLocalFile(downloadItem->localFullFileName());
    const QPixmap pixmap = MimeDatabase::fileIcon(url);

    auto drag = new QDrag(this);
    auto mimeData = new QMimeData;
    mimeData->setUrls(QList<QUrl>() << url);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()/2));

    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
    if (dropAction == Qt::MoveAction) {
        emit dropped(queueItem);
    }
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
    explicit QueueViewItemDelegate(QObject *parent = Q_NULLPTR);

    // painting
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index ) const Q_DECL_OVERRIDE;

    // editing
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;

    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    QIcon m_idleIcon;
    QIcon m_resumeIcon;
    QIcon m_pauseIcon;
    QIcon m_stopIcon;
    QIcon m_completedIcon;

    QColor stateColor(IDownloadItem::State state) const;
    QIcon stateIcon(IDownloadItem::State state) const;
};

QueueViewItemDelegate::QueueViewItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    m_idleIcon.addPixmap(QPixmap(":/icons/menu/icon_idle_16x16.png"), QIcon::Normal, QIcon::On);
    m_resumeIcon.addPixmap(QPixmap(":/icons/menu/icon_resume_16x16.png"), QIcon::Normal, QIcon::On);
    m_pauseIcon.addPixmap(QPixmap(":/icons/menu/icon_pause_16x16.png"), QIcon::Normal, QIcon::On);
    m_stopIcon.addPixmap(QPixmap(":/icons/menu/icon_cancel_16x16.png"), QIcon::Normal, QIcon::On);
    m_completedIcon.addPixmap(QPixmap(":/icons/menu/icon_remove_completed_16x16.png"), QIcon::Normal, QIcon::On);

}

void QueueViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index ) const
{
    QStyleOptionViewItem myOption = option;
    initStyleOption(&myOption, index);

    if (myOption.state & QStyle::State_Selected) {
        myOption.font.setBold(true);
    }

    myOption.palette.setColor(QPalette::All, QPalette::Highlight, s_lightBlue);
    myOption.palette.setColor(QPalette::All, QPalette::HighlightedText, s_black);

    if (index.column() == C_COL_0_FILE_NAME) {

        const QUrl url(myOption.text);
        const QPixmap pixmap = MimeDatabase::fileIcon(url, 16);

        myOption.icon.addPixmap(pixmap);
        myOption.decorationAlignment = Qt::AlignHCenter |Qt::AlignVCenter;
        myOption.decorationPosition = QStyleOptionViewItem::Left;
        myOption.features = myOption.features | QStyleOptionViewItem::HasDecoration;

        QStyledItemDelegate::paint(painter, myOption, index);

    } else if (index.column() == C_COL_2_PROGRESS_BAR) {

        const int progress = index.data(ProgressBar::ProgressRole).toInt();
        auto state = static_cast<IDownloadItem::State>(index.data(ProgressBar::StateRole).toInt());

        CustomStyleOptionProgressBar progressBarOption;
        progressBarOption.state = QStyle::State_Enabled;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = myOption.rect;
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.textVisible = false;
        progressBarOption.palette.setColor(QPalette::All, QPalette::Highlight, s_lightBlue);
        progressBarOption.palette.setColor(QPalette::All, QPalette::HighlightedText, s_black);
        progressBarOption.progress = progress;
        progressBarOption.color = stateColor(state);
        progressBarOption.icon = stateIcon(state);

        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QWidget* QueueViewItemDelegate::createEditor(QWidget *parent,
                                             const QStyleOptionViewItem &/*option*/,
                                             const QModelIndex &index) const
{
    if (!index.isValid())
        return Q_NULLPTR;

    if (index.column() != C_COL_0_FILE_NAME)
        return Q_NULLPTR;

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

void QueueViewItemDelegate::updateEditorGeometry(QWidget *editor,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect);
}

QColor QueueViewItemDelegate::stateColor(IDownloadItem::State state) const
{
    switch (state) {
    case IDownloadItem::Idle:
        return s_darkGrey;

    case IDownloadItem::Paused:
        return s_darkYellow;

    case IDownloadItem::Preparing:
    case IDownloadItem::Connecting:
    case IDownloadItem::DownloadingMetadata:
    case IDownloadItem::Downloading:
    case IDownloadItem::Endgame:
        return s_green;

    case IDownloadItem::Completed:
    case IDownloadItem::Seeding:
        return s_darkGreen;

    case IDownloadItem::Stopped:
    case IDownloadItem::Skipped:
    case IDownloadItem::NetworkError:
    case IDownloadItem::FileError:
        return s_darkRed;

    default:
        Q_UNREACHABLE();
        break;
    }
    return Qt::black;
}

QIcon QueueViewItemDelegate::stateIcon(IDownloadItem::State state) const
{
    switch (state) {
    case IDownloadItem::Idle:
        return m_idleIcon;

    case IDownloadItem::Paused:
        return m_pauseIcon;

    case IDownloadItem::Preparing:
    case IDownloadItem::Connecting:
    case IDownloadItem::DownloadingMetadata:
    case IDownloadItem::Downloading:
    case IDownloadItem::Endgame:
        return m_resumeIcon;

    case IDownloadItem::Completed:
    case IDownloadItem::Seeding:
        return m_completedIcon;

    case IDownloadItem::Stopped:
    case IDownloadItem::Skipped:
    case IDownloadItem::NetworkError:
    case IDownloadItem::FileError:
        return m_stopIcon;

    default:
        Q_UNREACHABLE();
        break;
    }
    return QIcon();
}

/******************************************************************************
 ******************************************************************************/
QueueItem::QueueItem(AbstractDownloadItem *downloadItem, QTreeWidget *view)
    : QObject(view)
    , QTreeWidgetItem(view, QTreeWidgetItem::UserType)
    , m_downloadItem(downloadItem)
{
    this->setSizeHint(C_COL_2_PROGRESS_BAR, QSize(C_COLUMN_DEFAULT_WIDTH, C_ROW_DEFAULT_HEIGHT));
    this->setFlags(Qt::ItemIsEditable | flags());

    connect(m_downloadItem, SIGNAL(changed()), this, SLOT(updateItem()));

    updateItem();
}

void QueueItem::updateItem()
{
    QString size;
    if (m_downloadItem->bytesTotal() > 0) {
        size = tr("%0 of %1")
                .arg(Format::fileSizeToString(m_downloadItem->bytesReceived()))
                .arg(Format::fileSizeToString(m_downloadItem->bytesTotal()));
    } else {
        size = tr("Unknown");
    }

    QString estTime = stateToString(m_downloadItem->state());
    if (m_downloadItem->state() == IDownloadItem::NetworkError) {

        if (m_downloadItem->streamErrorMessage().isEmpty()) {

            /*
             * See QNetworkReply::NetworkError Documentation for conversion
             */
            int httpErrorNumber = m_downloadItem->httpErrorNumber();
            if (httpErrorNumber == 201) httpErrorNumber = 401;
            if (httpErrorNumber == 203) httpErrorNumber = 404;
            estTime += tr("(%0)").arg(httpErrorNumber);

        } else {
            estTime += tr("(%0)").arg(m_downloadItem->streamErrorMessage());
        }

    } else if (m_downloadItem->state() == IDownloadItem::Downloading) {
        estTime = Format::timeToString(m_downloadItem->remainingTime());
    }

    QString speed = Format::currentSpeedToString(m_downloadItem->speed());

    this->setText(C_COL_0_FILE_NAME       , m_downloadItem->localFileName());
    this->setText(C_COL_1_WEBSITE_DOMAIN  , m_downloadItem->sourceUrl().host()); // todo domain only

    this->setData(C_COL_2_PROGRESS_BAR, ProgressBar::StateRole, m_downloadItem->state());
    this->setData(C_COL_2_PROGRESS_BAR, ProgressBar::ProgressRole, m_downloadItem->progress());

    this->setText(C_COL_3_PERCENT         , QString("%0%").arg(qMax(0, m_downloadItem->progress())));
    this->setText(C_COL_4_SIZE            , size);
    this->setText(C_COL_5_ESTIMATED_TIME  , estTime);
    this->setText(C_COL_6_SPEED           , speed);

    //item->setText(C_COL_7_SEGMENTS, "Unknown");
    // todo etc...
}

/******************************************************************************
 ******************************************************************************/
DownloadQueueView::DownloadQueueView(QWidget *parent) : QWidget(parent)
  , m_downloadEngine(Q_NULLPTR)
  , m_contextMenu(Q_NULLPTR)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));

    // Initialize some static strings
    QStringList headers;
    headers << tr("Download/Name")
            << tr("Domain")
            << tr("Progress")
            << tr("Percent")
            << tr("Size")
            << tr("Est. time")      /* Hidden by default */
            << tr("Speed")          /* Hidden by default */
               ;

    // Main queue list
    m_queueView = new QueueView(this);
    m_queueView->setItemDelegate(new QueueViewItemDelegate(this));
    m_queueView->setHeaderLabels(headers);
    m_queueView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_queueView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_queueView->setAlternatingRowColors(false);
    m_queueView->setRootIsDecorated(false);
    m_queueView->setMidLineWidth(3);

    setColumnWidths(QList<int>());

    // Edit with second click
    m_queueView->setEditTriggers(QAbstractItemView::SelectedClicked);

    connect(m_queueView, SIGNAL(itemSelectionChanged()),
            this, SLOT(onQueueViewItemSelectionChanged()));
    connect(m_queueView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onQueueViewDoubleClicked(QModelIndex)));

    connect(m_queueView->itemDelegate(), SIGNAL(commitData(QWidget*)),
            this, SLOT(onQueueItemCommitData(QWidget*)));

    // Drag-n-Drop
    connect(m_queueView, SIGNAL(dropped(QueueItem*)),
            this, SLOT(onQueueItemDropped(QueueItem*)));

    QLayout* layout = new QGridLayout(this);
    layout->addWidget(m_queueView);
    layout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(layout);
}

DownloadQueueView::~DownloadQueueView()
{
}

/******************************************************************************
 ******************************************************************************/
QSize DownloadQueueView::sizeHint() const
{
    const QHeaderView *header = m_queueView->header();

    // Add up the sizes of all header sections. The last section is
    // stretched, so its size is relative to the size of the width;
    // instead of counting it, we count the size of its largest value.
    int width = 200;
    // int width = fontMetrics().horizontalAdvance(tr("Downloading") + "  ");
    for (int i = 0; i < header->count() - 1; ++i)
        width += header->sectionSize(i);

    return QSize(width, QWidget::sizeHint().height()).expandedTo(QApplication::globalStrut());
}

/******************************************************************************
 ******************************************************************************/
QByteArray DownloadQueueView::saveState(int version) const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << 0xff; // VersionMarker
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
    int marker, v;
    stream >> marker;
    stream >> v;
    if (stream.status() != QDataStream::Ok || marker != 0xff || v != version) {
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
    return index == 0 ? C_COLUMN_0_DEFAULT_WIDTH : C_COLUMN_DEFAULT_WIDTH;
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
        m_queueView->setCurrentItem(treeItem, C_COL_0_FILE_NAME);
        m_queueView->editItem(treeItem, C_COL_0_FILE_NAME);
    }
}

/******************************************************************************
 ******************************************************************************/
DownloadEngine *DownloadQueueView::engine() const
{
    return m_downloadEngine;
}

void DownloadQueueView::setEngine(DownloadEngine *downloadEngine)
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
        { SIGNAL(jobStateChanged(IDownloadItem*)),
          SLOT(onJobStateChanged(IDownloadItem*)) },
        { SIGNAL(selectionChanged()),
          SLOT(onSelectionChanged()) },
        { SIGNAL(sortChanged()),
          SLOT(onSortChanged()) },
        { 0, 0 }
    };

    if (m_downloadEngine == downloadEngine) {
        return;
    }

    if (m_downloadEngine) {
        for (const Cx *cx = &connections[0]; cx->signal; cx++) {
            QObject::disconnect(m_downloadEngine, cx->signal, this, cx->slot);
        }
    }
    m_downloadEngine = downloadEngine;
    if (m_downloadEngine) {
        for (const Cx *cx = &connections[0]; cx->signal; cx++) {
            QObject::connect(m_downloadEngine, cx->signal, this, cx->slot);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::onJobAdded(DownloadRange range)
{
    foreach (auto item, range) {
        auto downloadItem = dynamic_cast<AbstractDownloadItem*>(item);
        auto queueItem = new QueueItem(downloadItem, m_queueView);
        m_queueView->addTopLevelItem(queueItem);
    }
}

void DownloadQueueView::onJobRemoved(DownloadRange range)
{
    foreach (auto item, range) {
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

void DownloadQueueView::onJobStateChanged(IDownloadItem *item)
{
    QueueItem* queueItem = getQueueItem(item);
    if (queueItem) {
        queueItem->updateItem();
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::onSelectionChanged()
{
    const QSignalBlocker blocker(m_downloadEngine);
    m_downloadEngine->beginSelectionChange();

    auto selection = m_downloadEngine->selection();
    for (int index = 0, count = m_queueView->topLevelItemCount(); index < count; ++index) {
        auto treeItem = m_queueView->topLevelItem(index);
        auto queueItem = dynamic_cast<const QueueItem *>(treeItem);
        auto isSelected = selection.contains(queueItem->downloadItem());
        treeItem->setSelected(isSelected);
    }

    m_downloadEngine->endSelectionChange();
}

void DownloadQueueView::onSortChanged()
{
    // Save selection and current item
    auto currentItem = m_queueView->currentItem();
    auto selection = m_downloadEngine->selection();

    auto items = m_downloadEngine->downloadItems();
    for (int i = 0, total = items.size(); i < total; ++i) {
        auto downloadItem = items.at(i);
        auto index = getIndex(downloadItem);
        if (index != -1) {
            // Rem: takeTopLevelItem() changes the selection
            auto treeItem =  m_queueView->takeTopLevelItem(index);
            if (treeItem) {
                m_queueView->insertTopLevelItem(i, treeItem);
            }
        }
    }
    // Restore selection and current item
    m_queueView->setCurrentItem(currentItem);
    m_downloadEngine->setSelection(selection);
}

/******************************************************************************
 ******************************************************************************/
int DownloadQueueView::getIndex(IDownloadItem *downloadItem) const
{
    for (int index = 0, count = m_queueView->topLevelItemCount(); index < count; ++index) {
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

QueueItem* DownloadQueueView::getQueueItem(IDownloadItem *downloadItem)
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
    return Q_NULLPTR;
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
    QList<IDownloadItem *> selection;
    foreach (auto treeItem, m_queueView->selectedItems()) {
        auto queueItem = dynamic_cast<const QueueItem *>(treeItem);
        selection << queueItem->downloadItem();
    }
    m_downloadEngine->setSelection(selection);
}

/*!
 * Update the Engine data.
 */
void DownloadQueueView::onQueueItemCommitData(QWidget *editor)
{
    auto lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) {
        QString newName = lineEdit->text();

        // remove extension from base name
        int pos = newName.lastIndexOf('.');
        if (pos != -1) {
            newName = newName.left(pos);
        }

        auto treeItem = m_queueView->currentItem();
        auto queueItem = dynamic_cast<QueueItem *>(treeItem);
        AbstractDownloadItem* downloadItem = queueItem->downloadItem();

        downloadItem->rename(newName);
        queueItem->updateItem();
    }
}

void DownloadQueueView::onQueueItemDropped(QueueItem *queueItem)
{
    if (queueItem) {
        QList<IDownloadItem*> items;
        items << queueItem->downloadItem();
        m_downloadEngine->remove(items);
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
