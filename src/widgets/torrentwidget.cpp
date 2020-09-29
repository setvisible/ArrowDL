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

#include "torrentwidget.h"
#include "ui_torrentwidget.h"

#include <Core/Format>
#include <Core/ITorrentContext>
#include <Core/Torrent>
#include <Core/TorrentMessage>
#include <Widgets/CustomStyle>
#include <Widgets/CustomStyleOptionProgressBar>
#include <Widgets/Globals>
#include <Widgets/TorrentProgressBar>

#include <QtCore/QDebug>
#include <QtCore/QAbstractTableModel>
#include <QtCore/QBitArray>
#include <QtCore/QPair>
#include <QtCore/QVector>
#include <QtGui/QClipboard>
#include <QtGui/QPainter>
#include <QtWidgets/QAction>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableView>

#define C_COLUMN_MINIMUM_WIDTH       10
#define C_COLUMN_DEFAULT_WIDTH      100
#define C_ROW_DEFAULT_HEIGHT         18


/******************************************************************************
 ******************************************************************************/
class Headers // Holds column header's widths and titles
{
public:
    Headers() = default;
    Headers(const QList<QPair<int, QString> > &l) {d = l; }
    Headers &operator=(const QList<QPair<int, QString> > &l) { d = l; return *this; }

    int count() const { return d.count(); }

    QString title(int index) const {
        return (index >= 0 && index < d.count()) ? d.at(index).second : QString();
    }

    int width(int index) const {
        return (index >= 0 && index < d.count()) ? d.at(index).first : 100;
    }

    QList<int> widths() const
    {
        QList<int> widths;
        foreach (auto header, d) { widths << header.first; }
        return widths;
    }

private:
    QList<QPair<int, QString> > d;
};

/******************************************************************************
 ******************************************************************************/
static const Headers fileTableHeaders
({
     { 320, QLatin1String("Name")},
     { 460, QLatin1String("Path")},
     {  60, QLatin1String("Size")},
     {  60, QLatin1String("Done")},
     {  60, QLatin1String("Percent")},
     {  60, QLatin1String("First Piece")},
     {  60, QLatin1String("# Pieces")},
     { 120, QLatin1String("Pieces")}, // drawn as a progress bar
     {  60, QLatin1String("Priority")},
     { 120, QLatin1String("Modification date")},
     { 100, QLatin1String("SHA-1")},
     { 100, QLatin1String("CRC-32")}
 });

static const Headers peerTableHeaders
({
     { 120, QLatin1String("IP")},
     {  50, QLatin1String("Port")},
     { 120, QLatin1String("User Agent")},
     {  80, QLatin1String("Downloaded")},
     {  80, QLatin1String("Uploaded")},
     { 120, QLatin1String("Pieces")}, // drawn as a progress bar
     {  80, QLatin1String("Request Time")},
     {  80, QLatin1String("Active Time")},
     {  80, QLatin1String("Queue Time")},
     { 160, QLatin1String("Flags")},
     { 100, QLatin1String("Source Flags")}
 });

static const Headers trackerTableHeaders
({
     { 360, QLatin1String("Url")},
     {  60, QLatin1String("Id")},
     { 240, QLatin1String("Number of listened sockets (endpoints)")},
     { 160, QLatin1String("Tier this tracker belongs to")},
     { 120, QLatin1String("Max number of failures")},
     {  80, QLatin1String("Source")},
     {  80, QLatin1String("Verified?")}
 });


/******************************************************************************
 ******************************************************************************/
