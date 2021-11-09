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

#include "streamtoolbox.h"
#include "ui_streamtoolbox.h"

//#include <Core/Stream>
#include <Core/Theme>

#include <QtCore/QDebug>

/******************************************************************************
 ******************************************************************************/
//StreamSubtitleTableModel::StreamSubtitleTableModel(QObject *parent) : QAbstractTableModel(parent)
//{
//}

//StreamSubtitleTableModel::~StreamSubtitleTableModel()
//{
//}

//int StreamSubtitleTableModel::rowCount(const QModelIndex &parent) const
//{
//    return 0;
//}

//int StreamSubtitleTableModel::columnCount(const QModelIndex &parent) const
//{
//    return 2;
//}

//QVariant StreamSubtitleTableModel::data(const QModelIndex &index, int role) const
//{
//    return QVariant();
//}

//QVariant StreamSubtitleTableModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    if (section == 0) {
//        return tr("Download?");
//    }
//    if (section == 1) {
//        return tr("Language");
//    }
//    return QVariant();
//}

/******************************************************************************
 ******************************************************************************/
StreamToolBox::StreamToolBox(QWidget *parent, Qt::WindowFlags f) : QToolBox(parent, f)
  , ui(new Ui::StreamToolBox)
  //  , m_subtitleModel(new StreamSubtitleTableModel(this))
{
    ui->setupUi(this);

#ifdef HACK_CLOSABLE_PAGES
    // **********************************************************************
    // HACK: closable toolbox buttons
    // ==============================
    // The QTtoolBox has a 'hidden' (last) page, whici is empty.
    // Click event of page's buttons are observed.
    // When the user clicks an already opened page, a signal is emitted as
    // 'Qt::QueuedConnection' (very important!), in order to push
    // this event *after* the QToolBox events.
    // **********************************************************************
    auto objs = this->findChildren<QAbstractButton*>();
    foreach (auto obj, objs) {
        if (obj->objectName() == QLatin1String("qt_toolbox_toolboxbutton")) {
            obj->setCursor(Qt::PointingHandCursor);
            m_buttons.append(obj);
            connect(obj, SIGNAL(released()), this, SLOT(_q_onToolButtonReleased()));
        }
    }
    m_buttons.last()->setVisible(false);
    connect(this, SIGNAL(_q_closeToolButton()), this, SLOT(_q_onToolButtonClosed()), Qt::QueuedConnection);
#endif

    //    ui->subtitleTableView->setModel(m_subtitleModel);


    /* *********************************** */
    /* Connect main signals */
    QList<QCheckBox*> checkboxes;

    // Overview
    checkboxes << ui->skipDownloadCheckBox;
    checkboxes << ui->markWatchedCheckBox;

    // Subtitles
    checkboxes << ui->subtitleCheckBox;

    // Comments
    checkboxes << ui->commentCheckBox;

    // Thumbnails
    checkboxes << ui->thumbnailCheckBox;

    // Other Media
    checkboxes << ui->otherDescriptionCheckBox;
    checkboxes << ui->otherMetadataCheckBox;
    checkboxes << ui->otherShortcutCheckBox;

    // Post-Processing

    // SponsorBlock


    foreach (auto checkbox, checkboxes) {
        connect(checkbox, SIGNAL(toggled(bool)), this, SLOT(onCheckBoxToggled(bool)));
    }
    /* *********************************** */

    /* Connect buddies signals */
    connect(ui->commentLimitCheckBox, SIGNAL(toggled(bool)),
            ui->commentLimitSpinBox, SLOT(setEnabled(bool)));


    propagateIcons();
}

StreamToolBox::~StreamToolBox()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void StreamToolBox::clear()
{
    // Don't clear() to keep the columns
    //m_model->removeRows(0, m_model->rowCount());
}


/******************************************************************************
 ******************************************************************************/
void StreamToolBox::setConfig(const StreamObject::Config &config)
{
    QSignalBlocker blocker(this);
    clear();

    // Overview
    ui->skipDownloadCheckBox->setChecked(config.overview.skipVideo);
    ui->markWatchedCheckBox->setChecked(config.overview.markWatched);

    // Subtitles
    ui->subtitleCheckBox->setChecked(config.subtitle.writeDefaultSubtitle);
    // ui->subtitleFormatComboBox->clear();
    // ui->subtitleFormatComboBox->addItems(streamObject.subtitleFormats());
    //    foreach (auto subtitle, streamObject.subtitles()) {
    //        qDebug() << subtitle;
    //    }
    // m_subtitleModel->read(streamObject)

    // Comments
    ui->commentCheckBox->setChecked(config.comment.writeComment);

    // Thumbnails
    ui->thumbnailCheckBox->setChecked(config.thumbnail.writeDefaultThumbnail);

    // Other Media
    ui->otherDescriptionCheckBox->setChecked(config.metadata.writeDescription);
    ui->otherMetadataCheckBox->setChecked(config.metadata.writeMetadata);
    ui->otherShortcutCheckBox->setChecked(config.metadata.writeInternetShortcut);

    // Post-Processing

    // SponsorBlock
}

/******************************************************************************
 ******************************************************************************/
void StreamToolBox::onCheckBoxToggled(bool checked)
{
    Q_UNUSED(checked)
    onChanged();
}

/******************************************************************************
 ******************************************************************************/
void StreamToolBox::onChanged()
{
    StreamObject::Config config;

    // Overview
    config.overview.skipVideo = ui->skipDownloadCheckBox->isChecked();
    config.overview.markWatched = ui->markWatchedCheckBox->isChecked();

    // Subtitles
    config.subtitle.writeDefaultSubtitle = ui->subtitleCheckBox->isChecked();

    // Chapters

    // Thumbnails
    config.thumbnail.writeDefaultThumbnail = ui->thumbnailCheckBox->isChecked();

    // Comments
    config.comment.writeComment = ui->commentCheckBox->isChecked();

    // Other Media
    config.metadata.writeDescription = ui->otherDescriptionCheckBox->isChecked();
    config.metadata.writeMetadata = ui->otherMetadataCheckBox->isChecked();
    config.metadata.writeInternetShortcut = ui->otherShortcutCheckBox->isChecked();

    // Processing

    // SponsorBlock

    emit configChanged(config);
}

/******************************************************************************
 ******************************************************************************/
void StreamToolBox::propagateIcons()
{
    // No Theme::setIcons(...) here.
    setItemIcon(0, QIcon::fromTheme("home")); // Overview
    setItemIcon(1, QIcon::fromTheme("stream-subtitle")); // Subtitles
    setItemIcon(2, QIcon::fromTheme("mask")); // Chapters
    setItemIcon(3, QIcon::fromTheme("stream-video"));  // Thumbnails
    setItemIcon(4, QIcon::fromTheme("rename"));  // Comment
    setItemIcon(5, QIcon::fromTheme("info")); // Other Media
    setItemIcon(6, QIcon::fromTheme("preference")); // Processing
    setItemIcon(7, QIcon::fromTheme("remove-downloaded")); // SponsorBlock
}

/******************************************************************************
 ******************************************************************************/
#ifdef HACK_CLOSABLE_PAGES
void StreamToolBox::_q_onToolButtonReleased()
{
    QAbstractButton* button = qobject_cast<QAbstractButton*>(sender());
    if (button) {
        if (button == m_buttons.last()) {
            return;
        }
        auto index = m_buttons.indexOf(button);
        if (currentIndex() == index) {
            emit _q_closeToolButton();
        }
    }
}

void StreamToolBox::_q_onToolButtonClosed()
{
    auto last = m_buttons.count() - 1;
    setCurrentIndex(last);
}
#endif
