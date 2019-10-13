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

#include "downloadqueueview.h"

#include <Core/AbstractDownloadItem>
#include <Core/DownloadEngine>
#include <Core/Format>
#include <Core/MimeDatabase>
#include <Widgets/CustomStyle>
#include <Widgets/CustomStyleOptionProgressBar>

#include <QtCore/QDebug>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMenu>
#include <QtWidgets/QStyledItemDelegate>


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
    case IDownloadItem::Downloading:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Downloading");
        break;
    case IDownloadItem::Endgame:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Finishing");
        break;
    case IDownloadItem::Completed:
        stateString = QT_TRANSLATE_NOOP(DownloadItem, "Complete");
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
/*!
 * QueueView extends QTreeWidget to allow drag and drop.
 */
class QueueView : public QTreeWidget
{
    friend class DownloadQueueView; /* To acceed protected members */
    Q_OBJECT

public:
    QueueView(QWidget *parent);

    //#if QT_CONFIG(draganddrop)
    //signals:
    //    void fileDropped(const QString &fileName);
    //
    //protected:
    //    void dragMoveEvent(QDragMoveEvent *event) override;
    //    void dropEvent(QDropEvent *event) override;
    //#endif
};

QueueView::QueueView(QWidget *parent)
    : QTreeWidget(parent)
{
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    //#if QT_CONFIG(draganddrop)
    //    setAcceptDrops(true);
    //#endif
}

/******************************************************************************
 ******************************************************************************/
/*!
 * QueueViewItemDelegate is used to draw the progress bars.
 */
class QueueViewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    inline QueueViewItemDelegate(DownloadQueueView *parent);

    // painting
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index ) const Q_DECL_OVERRIDE;



private:
    QIcon m_idleIcon;
    QIcon m_resumeIcon;
    QIcon m_pauseIcon;
    QIcon m_stopIcon;
    QIcon m_completedIcon;

    QColor stateColor(IDownloadItem::State state) const;
    QIcon stateIcon(IDownloadItem::State state) const;
};