FileTableViewItemDelegate::FileTableViewItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void FileTableViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                      const QModelIndex &index ) const
{
    QStyleOptionViewItem myOption = option;
    initStyleOption(&myOption, index);

    myOption.palette.setColor(QPalette::All, QPalette::Highlight, s_lightBlue);
    myOption.palette.setColor(QPalette::All, QPalette::HighlightedText, s_black);

    if (index.column() == 7) {
        const int progress = index.data(AbstractTorrentTableModel::ProgressRole).toInt();

        QBitArray segments = index.data(AbstractTorrentTableModel::SegmentRole).toBitArray();

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
        progressBarOption.color = progress < 100 ? s_green : s_darkGreen;
        progressBarOption.icon = QIcon();

        progressBarOption.hasSegments = true;
        progressBarOption.segments = segments;

        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    } else {
        QStyledItemDelegate::paint(painter, myOption, index);
    }
}

/******************************************************************************
 ******************************************************************************/
PeerTableViewItemDelegate::PeerTableViewItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void PeerTableViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    initStyleOption(&myOption, index);

    if (index.column() == 5) {
        const int progress = index.data(AbstractTorrentTableModel::ProgressRole).toInt();
        QBitArray segments = index.data(AbstractTorrentTableModel::SegmentRole).toBitArray();

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
        progressBarOption.color = progress < 100 ? s_purple : s_darkPurple;
        progressBarOption.icon = QIcon();

        progressBarOption.hasSegments = true;
        progressBarOption.segments = segments;

        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

/******************************************************************************
 ******************************************************************************/
TorrentWidget::TorrentWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::TorrentWidget)
  , m_torrentContext(Q_NULLPTR)
  , m_torrent(Q_NULLPTR)
{
    ui->setupUi(this);

    m_fileColumnsWidths    = fileTableHeaders.widths();
    m_peerColumnsWidths    = peerTableHeaders.widths();
    m_trackerColumnsWidths = trackerTableHeaders.widths();

    setupUiTableView(ui->fileTableView);
    setupUiTableView(ui->peerTableView);
    setupUiTableView(ui->trackerTableView);

    ui->fileTableView->setItemDelegate(new FileTableViewItemDelegate(this));
    ui->peerTableView->setItemDelegate(new PeerTableViewItemDelegate(this));

    setupContextMenus();
    setupInfoCopy();

    resetUi();
}

TorrentWidget::~TorrentWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
ITorrentContext *TorrentWidget::torrentContext() const
{
    return m_torrentContext;
}

void TorrentWidget::setTorrentContext(ITorrentContext *torrentContext)
{
    m_torrentContext = torrentContext;
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::clear()
{
    m_torrent = Q_NULLPTR;
    resetUi();
}

bool TorrentWidget::isEmpty() const
{
    return m_torrent == Q_NULLPTR;
}

/******************************************************************************
 ******************************************************************************/
Torrent *TorrentWidget::torrent() const
{
    return m_torrent;
}

void TorrentWidget::setTorrent(Torrent *torrent)
{
    if (m_torrent == torrent) {
        return;
    }
    if (m_torrent) {
        disconnect(m_torrent, &Torrent::changed, this, &TorrentWidget::onChanged);
    }
    m_torrent = torrent;
    if (m_torrent) {
        connect(m_torrent, &Torrent::changed, this, &TorrentWidget::onChanged);
    }
    resetUi();
}

/******************************************************************************
 ******************************************************************************/
QByteArray TorrentWidget::saveState(int version) const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << 0xff; // VersionMarker
    stream << version;
    stream << ui->tabWidget->currentIndex();
    stream << m_fileColumnsWidths;
    stream << m_peerColumnsWidths;
    stream << m_trackerColumnsWidths;
    return data;
}

