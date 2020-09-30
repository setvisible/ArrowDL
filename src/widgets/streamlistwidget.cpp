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
#include <Widgets/Globals>

#include <QtCore/QDebug>
#include <QtGui/QKeyEvent>
#include <QtGui/QMovie>

#include <algorithm> /* std::sort */

constexpr int column_id_width = 10;
constexpr int column_name_width = 200;


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

    ~StreamListItemDelegate() Q_DECL_OVERRIDE = default;

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
        const StreamObject streamObject = model->itemAt(index.row());
        if (!streamObject.isAvailable()){
            myOption.palette.setColor(QPalette::All, QPalette::Text, s_red);
        }
    }

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

    QList<int> defaultWidths = {-1, column_id_width, column_name_width, -1, -1, -1};
    setColumnWidths(defaultWidths);

    adjustSize();

    connect(m_playlistModel, SIGNAL(checkStateChanged(QModelIndex, bool)),
            this, SLOT(onCheckStateChanged(QModelIndex, bool)));

    connect(ui->playlistView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(onSelectionChanged(const QItemSelection &, const QItemSelection &)));

    connect(ui->streamWidget, SIGNAL(streamObjectChanged(StreamObject)),
            this, SLOT(onStreamObjectChanged(StreamObject)));

    connect(ui->trackNumberCheckBox, SIGNAL(stateChanged(int)), this,
            SLOT(onTrackNumberChecked(int)));

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

void StreamListWidget::setMessageError(const QString &errorMessage)
{
    setState(StreamListWidget::Error);
    ui->errorMessageLabel->setText(errorMessage);
    m_playlistModel->clear();
}

/******************************************************************************
 ******************************************************************************/
void StreamListWidget::setStreamObjects(const StreamObject &streamObject)
{
    setStreamObjects(QList<StreamObject>() << streamObject);
}

void StreamListWidget::setStreamObjects(const QList<StreamObject> &streamObjects)
{
    setState(StreamListWidget::Normal);
    m_playlistModel->setStreamObjects(streamObjects);
    if (!streamObjects.isEmpty()) {
        auto first = streamObjects.first();
        ui->playlistTitleLabel->setText(first.playlist);

        // Check all available videos in the playlist
        for (int i = 0; i < streamObjects.count(); ++i) {
            auto streamObject = streamObjects.at(i);
            if (streamObject.isAvailable()) {
                m_playlistModel->setData(m_playlistModel->index(i, 0), true, CheckableTableModel::CheckStateRole);
            }
        }
        // Select the first entry
        ui->playlistView->selectRow(0);
        // Force signal
    }
    // Eventually, hide the playlist panel
    ui->playlistPanelWidget->setVisible(streamObjects.count() > 1);
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
        auto streamObject = m_playlistModel->itemAt(rows.first());
        ui->streamWidget->setStreamObject(streamObject);
        ui->streamWidget->setVisible(true);
    } else {
        ui->streamWidget->setVisible(false);
    }
}

void StreamListWidget::onStreamObjectChanged(const StreamObject &streamObject)
{
    auto selectedRows = ui->playlistView->selectionModel()->selectedRows(0);
    if (selectedRows.count() == 1) {
        auto index = selectedRows.first();
        m_playlistModel->setItemAt(index.row(), streamObject);
    }
}

void StreamListWidget::onCheckStateChanged(const QModelIndex &index, bool checked)
{
    if (checked) {
        auto streamObject = m_playlistModel->itemAt(index.row());
        if (!streamObject.isAvailable()) {
            m_playlistModel->setData(index, false, CheckableTableModel::CheckStateRole);
        }
    }
}

void StreamListWidget::onTrackNumberChecked(int state)
{
    auto checked = static_cast<Qt::CheckState>(state) == Qt::Checked;
    m_playlistModel->enableTrackNumberPrefix(checked);
    onSelectionChanged(QItemSelection(), QItemSelection());
}

/******************************************************************************
 ******************************************************************************/
QList<StreamObject> StreamListWidget::selection() const
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

void StreamTableModel::setStreamObjects(const QList<StreamObject> &streamObjects)
{
    clear();
    if (!streamObjects.isEmpty()) {
        QModelIndex parent = QModelIndex(); // root is always empty
        beginInsertRows(parent, 0, streamObjects.count());
        m_items = streamObjects;
        endInsertRows();
    }
}

/******************************************************************************
 ******************************************************************************/
void StreamTableModel::enableTrackNumberPrefix(bool enable)
{
    for (int row = 0; row < m_items.count(); ++row) {
        auto item = m_items.at(row); // copy!
        auto title = item.title();
        auto prefix = QString("%0 ").arg(item.playlist_index);

        // remove previous track number
        if (title.startsWith(prefix)) {
            title.remove(0, prefix.length());
        }
        if (enable) {
            title.prepend(prefix);
        }
        item.setTitle(title);
        m_items.replace(row, item);
    }
    emit dataChanged(index(0, 0), index(rowCount(), columnCount()), {Qt::DisplayRole});
}

/******************************************************************************
 ******************************************************************************/
StreamObject StreamTableModel::itemAt(int row) const
{
    Q_ASSERT(row >= 0 && row < m_items.count());
    return m_items.at(row);
}

void StreamTableModel::setItemAt(int row, const StreamObject &streamObject)
{
    Q_ASSERT(row >= 0 && row < m_items.count());
    auto old = m_items.at(row);
    if (streamObject != old) {
        m_items.replace(row, streamObject);
        emit dataChanged(index(row, 0), index(row, columnCount()), {Qt::DisplayRole});
    }
}

/******************************************************************************
 ******************************************************************************/
QList<StreamObject> StreamTableModel::selection() const
{
    QList<StreamObject> selection;
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
        }
        return QVariant();
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
        auto streamObject = m_items.at(index.row());
        switch (index.column()) {
        case  0: return QVariant();
        case  1: return streamObject.playlist_index;
        case  2: return filenameOrErrorMessage(streamObject);
        case  3: return streamObject.defaultTitle;
        case  4: return Format::fileSizeToString(streamObject.guestimateFullSize());
        case  5: return streamObject.formatToString();
        default:
            break;
        }
    }
    return CheckableTableModel::data(index, role);
}

QString StreamTableModel::filenameOrErrorMessage(const StreamObject &streamObject) const
{
    if (streamObject.isAvailable()) {
        return streamObject.fullFileName();
    }
    return QString("[%0]").arg(tr("Video unavailable"));
}

#include "streamlistwidget.moc"
