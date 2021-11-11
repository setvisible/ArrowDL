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

#include <Core/Theme>

#include <QtCore/QDebug>
#include <QtWidgets/QListView>

static const QString s_default_caption = QLatin1String("default");
static const QString s_automatic_caption = QLatin1String("automatic");


StreamToolBox::StreamToolBox(QWidget *parent, Qt::WindowFlags f) : QToolBox(parent, f)
  , ui(new Ui::StreamToolBox)
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

    // **********************************************************************
    // Connect signals
    // ===============
    // Whenever a input changes, this widget *must* notify the change to its parent.
    // The parent widget (playlist) keeps the real states of the object.
    // **********************************************************************
    {
        QList<QWidget*> done;
        foreach (auto w, this->findChildren<QCheckBox*>()) {
            connect(w, SIGNAL(toggled(bool)), this, SLOT(onChangedBool(bool)));
            // done << w;
        }
        foreach (auto w, this->findChildren<QComboBox*>()) {
            connect(w, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangedInt(int)));
            // done << w;
        }
        foreach (auto w, this->findChildren<QSpinBox*>()) {
            connect(w, SIGNAL(valueChanged(int)), this, SLOT(onChangedInt(int)));
            // done << w;
        }
        foreach (auto w, this->findChildren<QLineEdit*>()) {
            connect(w, SIGNAL(editingFinished()), this, SLOT(onChanged()));
            // done << w;
        }
        // foreach (auto w, this->findChildren<QWidget*>()) {
        //   QSet<QString> undone;
        //   if (!done.contains(w)) {
        //     undone.insert(QString::fromLatin1(w->metaObject()->className()));
        //   }
        //   foreach (auto value, undone.values()) {
        //     qDebug() << Q_FUNC_INFO << "WARNING: unconnected input:" << value;
        //   }
        // }
    }

    initializeSubtitles();

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
void StreamToolBox::initializeSubtitles()
{
    QStringList supportedConvertExts{"srt", "vtt", "ass", "lrc"};
    ui->subtitleConvComboBox->clear();
    foreach (auto ext, supportedConvertExts) {
        ui->subtitleConvComboBox->addItem(ext.toUpper(), ext.toLower());
    }
    ui->subtitleConvComboBox->setCurrentIndex(0);
    ui->subtitleConvComboBox->setEnabled(false);

    connect(ui->subtitleCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(onSubtitleToggled(bool)));

    connect(ui->subtitleHideAutoCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(setAutoSubtitleHidden(bool)));
    ui->subtitleHideAutoCheckBox->setChecked(true);
    setAutoSubtitleHidden(ui->subtitleHideAutoCheckBox->isChecked());

    connect(ui->subtitleConvCheckBox, SIGNAL(toggled(bool)),
            ui->subtitleConvComboBox, SLOT(setEnabled(bool)));
    ui->subtitleConvCheckBox->setChecked(false);
    onSubtitleToggled(ui->subtitleConvCheckBox->isChecked());
}

/******************************************************************************
 ******************************************************************************/
void StreamToolBox::clear()
{
    // Don't clear() to keep the columns
}

/******************************************************************************
 ******************************************************************************/
void StreamToolBox::onSubtitleToggled(bool toggled)
{
    ui->subtitleLangComboBox->setEnabled(toggled);
    ui->subtitleExtLabel->setEnabled(toggled);
    ui->subtitleExtComboBox->setEnabled(toggled);
    ui->subtitleHideAutoCheckBox->setEnabled(toggled);
    ui->subtitleConvCheckBox->setEnabled(toggled);
    ui->subtitleConvComboBox->setEnabled(toggled && ui->subtitleConvCheckBox->isChecked());
}

void StreamToolBox::setAutoSubtitleHidden(bool hidden)
{
    auto view = qobject_cast<QListView *>(ui->subtitleLangComboBox->view());
    for (int i = 0; i < ui->subtitleLangComboBox->count(); ++i) {
        auto origin = ui->subtitleLangComboBox->itemData(i, SubtitleOriginRole);
        if (isSubtitleAutoGenerated(origin)) {
            view->setRowHidden(i, hidden);
        }
    }
}

bool StreamToolBox::isSubtitleAutoGenerated(const QVariant &origin) const
{
    return origin.isValid() && origin.toString() == s_automatic_caption;
}

/******************************************************************************
 ******************************************************************************/