bool TorrentWidget::restoreState(const QByteArray &state, int version)
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
    int currentTabIndex = 0;
    stream >> currentTabIndex;
    ui->tabWidget->setCurrentIndex(currentTabIndex);
    stream >> m_fileColumnsWidths;
    stream >> m_peerColumnsWidths;
    stream >> m_trackerColumnsWidths;
    bool restored = true;
    return restored;
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::onChanged()
{
    updateWidget();
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::setupUiTableView(QTableView *view)
{
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->setAlternatingRowColors(false);
    view->setMidLineWidth(3);
    view->setShowGrid(false);
    view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->verticalHeader()->setHighlightSections(false);
    view->verticalHeader()->setDefaultSectionSize(C_ROW_DEFAULT_HEIGHT);
    view->verticalHeader()->setMinimumSectionSize(C_ROW_DEFAULT_HEIGHT);
    view->horizontalHeader()->setHighlightSections(false);
    view->horizontalHeader()->setDefaultSectionSize(C_COLUMN_DEFAULT_WIDTH);
    view->horizontalHeader()->setMinimumSectionSize(C_COLUMN_MINIMUM_WIDTH);
    view->verticalHeader()->setVisible(false);
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::setupInfoCopy()
{
    setupInfoCopy(ui->addedOnLabel      , ui->addedOnLineEdit);
    setupInfoCopy(ui->commentLabel      , ui->commentLineEdit);
    setupInfoCopy(ui->completedOnLabel  , ui->completedOnLineEdit);
    setupInfoCopy(ui->createdByLabel    , ui->createdByLineEdit);
    setupInfoCopy(ui->createdOnLabel    , ui->createdOnLineEdit);
    setupInfoCopy(ui->downLimitLabel    , ui->downLimitLineEdit);
    setupInfoCopy(ui->downSpeedLabel    , ui->downSpeedLineEdit);
    setupInfoCopy(ui->downloadedLabel   , ui->downloadedLineEdit);
    setupInfoCopy(ui->hashLabel         , ui->hashLineEdit);
    setupInfoCopy(ui->magnetLinkLabel   , ui->magnetLinkEdit);
    setupInfoCopy(ui->peersLabel        , ui->peersLineEdit);
    setupInfoCopy(ui->piecesLabel       , ui->piecesLineEdit);
    setupInfoCopy(ui->saveAsLabel       , ui->saveAsLineEdit);
    setupInfoCopy(ui->seedsLabel        , ui->seedsLineEdit);
    setupInfoCopy(ui->shareRatioLabel   , ui->shareRatioLineEdit);
    setupInfoCopy(ui->statusLabel       , ui->statusLineEdit);
    setupInfoCopy(ui->timeElapsedLabel  , ui->timeElapsedLineEdit);
    setupInfoCopy(ui->timeRemainingLabel, ui->timeRemainingLineEdit);
    setupInfoCopy(ui->totalSizeLabel    , ui->totalSizeLineEdit);
    setupInfoCopy(ui->upLimitLabel      , ui->upLimitLineEdit);
    setupInfoCopy(ui->upSpeedLabel      , ui->upSpeedLineEdit);
    setupInfoCopy(ui->uploadedLabel     , ui->uploadedLineEdit);
    setupInfoCopy(ui->wastedLabel       , ui->wastedLineEdit);
}

void TorrentWidget::setupInfoCopy(QLabel *label, QFrame *buddy)
{
    Q_ASSERT(label);
    Q_ASSERT(buddy);

    label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // General properties
    buddy->setFocusPolicy(Qt::StrongFocus);

    if (auto buddyLabel = qobject_cast<QLabel*>(buddy)) {
        buddyLabel->setWordWrap(true);
        buddyLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    }
    if (auto buddyTextEdit = qobject_cast<QPlainTextEdit*>(buddy)) {
        buddyTextEdit->setReadOnly(true);
        buddyTextEdit->setFrameShape(QFrame::NoFrame);
    }

    foreach (auto action, label->actions()) { label->removeAction(action); }
    foreach (auto action, buddy->actions()) { buddy->removeAction(action); }

    // Context menu > Copy
    QAction *copyAction = new QAction(tr("Copy"), buddy);
    connect(copyAction, &QAction::triggered, this, &TorrentWidget::copy);

    label->setBuddy(buddy);
    label->setContextMenuPolicy(Qt::ActionsContextMenu);
    buddy->setContextMenuPolicy(Qt::ActionsContextMenu);
    label->addAction(copyAction);
    buddy->addAction(copyAction);
}

void TorrentWidget::copy()
{
    QAction *copyAction = qobject_cast<QAction*>(sender());
    if (!copyAction) {
        return;
    }
    if (auto buddy = qobject_cast<QLabel*>(copyAction->parentWidget())) {
        QApplication::clipboard()->setText(buddy->text());

    } else if (auto buddy = qobject_cast<QPlainTextEdit*>(copyAction->parentWidget())) {
        QApplication::clipboard()->setText(buddy->toPlainText());
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::setupContextMenus()
{
    ui->fileTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->peerTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->trackerTableView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->fileTableView, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenuFileTable(const QPoint &)));
    connect(ui->peerTableView, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenuPeerTable(const QPoint &)));
    connect(ui->trackerTableView, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenuTrackerTable(const QPoint &)));

}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::showContextMenuFileTable(const QPoint &/*pos*/)
{
    QMenu *contextMenu = new QMenu(this);

    QAction actionOpen(tr("Open"), contextMenu);
    QAction actionOpenFolder(tr("Open Containing Folder"), contextMenu);
    QAction actionScan(tr("Scan for viruses"), contextMenu);
    QAction actionByOrder(tr("Priorize by File order"), contextMenu);
    QAction actionHigh(tr("Priorize: High"), contextMenu);
    QAction actionNormal(tr("Priorize: Normal"), contextMenu);
    QAction actionLow(tr("Priorize: Low"), contextMenu);
    QAction actionSkip(tr("Don't download"), contextMenu);
    QAction actionRelocate(tr("Relocate..."), contextMenu);

    connect(&actionByOrder, &QAction::triggered, this, &TorrentWidget::setPriorityByFileOrder);
    connect(&actionHigh, &QAction::triggered, this, &TorrentWidget::setPriorityHigh);
    connect(&actionNormal, &QAction::triggered, this, &TorrentWidget::setPriorityNormal);
    connect(&actionLow, &QAction::triggered, this, &TorrentWidget::setPriorityLow);
    connect(&actionSkip, &QAction::triggered, this, &TorrentWidget::setPrioritySkip);

    actionOpen.setEnabled(false);
    actionOpenFolder.setEnabled(false);
    actionScan.setEnabled(false);
    actionRelocate.setEnabled(false);

    contextMenu->addAction(&actionOpen);
    contextMenu->addAction(&actionOpenFolder);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionScan);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionByOrder);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionHigh);
    contextMenu->addAction(&actionNormal);
    contextMenu->addAction(&actionLow);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionSkip);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionRelocate);

    contextMenu->exec(QCursor::pos());
    contextMenu->deleteLater();
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::setPriorityHigh()
{
    setPriority(TorrentFileInfo::High);
}

