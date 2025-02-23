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

#include "queueviewitemdelegate.h"

#include <Constants>
#include <Core/AbstractDownloadItem>
#include <Core/MimeDatabase>
#include <Core/QueueModel>
#include <Widgets/CustomStyle>
#include <Widgets/CustomStyleOptionProgressBar>
#include <Widgets/Globals>

#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>

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

void QueueViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
        auto progress = index.data(QueueModel::ProgressRole).toInt();
        auto state = static_cast<AbstractDownloadItem::State>(
            index.data(QueueModel::StateRole).toInt());

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

QWidget *QueueViewItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    if (!index.isValid() || index.column() != COL_0_FILE_NAME) {
        return nullptr;
    }
    auto editor = new QLineEdit(parent);
    editor->setAutoFillBackground(true);
    editor->setFocusPolicy(Qt::StrongFocus);
    editor->setContentsMargins(16 + 6, 0, 0, 0);
    return editor;
}

void QueueViewItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto lineEdit = qobject_cast<QLineEdit *>(editor);
    if (lineEdit) {
        QString text = index.data(Qt::EditRole).toString();
        lineEdit->setText(text);
    }
}

void QueueViewItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /*index*/) const
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