void StreamToolBox::setData(const StreamObject::Data &data)
{
    // Overview
    // Subtitles
    {
        ui->subtitleLangComboBox->clear();
        ui->subtitleLangComboBox->addItem(tr("(default language)"));
        ui->subtitleLangComboBox->setItemData(0, QString(), SubtitleLangRole);
        ui->subtitleLangComboBox->addItem(tr("All languages"));
        ui->subtitleLangComboBox->setItemData(1, QLatin1String("all"), SubtitleLangRole);
        ui->subtitleLangComboBox->insertSeparator(2);
        auto subtitles = data.subtitleLanguages();
        foreach (auto subtitle, subtitles) {
            auto code = subtitle.languageCode.toUpper();
            auto label = subtitle.isAutomatic
                    ? QString("%0 (auto-generated) - %1").arg(code, subtitle.languageName)
                    : QString("%0 - %1").arg(code, subtitle.languageName);
            auto origin = subtitle.isAutomatic ? s_automatic_caption : s_default_caption;
            ui->subtitleLangComboBox->addItem(label);
            auto index = ui->subtitleLangComboBox->count() - 1;
            ui->subtitleLangComboBox->setItemData(index, subtitle.languageCode, SubtitleLangRole);
            ui->subtitleLangComboBox->setItemData(index, origin, SubtitleOriginRole);
        }
        ui->subtitleLangComboBox->setCurrentIndex(0);
        setAutoSubtitleHidden(ui->subtitleHideAutoCheckBox->isChecked());
    }
    {
        ui->subtitleExtComboBox->clear();
        ui->subtitleExtComboBox->addItem(tr("(default)"), QString());
        auto extenstions = data.subtitleExtensions();
        extenstions.sort();
        foreach (auto ext, extenstions) {
            ui->subtitleExtComboBox->addItem(ext.toUpper(), ext.toLower());
        }
        ui->subtitleExtComboBox->setCurrentIndex(0);
    }
    // Comments
    // Thumbnails
    // Other Media
    // Post-Processing
    // SponsorBlock
}

/******************************************************************************
 ******************************************************************************/
void StreamToolBox::onChanged()
{
    emit configChanged();
}

void StreamToolBox::onChangedBool(bool /*checked*/)
{
    emit configChanged();
}

void StreamToolBox::onChangedInt(int /*index*/)
{
    emit configChanged();
}

/******************************************************************************
 ******************************************************************************/
StreamObject::Config StreamToolBox::config() const
{
    StreamObject::Config config;

    // Overview
    {
        config.overview.skipVideo = ui->skipDownloadCheckBox->isChecked();
        config.overview.markWatched = ui->markWatchedCheckBox->isChecked();
    }

    // Subtitles
    {
        config.subtitle.writeSubtitle = ui->subtitleCheckBox->isChecked();
        auto langs = ui->subtitleLangComboBox->currentData(SubtitleLangRole);
        if (langs.isValid()) {
            config.subtitle.languages = langs.toString();
        }
        auto origin = ui->subtitleLangComboBox->currentData(SubtitleOriginRole);
        if (!origin.isValid()) {
            // When item don't have the tag, it's deduced from the checkbox.
            config.subtitle.isAutoGenerated = !ui->subtitleHideAutoCheckBox->isChecked();
        } else {
            auto autogen = isSubtitleAutoGenerated(ui->subtitleLangComboBox->currentData(SubtitleOriginRole));
            config.subtitle.isAutoGenerated = autogen;
        }
        config.subtitle.extensions = ui->subtitleExtComboBox->currentData().toString();
        if (ui->subtitleConvCheckBox->isChecked()) {
            config.subtitle.convert = ui->subtitleConvComboBox->currentData().toString();
        }
    }

    // Chapters

    // Thumbnails
    {
        config.thumbnail.writeDefaultThumbnail = ui->thumbnailCheckBox->isChecked();
    }

    // Comments
    {
        config.comment.writeComment = ui->commentCheckBox->isChecked();
    }

    // Other Media
    {
        config.metadata.writeDescription = ui->otherDescriptionCheckBox->isChecked();
        config.metadata.writeMetadata = ui->otherMetadataCheckBox->isChecked();
        config.metadata.writeInternetShortcut = ui->otherShortcutCheckBox->isChecked();
    }

    // Processing
    // SponsorBlock

    return config;
}

void StreamToolBox::setConfig(const StreamObject::Config &config)
{
    QSignalBlocker blocker(this);
    clear();

    // Overview
    ui->skipDownloadCheckBox->setChecked(config.overview.skipVideo);
    ui->markWatchedCheckBox->setChecked(config.overview.markWatched);

    // Subtitles
    ui->subtitleCheckBox->setChecked(config.subtitle.writeSubtitle);

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