void TorrentWidget::setPriorityNormal()
{
    setPriority(TorrentFileInfo::Normal);
}

void TorrentWidget::setPriorityLow()
{
    setPriority(TorrentFileInfo::Low);
}

void TorrentWidget::setPrioritySkip()
{
    setPriority(TorrentFileInfo::Ignore);
}

void TorrentWidget::setPriorityByFileOrder()
{
    if (!m_torrentContext) {
        return;
    }
    QModelIndexList selection = ui->fileTableView->selectionModel()->selectedRows();
    int count = ui->fileTableView->model()->rowCount();
    for (int row = 0; row < count; ++row) {
        for (int i = 0; i < selection.count(); ++i) {
            QModelIndex index = selection.at(i);
            if (row == index.row()) {
                TorrentFileInfo::Priority priority = assessPriority(row, count);
                Q_ASSERT(m_torrentContext);
                m_torrentContext->setPriority(m_torrent, row, priority);
                continue;
            }
        }
    }
}

TorrentFileInfo::Priority TorrentWidget::assessPriority(int row, int count)
{
    if (count < 3) {
        return TorrentFileInfo::Normal;
    }
    int pos = 100 * (row + 1) / count;
    if (pos < 33) {
        return TorrentFileInfo::High;
    } else if (pos < 66) {
        return TorrentFileInfo::Normal;
    } else {
        return TorrentFileInfo::Low;
    }
}

