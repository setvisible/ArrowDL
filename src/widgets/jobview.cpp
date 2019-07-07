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

#include "jobview.h"

#include <Core/JobClient>
#include <Core/JobManager>

#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QItemDelegate>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
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


/*!
 * QueueView extends QTreeWidget to allow drag and drop.
 */
class QueueView : public QTreeWidget
{
    friend class JobView; /* To acceed protected members */
    Q_OBJECT

public:
    QueueView(QWidget *parent = 0);

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
    //#if QT_CONFIG(draganddrop)
    //    setAcceptDrops(true);
    //#endif
}

/******************************************************************************
 ******************************************************************************/
/*!
 * QueueViewItemDelegate is used to draw the progress bars.
 */
class QueueViewItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    inline QueueViewItemDelegate(JobView *parent) : QItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index ) const override
    {
        if (index.column() == 0) {

            // todo : add icon + text
            QItemDelegate::paint(painter, option, index);

        } else if (index.column() == 2) {

            // Set up a QStyleOptionProgressBar to precisely mimic the
            // environment of a progress bar.
            QStyleOptionProgressBar progressBarOption;
            progressBarOption.state = QStyle::State_Enabled;
            progressBarOption.direction = QApplication::layoutDirection();
            progressBarOption.rect = option.rect;
            progressBarOption.fontMetrics = QApplication::fontMetrics();
            progressBarOption.minimum = 0;
            progressBarOption.maximum = 100;
            progressBarOption.textAlignment = Qt::AlignCenter;
            progressBarOption.textVisible = false;
            //  progressBarOption.palette.setColor();

            // Set the progress and text values of the style option.
            const JobView *jobView = qobject_cast<const JobView *>(parent());
            const JobManager *manager = jobView->jobManager();
            const JobClient *job = manager->clientForRow(index.row());


            progressBarOption.progress = job->progress();

            //   painter->drawi
            // Draw the progress bar onto the view.
            QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);


        } else {
            QItemDelegate::paint(painter, option, index);
        }
    }
};

/******************************************************************************
 ******************************************************************************/
JobView::JobView(QWidget *parent) : QWidget(parent)
  , m_jobManager(Q_NULLPTR)
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

JobView::~JobView()
{
}

/******************************************************************************
 ******************************************************************************/
QSize JobView::sizeHint() const
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
const JobManager* JobView::jobManager() const
{
    return m_jobManager;
}

void JobView::setManager(JobManager *jobManager)
{
    if (m_jobManager) {
        this->disconnect(m_jobManager, SIGNAL(jobAppended(JobClient*)),
                         this, SLOT(onJobAdded(JobClient*)));
        this->disconnect(m_jobManager, SIGNAL(jobRemoved(JobClient*)),
                         this, SLOT(onJobRemoved(JobClient*)));
        this->disconnect(m_jobManager, SIGNAL(jobStateChanged(JobClient*)),
                         this, SLOT(onJobStateChanged(JobClient*)));
        this->disconnect(m_jobManager, SIGNAL(selectionChanged()),
                         this, SLOT(onSelectionChanged()));
    }
    m_jobManager = jobManager;
    if (m_jobManager) {
        this->connect(m_jobManager, SIGNAL(jobAppended(JobClient*)),
                      this, SLOT(onJobAdded(JobClient*)));
        this->connect(m_jobManager, SIGNAL(jobRemoved(JobClient*)),
                      this, SLOT(onJobRemoved(JobClient*)));
        this->connect(m_jobManager, SIGNAL(jobStateChanged(JobClient*)),
                      this, SLOT(onJobStateChanged(JobClient*)));
        this->connect(m_jobManager, SIGNAL(selectionChanged()),
                      this, SLOT(onSelectionChanged()));
    }
}

/******************************************************************************
 ******************************************************************************/
void JobView::onJobAdded(JobClient *job)
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    updateItem(item, job);
    m_queueView->addTopLevelItem(item);
    addItem(item, job);
}

void JobView::onJobRemoved(JobClient *job)
{
    // or get the index instead ?
    QTreeWidgetItem* item = getItem(job);
    removeItem(item);

    const int count = m_queueView->topLevelItemCount();
    for (int index = 0; index < count; ++index) {
        if (m_queueView->topLevelItem(index) == item) {
            m_queueView->takeTopLevelItem(index);
            delete item;
            item = 0;
        }
    }
}

void JobView::onJobStateChanged(JobClient *job)
{
    QTreeWidgetItem* item = getItem(job);
    updateItem(item, job);
}

