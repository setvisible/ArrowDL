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

#include "streamlistwidget.h"
#include "ui_streamlistwidget.h"

#include <Core/Format>
#include <Widgets/CheckableItemDelegate>

#include <QtCore/QDebug>
#include <QtGui/QKeyEvent>
#include <QtGui/QMovie>

#include <algorithm> /* std::sort */

#define C_COLUMN_ID_WIDTH         10
#define C_COLUMN_NAME_WIDTH      200

/* Constant */
/// \todo these colors are from downloadqueueview.cpp
static const QColor s_red           = QColor(205, 0, 0);
static const QColor s_lightRed      = QColor(224, 97, 170);


/******************************************************************************
 ******************************************************************************/
/*!
 * \class StreamListItemDelegate is used to draw unavailable video in a different color.
 */
class StreamListItemDelegate : public CheckableItemDelegate
{
    /*
     * Remark:
     * If use Q_OBJECT, signals and slots in nested classes, add
     * <code>
     * #include "streamlistwidget.moc"
     * </code>
     * at this file's end.
     */
    Q_OBJECT

public:
    explicit StreamListItemDelegate(QObject *parent = Q_NULLPTR)
        : CheckableItemDelegate(parent)
    {}

    ~StreamListItemDelegate() Q_DECL_OVERRIDE {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;
};

void StreamListItemDelegate::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    initStyleOption(&myOption, index);

    auto model = qobject_cast<const StreamTableModel*>(index.model());
    if (model) {
        const StreamInfo streamInfo = model->itemAt(index.row());
        if (!streamInfo.isAvailable()){
            myOption.palette.setColor(QPalette::All, QPalette::Text, s_red);
        }
    }

    /* Inactive keep same colors as Active */
    auto p = myOption.palette;
    p.setColor(QPalette::Active, QPalette::HighlightedText,
               p.color(QPalette::Active, QPalette::Text)); // otherwise it's another color
    p.setColor(QPalette::Inactive, QPalette::Base,
               p.color(QPalette::Active, QPalette::Base));
    p.setColor(QPalette::Inactive, QPalette::Highlight,
               p.color(QPalette::Active, QPalette::Highlight));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText,
               p.color(QPalette::Active, QPalette::HighlightedText));
    p.setColor(QPalette::Inactive, QPalette::Text,
               p.color(QPalette::Active, QPalette::Text));
    myOption.palette = p;

    CheckableItemDelegate::paint(painter, myOption, index);
}

/******************************************************************************
 ******************************************************************************/
StreamListWidget::StreamListWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::StreamListWidget)
  , m_playlistModel(new StreamTableModel(this))
{
    ui->setupUi(this);

    ui->playlistView->setItemDelegate(new StreamListItemDelegate(ui->playlistView));
    ui->playlistView->setModel(m_playlistModel);

    QList<int> defaultWidths = {-1, C_COLUMN_ID_WIDTH, C_COLUMN_NAME_WIDTH, -1, -1, -1};
    setColumnWidths(defaultWidths);

    adjustSize();

    connect(m_playlistModel, SIGNAL(checkStateChanged(QModelIndex, bool)),
            this, SLOT(onCheckStateChanged(QModelIndex, bool)));

    connect(ui->playlistView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(onSelectionChanged(const QItemSelection &, const QItemSelection &)));

    connect(ui->streamWidget, SIGNAL(streamInfoChanged(StreamInfo)),
            this, SLOT(onStreamInfoChanged(StreamInfo)));

    /* Fancy GIF animation */
    QMovie *movie = new QMovie(":/icons/menu/stream_wait_16x16.gif");
    ui->waitingIconLabel->setMovie(movie);
    movie->start();

    setMessageEmpty();
    retranslateUi();
}

StreamListWidget::~StreamListWidget()
{
    delete ui;
}