void TorrentWidget::setPriority(TorrentFileInfo::Priority priority)
{
    qDebug() << Q_FUNC_INFO << priority;
    if (!m_torrentContext) {
        return;
    }
    QModelIndexList selection = ui->fileTableView->selectionModel()->selectedRows();
    for (int i = 0; i < selection.count(); ++i) {
        QModelIndex index = selection.at(i);
        int row = index.row();
        Q_ASSERT(m_torrentContext);
        m_torrentContext->setPriority(m_torrent, row, priority);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::showContextMenuPeerTable(const QPoint &/*pos*/)
{
    QMenu *contextMenu = new QMenu(this);

    QAction actionAdd(tr("Add Peer..."), contextMenu);
    QAction actionCopy(tr("Copy Peer List"), contextMenu);

    connect(&actionAdd, &QAction::triggered, this, &TorrentWidget::addPeer);
    connect(&actionCopy, &QAction::triggered, this, &TorrentWidget::copyPeerList);

    contextMenu->addAction(&actionAdd);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionCopy);

    contextMenu->exec(QCursor::pos());
    contextMenu->deleteLater();
}

void TorrentWidget::addPeer()
{
    bool ok;
    QString input = QInputDialog::getText(
                this, tr("Add Peer"),
                tr("Enter the IP:port / [IPv6]:port of the peer to add:"),
                QLineEdit::Normal, QString(), &ok);
    if (ok && !input.isEmpty()) {
        m_torrent->addPeer(input);
    }
}

void TorrentWidget::copyPeerList()
{
    qDebug() << Q_FUNC_INFO;
    QStringList addresses;
    foreach (const TorrentPeerInfo &peer, m_torrent->detail().peers) {
        addresses << peer.endpoint.toString();
    }
    QString text;
    foreach (auto address, addresses) {
        text += address;
        text += QChar::LineFeed; // '\n'
    }
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::showContextMenuTrackerTable(const QPoint &/*pos*/)
{
    QMenu *contextMenu = new QMenu(this);

    QAction actionAdd(tr("Add Tracker..."), contextMenu);
    QAction actionRemove(tr("Remove Tracker"), contextMenu);
    QAction actionCopy(tr("Copy Tracker List"), contextMenu);

    connect(&actionAdd, &QAction::triggered, this, &TorrentWidget::addTracker);
    connect(&actionRemove, &QAction::triggered, this, &TorrentWidget::removeTracker);
    connect(&actionCopy, &QAction::triggered, this, &TorrentWidget::copyTrackerList);

    auto selected = !ui->trackerTableView->selectionModel()->selectedRows().isEmpty();
    actionRemove.setEnabled(selected);

    contextMenu->addAction(&actionAdd);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionRemove);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionCopy);

    contextMenu->exec(QCursor::pos());
    contextMenu->deleteLater();
}

void TorrentWidget::addTracker()
{
    bool ok;
    QString input = QInputDialog::getText(
                this, tr("Add Tracker"),
                tr("Enter the URL of the tracker to add:"),
                QLineEdit::Normal, QString(), &ok);
    if (ok && !input.isEmpty()) {
        m_torrent->addTracker(input);
    }
}

void TorrentWidget::removeTracker()
{
    QModelIndexList selection = ui->trackerTableView->selectionModel()->selectedRows();
    for (int i = 0; i < selection.count(); ++i) {
        QModelIndex index = selection.at(i);
        int row = index.row();
        m_torrent->removeTrackerAt(row);
    }
}