static inline QString stateToString(JobClient::State state)
{
    QString stateString;
    switch (state) {
    case JobClient::Idle:
        stateString = QT_TRANSLATE_NOOP(JobClient, "Idle");
        break;
    case JobClient::Paused:
        stateString = QT_TRANSLATE_NOOP(JobClient, "Paused");
        break;
    case JobClient::Stopped:
        stateString = QT_TRANSLATE_NOOP(JobClient, "Canceled");
        break;
    case JobClient::Preparing:
        stateString = QT_TRANSLATE_NOOP(JobClient, "Preparing");
        break;
    case JobClient::Connecting:
        stateString = QT_TRANSLATE_NOOP(JobClient, "Connecting");
        break;
    case JobClient::Downloading:
        stateString = QT_TRANSLATE_NOOP(JobClient, "Downloading");
        break;
    case JobClient::Endgame:
        stateString = QT_TRANSLATE_NOOP(JobClient, "Finishing");
        break;
    case JobClient::Completed:
        stateString = QT_TRANSLATE_NOOP(JobClient, "Complete");
        break;
    case JobClient::Skipped:
        stateString = QT_TRANSLATE_NOOP(JobClient, "Skipped");
        break;
    case JobClient::NetworkError:
        stateString = QT_TRANSLATE_NOOP(JobClient, "Server error");
        break;
    case JobClient::FileError:
        stateString = QT_TRANSLATE_NOOP(JobClient, "File error");
        break;
    default:
        stateString = QT_TRANSLATE_NOOP(JobClient, "????");
        break;
    }
    return stateString ;
}

void JobView::updateItem(QTreeWidgetItem* item, JobClient *job)
{
    if (!item) {
        return;
    }
    QString size;
    if (job->bytesTotal() > 0) {
        size = tr("%0 of %1")
                .arg(job->bytesReceived())
                .arg(job->bytesTotal());
    } else {
        size = tr("Unknown");
    }

    QString estTime = stateToString(job->state());
    if (job->state() == JobClient::NetworkError) {
        /*
         * See QNetworkReply::NetworkError Documentation for conversion
         */
        int httpErrorNumber = (int) job->error();
        if (httpErrorNumber == 201) httpErrorNumber = 401;
        if (httpErrorNumber == 203) httpErrorNumber = 404;
        estTime += tr("(%0)").arg(httpErrorNumber);

    } else if (job->state() == JobClient::Downloading) {
        estTime = "--:--";
        // todo
    }

    QString speed = "-";
    // speed.sprintf("%.1f KB/s", bytesPerSecond / 1024.0);


    item->setText(C_COL_0_FILE_NAME       , job->localFileName());
    item->setText(C_COL_1_WEBSITE_DOMAIN  , job->sourceUrl().host()); // todo domain only

    //item->setText(C_OL_2_PROGRESS_BAR    , QString());
    item->setSizeHint(C_COL_2_PROGRESS_BAR, QSize(100, 22));

    item->setText(C_COL_3_PERCENT         , QString::asprintf("%d%%", job->progress()));
    item->setText(C_COL_4_SIZE            , size);
    item->setText(C_COL_5_ESTIMATED_TIME  , estTime);
    item->setText(C_COL_6_SPEED           , speed);

    //item->setText(C_COL_7_SEGMENTS, "Unknown");
    // todo etc...
}

/******************************************************************************
 ******************************************************************************/
void JobView::onSelectionChanged()
{
    QList<JobClient*> selection = m_jobManager->selection();
    const int count = m_queueView->topLevelItemCount();
    for (int index = 0; index < count; ++index) {
        QTreeWidgetItem* item = m_queueView->topLevelItem(index);

        const bool isSelected = selection.contains(getJob(item));
        item->setSelected(isSelected);
    }
}

/******************************************************************************
 ******************************************************************************/
void JobView::addItem(QTreeWidgetItem* item, JobClient *job)
{
    m_map.insert(item, job);
}

void JobView::removeItem(QTreeWidgetItem* item)
{
    m_map.remove(item);
}

QTreeWidgetItem* JobView::getItem(JobClient *job)
{
    return m_map.key(job, 0);
}

JobClient* JobView::getJob(QTreeWidgetItem* item)
{
    return m_map.value(item, 0);
}

/******************************************************************************
 ******************************************************************************/
void JobView::onQueueViewDoubleClicked(const QModelIndex &index)
{
    QTreeWidgetItem *item = m_queueView->itemFromIndex(index);
    if (item) {
        auto job = getJob(item);
        emit doubleClicked(job);
    }
}

void JobView::onQueueViewItemSelectionChanged()
{
    QList<JobClient*> selection;
    foreach (auto item, m_queueView->selectedItems()) {
        selection << getJob(item);
    }
    m_jobManager->setSelection(selection);
}

/******************************************************************************
 ******************************************************************************/
void JobView::setContextMenu(QMenu *contextMenu)
{
    m_contextMenu = contextMenu;
}

void JobView::showContextMenu(const QPoint &pos)
{
    if (m_contextMenu) {
        m_contextMenu->exec(mapToGlobal(pos));
    }
}

#include "jobview.moc"