static void moveCursor(CheckableTableView *view, int key)
{
    Q_ASSERT(view);
    auto rowMax = view->model()->rowCount() - 1;
    auto currentIndex = view->currentIndex();
    auto row = currentIndex.row();
    switch (key) {
    case Qt::Key_Up:    row = qMax(0, row-1);       break;
    case Qt::Key_Down:  row = qMin(rowMax, row+1);  break;
    case Qt::Key_Home:  row = 0;                    break;
    case Qt::Key_End:   row = rowMax;               break;
    default: break;
    }
    view->setCurrentIndex(view->model()->index(row, currentIndex.column()));
}

void StreamListWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Home:
    case Qt::Key_End:
        moveCursor(ui->playlistView, event->key());
        break;
    default:
        break;
    }
    QWidget::keyPressEvent(event);
}

void StreamListWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void StreamListWidget::retranslateUi()
{
    m_playlistModel->retranslateUi();
}

bool StreamListWidget::isValid() const
{
    return m_state == Normal;
}

/******************************************************************************
 ******************************************************************************/
void StreamListWidget::setMessageEmpty()
{
    setState(Empty);
    m_playlistModel->clear();
}

void StreamListWidget::setMessageWait()
{
    setState(Downloading);
    m_playlistModel->clear();
}

void StreamListWidget::setMessageError(QString errorMessage)
{
    setState(StreamListWidget::Error);
    ui->errorMessageLabel->setText(errorMessage);
    m_playlistModel->clear();
}

/******************************************************************************
 ******************************************************************************/
void StreamListWidget::setStreamInfoList(StreamInfo streamInfo)
{
    setStreamInfoList(QList<StreamInfo>() << streamInfo);
}

void StreamListWidget::setStreamInfoList(QList<StreamInfo> streamInfoList)
{
    setState(StreamListWidget::Normal);
    m_playlistModel->setStreamInfoList(streamInfoList);
    if (!streamInfoList.isEmpty()) {
        auto first = streamInfoList.first();
        ui->playlistTitleLabel->setText(first.playlist);

        // Check all available videos in the playlist
        for (int i = 0; i < streamInfoList.count(); ++i) {
            auto streamInfo = streamInfoList.at(i);
            if (streamInfo.isAvailable()) {
                m_playlistModel->setData(m_playlistModel->index(i, 0), true, CheckableTableModel::CheckStateRole);
            }
        }
        // Select the first entry
        ui->playlistView->selectRow(0);
        // Force signal
    }
    // Eventually, hide the playlist panel
    ui->playlistPanelWidget->setVisible(streamInfoList.count() > 1);
}

/******************************************************************************
 ******************************************************************************/
QList<int> StreamListWidget::columnWidths() const
{
    return ui->playlistView->columnWidths();
}

void StreamListWidget::setColumnWidths(const QList<int> &widths)
{
    ui->playlistView->setColumnWidths(widths);
}

/******************************************************************************
 ******************************************************************************/
void StreamListWidget::onSelectionChanged(const QItemSelection &, const QItemSelection &)
{
    auto rows = selectedRows();
    if (rows.count() == 1) {
        auto streamInfo = m_playlistModel->itemAt(rows.first());
        ui->streamWidget->setStreamInfo(streamInfo);
        ui->streamWidget->setVisible(true);
    } else {
        ui->streamWidget->setVisible(false);
    }
}

void StreamListWidget::onStreamInfoChanged(StreamInfo streamInfo)
{
    auto selectedRows = ui->playlistView->selectionModel()->selectedRows(0);
    if (selectedRows.count() == 1) {
        auto index = selectedRows.first();
        m_playlistModel->setItemAt(index.row(), streamInfo);
    }
}

void StreamListWidget::onCheckStateChanged(QModelIndex index, bool checked)
{
    if (checked) {
        auto streamInfo = m_playlistModel->itemAt(index.row());
        if (!streamInfo.isAvailable()) {
            m_playlistModel->setData(index, false, CheckableTableModel::CheckStateRole);
        }
    }
}


/******************************************************************************
 ******************************************************************************/
QList<StreamInfo> StreamListWidget::selection() const
{
    return m_playlistModel->selection();
}

/******************************************************************************
 ******************************************************************************/
StreamListWidget::State StreamListWidget::state() const
{
    return m_state;
}

