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

#include "preferencedialog.h"
#include "ui_preferencedialog.h"

#include <Globals>
#include <Core/Locale>
#include <Core/Settings>
#include <Core/Stream>
#include <Widgets/AdvancedSettingsWidget>
#include <Widgets/PathWidget>

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QSignalBlocker>
#include <QtGui/QCloseEvent>
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocument>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSystemTrayIcon>

#define C_DEFAULT_WIDTH     700
#define C_DEFAULT_HEIGHT    500
#define C_DEFAULT_INDEX     0
#define C_COLUMN_WIDTH      200


PreferenceDialog::PreferenceDialog(Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PreferenceDialog)
    , m_settings(settings)
{
    Q_ASSERT(m_settings);
    ui->setupUi(this);

    refreshTitle();

    connectUi();
    initializeUi();
    initializeWarnings();
    read();
    readSettings();
}

PreferenceDialog::~PreferenceDialog()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void PreferenceDialog::accept()
{
    write();
    writeSettings();
    QDialog::accept();
}

void PreferenceDialog::reject()
{
    writeSettings();
    QDialog::reject();
}

void PreferenceDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        refreshTitle();
    }
    QDialog::changeEvent(event);
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::connectUi()
{
    // Tab General

    // Tab Interface
    connect(ui->localeComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(languageChanged(int)));

    // Tab Network
    connect(ui->maxSimultaneousDownloadSlider, SIGNAL(valueChanged(int)),
            this, SLOT(maxSimultaneousDownloadSlided(int)));

    // Tab Privacy
    connect(ui->browseDatabaseFile, SIGNAL(currentPathValidityChanged(bool)),
            ui->okButton, SLOT(setEnabled(bool)));

    connect(ui->streamCleanCacheButton, SIGNAL(released()),
            this, SLOT(onStreamCleanCacheButtonReleased()));

    connect(ui->checkUpdateNowPushButton, SIGNAL(released()),
            this, SIGNAL(checkUpdate()), Qt::QueuedConnection);

    // Tab Filters
    connect(ui->filterTableWidget, SIGNAL(itemSelectionChanged()),
            this, SLOT(filterSelectionChanged()));
    connect(ui->filterCaptionLineEdit, SIGNAL(editingFinished()),
            this, SLOT(filterTextChanged()));
    connect(ui->filterRegexLineEdit, SIGNAL(editingFinished()),
            this, SLOT(filterTextChanged()));

    // Tab Torrent
    connect(ui->torrentCheckBox, &QCheckBox::toggled,
            ui->torrentShareFolderGroupBox, &QGroupBox::setEnabled);
    connect(ui->torrentCheckBox, &QCheckBox::toggled,
            ui->torrentBandwidthGroupBox, &QGroupBox::setEnabled);
    connect(ui->torrentCheckBox, &QCheckBox::toggled,
            ui->torrentConnectionGroupBox, &QGroupBox::setEnabled);
    connect(ui->torrentShareFolderCheckBox, &QCheckBox::toggled,
            ui->torrentShareFolderPathWidget, &PathWidget::setEnabled);

    connect(ui->torrentUpRateSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(bandwidthSettingsChanged(int)));
    connect(ui->torrentDownRateSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(bandwidthSettingsChanged(int)));
    connect(ui->torrentConnectionMaxCountSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(bandwidthSettingsChanged(int)));
    connect(ui->torrentPeerMaxCountSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(bandwidthSettingsChanged(int)));

    // Tab Advanced
    connect(ui->advancedSettingsWidget, &AdvancedSettingsWidget::changed,
            this, &PreferenceDialog::setBandwidthSettings);
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::initializeUi()
{
    // otherwise it's not white?
    this->setAutoFillBackground(true);
    ui->tabWidget->setAutoFillBackground(true);
    ui->tabWidget->tabBar()->setAutoFillBackground(true);

    // Tab General

    // Tab Interface
    const QSignalBlocker blocker(ui->localeComboBox);
    ui->localeComboBox->clear();
    ui->localeComboBox->addItems(Locale::availableLanguages());
    ui->showSystemTrayIconCheckBox->setEnabled(QSystemTrayIcon::isSystemTrayAvailable());
    ui->hideWhenMinimizedCheckBox->setEnabled(ui->showSystemTrayIconCheckBox->isChecked());
    ui->showSystemTrayBalloonCheckBox->setEnabled(ui->showSystemTrayIconCheckBox->isChecked());

    ui->streamHostPlainTextEdit->setEnabled(ui->streamHostCheckBox->isChecked());

    // Tab Network

    // Tab Privacy
    ui->browseDatabaseFile->setPathType(PathWidget::File);
    ui->browseDatabaseFile->setSuffixName(tr("Queue Database"));
    ui->browseDatabaseFile->setSuffix(".json");

    ui->streamCleanCacheLabel->setText(
                QString("Located at <a href=\"%0\">%0</a>").arg(StreamCleanCache::cacheDir()));

    // Tab Filters
    ui->filterTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->filterTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->filterTableWidget->setHorizontalHeaderLabels(
                QStringList() << tr("Caption") << tr("Extensions"));
    ui->filterTableWidget->setColumnWidth(0, C_COLUMN_WIDTH);

    // Tab Torrent
    ui->torrentCheckBox->setChecked(true);
    ui->torrentShareFolderCheckBox->setChecked(true);
    ui->torrentShareFolderPathWidget->setPathType(PathWidget::Directory);
    setBandwidthSettings();

    // Tab Advanced
}

void PreferenceDialog::initializeWarnings()
{
    ui->systemTrayIconWarning->setVisible(!QSystemTrayIcon::isSystemTrayAvailable());
    ui->systemTrayIconWarning->setText(tr("Warning: The system tray is not available."));

    ui->systemTrayBalloonWarning->setVisible(!QSystemTrayIcon::supportsMessages());
    ui->systemTrayBalloonWarning->setText(tr("Warning: The system tray doesn't support balloon messages."));
}

void PreferenceDialog::refreshTitle()
{
    setWindowTitle(QString("%0 - %1").arg(STR_APPLICATION_NAME).arg(tr("Preferences")));
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::filterSelectionChanged()
{
    ui->filterCaptionLineEdit->clear();
    ui->filterRegexLineEdit->clear();

    QList<QTableWidgetItem *> items = ui->filterTableWidget->selectedItems();
    foreach (auto item, items) {
        if (item->column() == 0) {
            ui->filterCaptionLineEdit->setText(item->text());
        } else if (item->column() == 1) {
            ui->filterRegexLineEdit->setText(item->text());
        }
    }
}

void PreferenceDialog::filterTextChanged()
{
    QList<QTableWidgetItem *> items = ui->filterTableWidget->selectedItems();
    foreach (auto item, items) {
        if (item->column() == 0) {
            item->setText(ui->filterCaptionLineEdit->text());
        } else if (item->column() == 1) {
            item->setText(ui->filterRegexLineEdit->text());
        }
    }
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::languageChanged(int value)
{
    Locale::applyLanguage(Locale::toLanguage(value));
}

void PreferenceDialog::resetLanguage()
{
    Locale::applyLanguage(m_settings->language());
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::maxSimultaneousDownloadSlided(int value)
{
    ui->maxSimultaneousDownloadLabel->setText(QString::number(value));
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::bandwidthSettingsChanged(int /*value*/)
{
    QVector<int> settings = {
        ui->torrentUpRateSpinBox->value() * 1024,
        ui->torrentDownRateSpinBox->value() * 1024,
        ui->torrentConnectionMaxCountSpinBox->value(),
        ui->torrentPeerMaxCountSpinBox->value()
    };
    ui->advancedSettingsWidget->setBandwidthSettings(settings);
}

void PreferenceDialog::setBandwidthSettings()
{
    QSignalBlocker blocker0(ui->torrentUpRateSpinBox);
    QSignalBlocker blocker1(ui->torrentDownRateSpinBox);
    QSignalBlocker blocker2(ui->torrentConnectionMaxCountSpinBox);
    QSignalBlocker blocker3(ui->torrentPeerMaxCountSpinBox);

    QVector<int> settings = ui->advancedSettingsWidget->bandwidthSettings();
    ui->torrentUpRateSpinBox->setValue(settings.at(0) / 1024);
    ui->torrentDownRateSpinBox->setValue(settings.at(1) / 1024);
    ui->torrentConnectionMaxCountSpinBox->setValue(settings.at(2));
    ui->torrentPeerMaxCountSpinBox->setValue(settings.at(3));
}

/******************************************************************************
 ******************************************************************************/
/**
 * Application Settings
 */
void PreferenceDialog::restoreDefaultSettings()
{
    m_settings->beginRestoreDefault();
    read();
    m_settings->endRestoreDefault();
}

void PreferenceDialog::read()
{
    // Tab General
    setExistingFileOption(m_settings->existingFileOption());

    // Tab Interface
    const QSignalBlocker blocker(ui->localeComboBox);
    ui->localeComboBox->setCurrentIndex(Locale::fromLanguage(m_settings->language()));
    ui->dontShowTutorialCheckBox->setChecked(m_settings->isDontShowTutorialEnabled());
    ui->showSystemTrayIconCheckBox->setChecked(m_settings->isSystemTrayIconEnabled());
    ui->hideWhenMinimizedCheckBox->setChecked(m_settings->isHideWhenMinimizedEnabled());
    ui->showSystemTrayBalloonCheckBox->setChecked(m_settings->isSystemTrayBalloonEnabled());
    ui->confirmRemovalCheckBox->setChecked(m_settings->isConfirmRemovalEnabled());
    ui->confirmBatchCheckBox->setChecked(m_settings->isConfirmBatchDownloadEnabled());
    ui->streamHostCheckBox->setChecked(m_settings->isStreamHostEnabled());
    setStreamHosts(m_settings->streamHosts());

    // Tab Network
    ui->maxSimultaneousDownloadSlider->setValue(m_settings->maxSimultaneousDownloads());

    ui->customBatchGroupBox->setChecked(m_settings->isCustomBatchEnabled());
    ui->customBatchButtonLabelLineEdit->setText(m_settings->customBatchButtonLabel());
    ui->customBatchRangeLineEdit->setText(m_settings->customBatchRange());

    // Tab Privacy
    ui->privacyRemoveCompletedCheckBox->setChecked(m_settings->isRemoveCompletedEnabled());
    ui->privacyRemoveCanceledCheckBox->setChecked(m_settings->isRemoveCanceledEnabled());
    ui->privacyRemovePausedCheckBox->setChecked(m_settings->isRemovePausedEnabled());

    ui->browseDatabaseFile->setCurrentPath(m_settings->database());

    int index = static_cast<int>(m_settings->checkUpdateBeatMode());
    ui->checkUpdateComboBox->setCurrentIndex(index);

    // Tab Filters
    setFilters(m_settings->filters());

    // Tab Torrent
    ui->torrentCheckBox->setChecked(m_settings->isTorrentEnabled());
    ui->torrentShareFolderCheckBox->setChecked(m_settings->isTorrentShareFolderEnabled());
    ui->torrentShareFolderPathWidget->setCurrentPath(m_settings->shareFolder());
    ui->torrentPeersPlainTextEdit->setPlainText(m_settings->torrentPeers());

    // Tab Advanced
    ui->advancedSettingsWidget->setTorrentSettings(m_settings->torrentSettings());
}

void PreferenceDialog::write()
{
    // Tab General
    m_settings->setExistingFileOption(existingFileOption());

    // Tab Interface
    m_settings->setLanguage(Locale::toLanguage(ui->localeComboBox->currentIndex()));
    m_settings->setDontShowTutorialEnabled(ui->dontShowTutorialCheckBox->isChecked());
    m_settings->setSystemTrayIconEnabled(ui->showSystemTrayIconCheckBox->isChecked());
    m_settings->setHideWhenMinimizedEnabled(ui->hideWhenMinimizedCheckBox->isChecked());
    m_settings->setSystemTrayBalloonEnabled(ui->showSystemTrayBalloonCheckBox->isChecked());
    m_settings->setConfirmRemovalEnabled(ui->confirmRemovalCheckBox->isChecked());
    m_settings->setConfirmBatchDownloadEnabled(ui->confirmBatchCheckBox->isChecked());
    m_settings->setStreamHostEnabled(ui->streamHostCheckBox->isChecked());
    m_settings->setStreamHosts(streamHosts());

    // Tab Network
    m_settings->setMaxSimultaneousDownloads(ui->maxSimultaneousDownloadSlider->value());

    m_settings->setCustomBatchEnabled(ui->customBatchGroupBox->isChecked());
    m_settings->setCustomBatchButtonLabel(ui->customBatchButtonLabelLineEdit->text());
    m_settings->setCustomBatchRange(ui->customBatchRangeLineEdit->text());

    // Tab Privacy
    m_settings->setRemoveCompletedEnabled(ui->privacyRemoveCompletedCheckBox->isChecked());
    m_settings->setRemoveCanceledEnabled(ui->privacyRemoveCanceledCheckBox->isChecked());
    m_settings->setRemovePausedEnabled(ui->privacyRemovePausedCheckBox->isChecked());

    m_settings->setDatabase(ui->browseDatabaseFile->currentPath());

    CheckUpdateBeatMode mode = static_cast<CheckUpdateBeatMode>(
                ui->checkUpdateComboBox->currentIndex());
    m_settings->setCheckUpdateBeatMode(mode);

    // Tab Filters
    m_settings->setFilters(filters());

    // Tab Torrent
    m_settings->setTorrentEnabled(ui->torrentCheckBox->isChecked());
    m_settings->setTorrentShareFolderEnabled(ui->torrentShareFolderCheckBox->isChecked());
    m_settings->setShareFolder(ui->torrentShareFolderPathWidget->currentPath());
    m_settings->setTorrentPeers(ui->torrentPeersPlainTextEdit->toPlainText());

    // Tab Advanced
    m_settings->setTorrentSettings(ui->advancedSettingsWidget->torrentSettings());
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::setStreamHosts(const QStringList &streamHosts)
{
    ui->streamHostPlainTextEdit->clear();
    foreach (auto streamHost, streamHosts) {
        ui->streamHostPlainTextEdit->appendPlainText(streamHost);
    }

    // Scroll to top
    ui->streamHostPlainTextEdit->moveCursor(QTextCursor::Start);
    ui->streamHostPlainTextEdit->ensureCursorVisible();
}

QStringList PreferenceDialog::streamHosts() const
{
    QStringList streamHosts;
    auto doc = ui->streamHostPlainTextEdit->document();
    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next()) {
        streamHosts.append(it.text().trimmed());
    }
    return streamHosts;
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::onStreamCleanCacheButtonReleased()
{
    ui->streamCleanCacheButton->setText(tr("Cleaning..."));
    ui->streamCleanCacheButton->setEnabled(false);

    StreamCleanCache *s = new StreamCleanCache(this);
    connect(s, &StreamCleanCache::done, this, &PreferenceDialog::cleaned);
    connect(s, &StreamCleanCache::done, s, &QObject::deleteLater);
    s->runAsync();
}

void PreferenceDialog::cleaned()
{
    ui->streamCleanCacheButton->setText(tr("Clean Cache"));
    ui->streamCleanCacheButton->setEnabled(true);
}

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::setFilters(const QList<Filter> &filters)
{
    ui->filterTableWidget->clearContents();
    while (ui->filterTableWidget->rowCount()>0) {
        ui->filterTableWidget->removeRow(0);
    }

    foreach (auto filter, filters) {
        const int row = ui->filterTableWidget->rowCount();
        auto item0 = new QTableWidgetItem(filter.title);
        auto item1 = new QTableWidgetItem(filter.regexp);

        ui->filterTableWidget->insertRow(row);

        ui->filterTableWidget->setItem(row, 0, item0);
        ui->filterTableWidget->setItem(row, 1, item1);
    }
}

QList<Filter> PreferenceDialog::filters() const
{
    QList<Filter> filters;
    for (int row = 0; row < ui->filterTableWidget->rowCount(); ++row) {
        const QString title = ui->filterTableWidget->item(row, 0)->text();
        const QString regexp = ui->filterTableWidget->item(row, 1)->text();
        Filter filter;
        filter.title = title;
        filter.regexp = regexp;
        filters.append(filter);
    }
    return filters;
}

/******************************************************************************
 ******************************************************************************/
/**
 * Preference Dialog Settings
 */
void PreferenceDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("Preference");

    const QSize defaultSize(C_DEFAULT_WIDTH, C_DEFAULT_HEIGHT);
    resize(settings.value("DialogSize", defaultSize).toSize());

    const int index = settings.value("TabIndex", C_DEFAULT_INDEX).toInt();
    if (index >=0 && index < ui->tabWidget->count()) {
        ui->tabWidget->setCurrentIndex(index);
    }
    settings.endGroup();
}

void PreferenceDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Preference");
    settings.setValue("DialogSize", size());
    settings.setValue("TabIndex", ui->tabWidget->currentIndex());
    settings.endGroup();

    resetLanguage();
}

/******************************************************************************
 ******************************************************************************/
ExistingFileOption PreferenceDialog::existingFileOption() const
{
    if (ui->renameRadioButton->isChecked()) {
        return ExistingFileOption::Rename;
    }
    if (ui->overwriteRadioButton->isChecked()) {
        return ExistingFileOption::Overwrite;
    }
    if (ui->skipRadioButton->isChecked()) {
        return ExistingFileOption::Skip;
    }
    if (ui->askRadioButton->isChecked()) {
        return ExistingFileOption::Ask;
    }
    Q_UNREACHABLE();
    return ExistingFileOption::LastOption;
}

void PreferenceDialog::setExistingFileOption(ExistingFileOption option)
{
    switch (option) {
    case ExistingFileOption::Rename:
        ui->renameRadioButton->setChecked(true);
        break;
    case ExistingFileOption::Overwrite:
        ui->overwriteRadioButton->setChecked(true);
        break;
    case ExistingFileOption::Skip:
        ui->skipRadioButton->setChecked(true);
        break;
    case ExistingFileOption::Ask:
        ui->askRadioButton->setChecked(true);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}