void TorrentWidget::copyTrackerList()
{
    QStringList urls;
    foreach (const TorrentTrackerInfo &tracker, m_torrent->detail().trackers) {
        urls << tracker.url;
    }
    QString text;
    foreach (auto url, urls) {
        text += url;
        text += QChar::LineFeed;
    }
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);

}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::resetUi()
{
    if (m_torrent) {
        m_torrent->retranslateUi();

        QAbstractTableModel* fileModel = m_torrent->fileModel();
        QAbstractTableModel* peerModel = m_torrent->peerModel();
        QAbstractTableModel* trackerModel = m_torrent->trackerModel();

        ui->fileTableView->setModel(fileModel);
        ui->peerTableView->setModel(peerModel);
        ui->trackerTableView->setModel(trackerModel);

        setColumnWidths(ui->fileTableView, m_fileColumnsWidths);
        setColumnWidths(ui->peerTableView, m_peerColumnsWidths);
        setColumnWidths(ui->trackerTableView, m_trackerColumnsWidths);

        // hide column 0
        ui->fileTableView->hideColumn(0);

    } else {

        getColumnWidths(ui->fileTableView, &m_fileColumnsWidths);
        getColumnWidths(ui->peerTableView, &m_peerColumnsWidths);
        getColumnWidths(ui->trackerTableView, &m_trackerColumnsWidths);

        ui->fileTableView->setModel(Q_NULLPTR);
        ui->peerTableView->setModel(Q_NULLPTR);
        ui->trackerTableView->setModel(Q_NULLPTR);
    }

    updateWidget();
}