QueueViewItemDelegate::QueueViewItemDelegate(DownloadQueueView *parent) : QStyledItemDelegate(parent)
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

    if (index.column() == 0) {

        const QUrl url(myOption.text);
        const QPixmap pixmap = MimeDatabase::fileIcon(url, 16);

        myOption.icon.addPixmap(pixmap);
        myOption.decorationAlignment = Qt::AlignHCenter |Qt::AlignVCenter;
        myOption.decorationPosition = QStyleOptionViewItem::Left;
        myOption.features = myOption.features | QStyleOptionViewItem::HasDecoration;

        QStyledItemDelegate::paint(painter, myOption, index);

    } else if (index.column() == 2) {

        const int progress = index.data(ProgressBar::ProgressRole).toInt();
        const IDownloadItem::State state = (IDownloadItem::State)index.data(ProgressBar::StateRole).toInt();

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


QColor QueueViewItemDelegate::stateColor(IDownloadItem::State state) const
{
    switch (state) {
    case IDownloadItem::Idle:
        return s_darkGrey;
        break;
    case IDownloadItem::Paused:
        return s_darkYellow;
        break;
    case IDownloadItem::Preparing:
    case IDownloadItem::Connecting:
    case IDownloadItem::Downloading:
    case IDownloadItem::Endgame:
        return s_green;
        break;
    case IDownloadItem::Completed:
        return s_darkGreen;
        break;
    case IDownloadItem::Stopped:
    case IDownloadItem::Skipped:
    case IDownloadItem::NetworkError:
    case IDownloadItem::FileError:
        return s_darkRed;
        break;
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
        break;
    case IDownloadItem::Paused:
        return m_pauseIcon;
        break;
    case IDownloadItem::Preparing:
    case IDownloadItem::Connecting:
    case IDownloadItem::Downloading:
    case IDownloadItem::Endgame:
        return m_resumeIcon;
        break;
    case IDownloadItem::Completed:
        return m_completedIcon;
        break;
    case IDownloadItem::Stopped:
    case IDownloadItem::Skipped:
    case IDownloadItem::NetworkError:
    case IDownloadItem::FileError:
        return m_stopIcon;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    return QIcon();
}

/******************************************************************************
 ******************************************************************************/
class QueueItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT

public:
    explicit QueueItem(AbstractDownloadItem *downloadItem, QTreeWidget *view);

    AbstractDownloadItem* downloadItem() const { return m_downloadItem; }

public slots:
    void updateItem();

private:
    AbstractDownloadItem *m_downloadItem;
};

QueueItem::QueueItem(AbstractDownloadItem *downloadItem, QTreeWidget *view)
    : QObject(view)
    , QTreeWidgetItem(view, QTreeWidgetItem::UserType)
    , m_downloadItem(downloadItem)
{
    setSizeHint(C_COL_2_PROGRESS_BAR, QSize(C_COLUMN_DEFAULT_WIDTH, C_ROW_DEFAULT_HEIGHT));
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
        /*
         * See QNetworkReply::NetworkError Documentation for conversion
         */
        int httpErrorNumber = m_downloadItem->httpErrorNumber();
        if (httpErrorNumber == 201) httpErrorNumber = 401;
        if (httpErrorNumber == 203) httpErrorNumber = 404;
        estTime += tr("(%0)").arg(httpErrorNumber);

    } else if (m_downloadItem->state() == IDownloadItem::Downloading) {
        estTime = Format::remaingTimeToString(m_downloadItem->remainingTime());
    }

    QString speed = Format::currentSpeedToString(m_downloadItem->speed());

    this->setText(C_COL_0_FILE_NAME       , m_downloadItem->localFileName());
    this->setText(C_COL_1_WEBSITE_DOMAIN  , m_downloadItem->sourceUrl().host()); // todo domain only

    this->setData(C_COL_2_PROGRESS_BAR, ProgressBar::StateRole, m_downloadItem->state());
    this->setData(C_COL_2_PROGRESS_BAR, ProgressBar::ProgressRole, m_downloadItem->progress());

    this->setText(C_COL_3_PERCENT         , QString::asprintf("%d%%", qMax(0, m_downloadItem->progress())));
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
               // << tr("Segments")    /* hidden */
               // << tr("Mask")        /* hidden */
               // << tr("Save path")   /* hidden */
               // << tr("Checksum")    /* hidden */
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

    connect(m_queueView, SIGNAL(itemSelectionChanged()),
            this, SLOT(onQueueViewItemSelectionChanged()));
    connect(m_queueView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(onQueueViewDoubleClicked(QModelIndex)));

    QLayout* layout = new QGridLayout(this);
    layout->addWidget(m_queueView);

    this->setLayout(layout);

    // To enable the user to move the items around within the view,
    // we must set the list widget's dragDropMode:
    m_queueView->setDragDropMode(QAbstractItemView::InternalMove);
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
QList<int> DownloadQueueView::columnWidths() const
{
    QList<int> widths;
    for (int column = 0; column < m_queueView->columnCount(); ++column) {
        const int width = m_queueView->columnWidth(column);
        widths.append(width);
    }
    return widths;
}

void DownloadQueueView::setColumnWidths(const QList<int> &widths)
{
    for (int column = 0; column < m_queueView->columnCount(); ++column) {
        if (column < widths.count()) {
            const int width = widths.at(column);
            m_queueView->setColumnWidth(column, width);
        } else {
            m_queueView->setColumnWidth(column, C_COLUMN_DEFAULT_WIDTH);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
const DownloadEngine *DownloadQueueView::engine() const
{
    return m_downloadEngine;
}

void DownloadQueueView::setEngine(DownloadEngine *downloadEngine)
{
    if (m_downloadEngine) {
        this->disconnect(m_downloadEngine, SIGNAL(jobAppended(IDownloadItem*)),
                         this, SLOT(onJobAdded(IDownloadItem*)));
        this->disconnect(m_downloadEngine, SIGNAL(jobRemoved(IDownloadItem*)),
                         this, SLOT(onJobRemoved(IDownloadItem*)));
        this->disconnect(m_downloadEngine, SIGNAL(jobStateChanged(IDownloadItem*)),
                         this, SLOT(onJobStateChanged(IDownloadItem*)));
        this->disconnect(m_downloadEngine, SIGNAL(selectionChanged()),
                         this, SLOT(onSelectionChanged()));
    }
    m_downloadEngine = downloadEngine;
    if (m_downloadEngine) {
        this->connect(m_downloadEngine, SIGNAL(jobAppended(IDownloadItem*)),
                      this, SLOT(onJobAdded(IDownloadItem*)));
        this->connect(m_downloadEngine, SIGNAL(jobRemoved(IDownloadItem*)),
                      this, SLOT(onJobRemoved(IDownloadItem*)));
        this->connect(m_downloadEngine, SIGNAL(jobStateChanged(IDownloadItem*)),
                      this, SLOT(onJobStateChanged(IDownloadItem*)));
        this->connect(m_downloadEngine, SIGNAL(selectionChanged()),
                      this, SLOT(onSelectionChanged()));
    }
}

/******************************************************************************
 ******************************************************************************/
void DownloadQueueView::onJobAdded(IDownloadItem *item)
{
    AbstractDownloadItem* downloadItem = static_cast<AbstractDownloadItem*>(item);
    QueueItem* queueItem = new QueueItem(downloadItem, m_queueView);
    m_queueView->addTopLevelItem(queueItem);
}

void DownloadQueueView::onJobRemoved(IDownloadItem *item)
{
    const int index = getIndex(item);
    if (index >= 0) {
        QTreeWidgetItem *treeItem = m_queueView->takeTopLevelItem(index);
        if (treeItem) {
            delete treeItem;
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
    m_downloadEngine->beginSelectionChange();

    const QList<IDownloadItem *> selection = m_downloadEngine->selection();
    const int count = m_queueView->topLevelItemCount();
    for (int index = 0; index < count; ++index) {
        QTreeWidgetItem* treeItem = m_queueView->topLevelItem(index);
        const QueueItem* queueItem = static_cast<const QueueItem *>(treeItem);
        const bool isSelected = selection.contains(queueItem->downloadItem());
        treeItem->setSelected(isSelected);
    }

    m_downloadEngine->endSelectionChange();
}

/******************************************************************************
 ******************************************************************************/
int DownloadQueueView::getIndex(IDownloadItem *downloadItem) const
{
    const int count = m_queueView->topLevelItemCount();
    for (int index = 0; index < count; ++index) {
        QTreeWidgetItem *treeItem = m_queueView->topLevelItem(index);
        if (treeItem->type() == QTreeWidgetItem::UserType) {
            const QueueItem *queueItem = static_cast<const QueueItem *>(treeItem);
            if (queueItem && downloadItem && queueItem->downloadItem() == downloadItem) {
                return index;
            }
        }
    }
    return -1;
}

QueueItem* DownloadQueueView::getQueueItem(IDownloadItem *downloadItem)
{
    const int index = getIndex(downloadItem);
    if (index >= 0) {
        QTreeWidgetItem *treeItem = m_queueView->topLevelItem(index);
        if (treeItem->type() == QTreeWidgetItem::UserType) {
            QueueItem *queueItem = static_cast<QueueItem *>(treeItem);
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
    QTreeWidgetItem *treeItem = m_queueView->itemFromIndex(index);
    const QueueItem *queueItem = static_cast<const QueueItem *>(treeItem);
    emit doubleClicked(queueItem->downloadItem());
}

void DownloadQueueView::onQueueViewItemSelectionChanged()
{
    QList<IDownloadItem *> selection;
    foreach (auto treeItem, m_queueView->selectedItems()) {
        const QueueItem *queueItem = static_cast<const QueueItem *>(treeItem);
        selection << queueItem->downloadItem();
    }
    m_downloadEngine->setSelection(selection);
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

#include "downloadqueueview.moc"