void StreamListWidget::setState(State state)
{
    m_state = state;
    switch (m_state) {
    case Empty:       ui->stackedWidget->setCurrentWidget(ui->pageEmpty);       break;
    case Downloading: ui->stackedWidget->setCurrentWidget(ui->pageDownloading); break;
    case Normal:      ui->stackedWidget->setCurrentWidget(ui->pageNormal);      break;
    case Error:       ui->stackedWidget->setCurrentWidget(ui->pageError);       break;
    }
}

QList<int> StreamListWidget::selectedRows() const
{
    QSet<int> rows;
    auto indexes = ui->playlistView->selectionModel()->selectedRows(0);
    foreach (auto index, indexes) {
        rows.insert(index.row());
    }
    auto list = rows.toList();
    std::sort(list.begin(), list.end());
    return list;

}

/******************************************************************************
 ******************************************************************************/
StreamTableModel::StreamTableModel(QObject *parent) : CheckableTableModel(parent)
{
    retranslateUi();
}

void StreamTableModel::clear()
{
    beginResetModel();
    CheckableTableModel::clear();
    m_items.clear();
    endResetModel();
}

void StreamTableModel::retranslateUi()
{
    m_headers = QStringList()
            << QString() // checkbox column
            << tr("#")
            << tr("File Name")
            << tr("Title")
            << tr("Size")
            << tr("Format");
}

void StreamTableModel::setStreamInfoList(QList<StreamInfo> streamInfoList)
{
    clear();
    if (!streamInfoList.isEmpty()) {
        QModelIndex parent = QModelIndex(); // root is always empty
        beginInsertRows(parent, 0, streamInfoList.count());
        m_items = streamInfoList;
        endInsertRows();
    }
}

/******************************************************************************
 ******************************************************************************/
StreamInfo StreamTableModel::itemAt(int row) const
{
    Q_ASSERT(row >= 0 && row < m_items.count());
    return m_items.at(row);
}

void StreamTableModel::setItemAt(int row, const StreamInfo &streamInfo)
{
    Q_ASSERT(row >= 0 && row < m_items.count());
    auto oldStreamInfo = m_items.at(row);
    if (streamInfo != oldStreamInfo) {
        m_items.replace(row, streamInfo);
        emit dataChanged(index(row, 0), index(row, columnCount()), {Qt::DisplayRole});
    }
}

/******************************************************************************
 ******************************************************************************/
QList<StreamInfo> StreamTableModel::selection() const
{
    QList<StreamInfo> selection;
    foreach (int row, this->checkedRows()) {
        if (row >= 0 && row < m_items.count()) {
            selection << m_items.at(row);
        }
    }
    return selection;
}

int StreamTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_headers.count();
}

QVariant StreamTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section >= 0 && section < m_headers.count()) {
            return m_headers.at(section);
        } else {
            return QVariant();
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

int StreamTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.count();
}

QVariant StreamTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.row() >= rowCount() || index.row() < 0) {
        return QVariant();
    }
    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case  0:
        case  1: return int(Qt::AlignCenter | Qt::AlignVCenter);
        case  2:
        case  3: return int(Qt::AlignLeft | Qt::AlignVCenter);
        case  4: return int(Qt::AlignRight | Qt::AlignVCenter);
        case  5: return int(Qt::AlignLeft | Qt::AlignVCenter);
        default:
            break;
        }

    } else if (role == Qt::DisplayRole) {
        auto streamInfo = m_items.at(index.row());
        switch (index.column()) {
        case  0: return QVariant();
        case  1: return streamInfo.playlist_index;
        case  2: return filenameOrErrorMessage(streamInfo);
        case  3: return streamInfo.defaultTitle;
        case  4: return Format::fileSizeToString(streamInfo.guestimateFullSize());
        case  5: return streamInfo.formatToString();
        default:
            break;
        }
    }
    return CheckableTableModel::data(index, role);
}

QString StreamTableModel::filenameOrErrorMessage(const StreamInfo &streamInfo) const
{
    if (streamInfo.isAvailable()) {
        return streamInfo.fullFileName();
    } else {
        return QString("[%0]").arg(tr("Video unavailable"));
    }
}

#include "streamlistwidget.moc"