void TorrentWidget::retranslateUi()
{
    if (m_torrent) {
        resetUi();
    }
    updateTorrentPage();
    setupInfoCopy();
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::updateWidget()
{
    if (!m_torrent) {
        ui->stackedWidget->setCurrentWidget(ui->pageGeneral);

    } else {
        if (m_torrent->metaInfo().error.type == TorrentError::NoError) {
            updateProgressBar();
            updateTorrentPage();
            ui->stackedWidget->setCurrentWidget(ui->pageTorrent);

        } else {
            ui->labelErrorMessage->setText(m_torrent->metaInfo().error.message);
            ui->stackedWidget->setCurrentWidget(ui->pageTorrentError);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::updateProgressBar()
{
    if (m_torrent && m_torrent->progress() >= 0) {
        ui->torrentProgressBar->setValue(m_torrent->progress());
        ui->torrentProgressBar->setPieces(m_torrent->info().downloadedPieces);
    } else {
        ui->torrentProgressBar->setValue(0);
        ui->torrentProgressBar->clearPieces();
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::updateTorrentPage()
{
    if (!m_torrent) {
        return;
    }
    const TorrentMetaInfo &mi = m_torrent->metaInfo();
    const TorrentInfo &ti = m_torrent->info();

    auto wasted         = tr("%0 (%1 hashfails)")
            .arg(Format::fileSizeToString(ti.bytesFailed + ti.bytesRedundant))
            .arg(Format::fileSizeToString(ti.bytesFailed));

    auto downloaded     = tr("%0 (total %1)")
            .arg(Format::fileSizeToString(ti.bytesSessionDownloaded))
            .arg(Format::fileSizeToString(ti.bytesSessionDownloaded + mi.bytesTotalDownloaded));

    auto uploaded       = tr("%0 (total %1)")
            .arg(Format::fileSizeToString(ti.bytesSessionUploaded))
            .arg(Format::fileSizeToString(ti.bytesSessionUploaded + mi.bytesTotalUploaded));

    auto seeds          = tr("%0 of %1 connected (%2 in swarm)")
            .arg(text(ti.connectedSeedsCount))
            .arg(text(ti.seedsCount))
            .arg(text(mi.seedsInSwarm));

    auto peers          = tr("%0 of %1 connected (%2 in swarm)")
            .arg(text(ti.connectedPeersCount))
            .arg(text(ti.peersCount))
            .arg(text(mi.peersInSwarm));

    auto shareRatio     = text(QString("0.000"));
    auto status         = text(m_torrent ? m_torrent->status() : QString());

    auto pieces         = tr("%0 x %1")
            .arg(text(mi.initialMetaInfo.pieceCount))
            .arg(Format::fileSizeToString(mi.initialMetaInfo.pieceByteSize));


    // GroupBox Transfer
    ui->timeElapsedLineEdit->setText(   Format::timeToString(ti.elapsedTime));
    ui->timeRemainingLineEdit->setText( Format::timeToString(ti.remaingTime));
    ui->wastedLineEdit->setText(        wasted);

    ui->downloadedLineEdit->setText(    downloaded);
    ui->downSpeedLineEdit->setText(     Format::currentSpeedToString(ti.downloadSpeed));
    ui->downLimitLineEdit->setText(     Format::currentSpeedToString(mi.downloadBandwidthLimit, true));

    ui->uploadedLineEdit->setText(      uploaded);
    ui->upSpeedLineEdit->setText(       Format::currentSpeedToString(ti.uploadSpeed));
    ui->upLimitLineEdit->setText(       Format::currentSpeedToString(mi.uploadBandwidthLimit, true));

    ui->seedsLineEdit->setText(         seeds);
    ui->peersLineEdit->setText(         peers);
    ui->shareRatioLineEdit->setText(    shareRatio);
    ui->statusLineEdit->setText(        status);

    // GroupBox General
    ui->saveAsLineEdit->setText(        text(mi.initialMetaInfo.name));
    ui->piecesLineEdit->setText         (pieces);
    ui->totalSizeLineEdit->setText(     Format::fileSizeToString(mi.initialMetaInfo.bytesTotal));
    ui->createdOnLineEdit->setText(     text(mi.initialMetaInfo.creationDate));
    ui->createdByLineEdit->setText(     text(mi.initialMetaInfo.creator));
    ui->addedOnLineEdit->setText(       text(mi.addedTime));
    ui->completedOnLineEdit->setText(   text(mi.completedTime));
    ui->hashLineEdit->setText(          text(mi.initialMetaInfo.infohash));
    ui->commentLineEdit->setText(       text(mi.initialMetaInfo.comment));
    ui->magnetLinkEdit->setPlainText(   text(mi.initialMetaInfo.magnetLink));
}

/******************************************************************************
 ******************************************************************************/
void TorrentWidget::getColumnWidths(QTableView *view, QList<int> *widths)
{
    if (view && view->model() && view->model()->columnCount() > 0) {
        widths->clear();
        for (int column = 0, count = view->model()->columnCount(); column < count; ++column) {
            auto width = view->columnWidth(column);
            widths->append(width);
        }
    }
}

void TorrentWidget::setColumnWidths(QTableView *view, const QList<int> &widths)
{
    if (view && view->model()) {
        for (int column = 0, count = view->model()->columnCount(); column < count; ++column) {
            if (column < widths.count()) {
                auto width = widths.at(column);
                view->setColumnWidth(column, width);
            } else {
                view->setColumnWidth(column, C_COLUMN_DEFAULT_WIDTH);
            }
        }
    }
}

/******************************************************************************
 ******************************************************************************/
inline QString TorrentWidget::text(int value, bool showInfiniteSymbol)
{
    const QString defaultSymbol = showInfiniteSymbol ? Format::infinity() : QString("-");
    return value < 0
            ? defaultSymbol
            : QString::number(value);
}

inline QString TorrentWidget::text(const QString &text)
{
    return text.isNull() || text.isEmpty()
            ? QString("-")
            : text;
}

inline QString TorrentWidget::text(const QDateTime &datetime)
{
    return datetime.isNull() || !datetime.isValid()
            ? QString("-")
            : datetime.toString("dd/MM/yy HH:mm:ss");
}
