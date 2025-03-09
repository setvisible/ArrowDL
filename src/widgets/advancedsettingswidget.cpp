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

#include "advancedsettingswidget.h"
#include "ui_advancedsettingswidget.h"

#include <Core/Theme>
#include <Torrent/TorrentContext>

#include <QtCore/QDebug>
#include <QtCore/QSignalBlocker>
#include <QtGui/QAction>
#include <QtGui/QBrush>
#include <QtGui/QFont>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>


AdvancedSettingsWidget::AdvancedSettingsWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::AdvancedSettingsWidget)
{
    ui->setupUi(this);

    connect(ui->presetDefaultButton, &QPushButton::released, this, &AdvancedSettingsWidget::setPresetDefault);
    connect(ui->presetMinCacheButton, &QPushButton::released, this, &AdvancedSettingsWidget::setPresetMinCache);
    connect(ui->presetHighPerfButton, &QPushButton::released, this, &AdvancedSettingsWidget::setPresetHighPerf);

    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &AdvancedSettingsWidget::setFilter);
    connect(ui->searchClearToolButton, &QToolButton::released, ui->searchLineEdit, &QLineEdit::clear);

    connect(ui->modifiedOnlyCheckBox, &QCheckBox::stateChanged, this, &AdvancedSettingsWidget::showModifiedOnly);

    /* Context menu */
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &AdvancedSettingsWidget::showContextMenu);

    /*
     * Workaround to make column 1 only editable:
     * 1) Set the editTriggers property of the QTreeWidget to NoEditTriggers
     * 2) On inserting items, add Qt:ItemIsEditable flag to QTreeWidgetItem
     * 3) Connect QTreeWidget's "doubleClicked" signal to conditional edit slot
     */
    ui->treeWidget->setEditTriggers(QTreeWidget::NoEditTriggers);
    connect(ui->treeWidget, &QTreeWidget::doubleClicked, this, &AdvancedSettingsWidget::edit);
    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &AdvancedSettingsWidget::format);

    populate();
    setupPresetToolTip();
}

AdvancedSettingsWidget::~AdvancedSettingsWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void AdvancedSettingsWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        setupPresetToolTip();
    } else if (event->type() == QEvent::StyleChange) {
        restylizeUi();
    }
    QWidget::changeEvent(event);
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Returns the non-default settings only.
 */
QMap<QString, QVariant> AdvancedSettingsWidget::torrentSettings() const
{
    QMap<QString, QVariant> map;
    for (auto i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        auto item = ui->treeWidget->topLevelItem(i);
        if (isModified(item)) {
            auto key = getKey(item);
            auto value = getValue(item);
            map.insert(key, value);
        }
    }
    return map;
}

void AdvancedSettingsWidget::setTorrentSettings(const QMap<QString, QVariant> &map)
{
    QMapIterator<QString, QVariant> it(map);
    while (it.hasNext()) {
        it.next();
        setValue(findItem(it.key()), it.value());
    }
}

/******************************************************************************
 ******************************************************************************/
inline int AdvancedSettingsWidget::itemToInteger(QTreeWidgetItem* item) const
{
    return item ? getValue(item).toInt() : -1;
}

QVector<int> AdvancedSettingsWidget::bandwidthSettings() const
{
    QVector<int> settings = {
        itemToInteger(findItem(TorrentContext::upload_rate_limit()  )),
        itemToInteger(findItem(TorrentContext::download_rate_limit())),
        itemToInteger(findItem(TorrentContext::connections_limit()  )),
        itemToInteger(findItem(TorrentContext::unchoke_slots_limit()))
    };
    return settings;
}

void AdvancedSettingsWidget::setBandwidthSettings(const QVector<int> &settings)
{
    setValue(findItem(TorrentContext::upload_rate_limit()  ), settings.at(0));
    setValue(findItem(TorrentContext::download_rate_limit()), settings.at(1));
    setValue(findItem(TorrentContext::connections_limit()  ), settings.at(2));
    setValue(findItem(TorrentContext::unchoke_slots_limit()), settings.at(3));
}

/******************************************************************************
 ******************************************************************************/
void AdvancedSettingsWidget::showContextMenu(const QPoint &/*pos*/)
{
    auto contextMenu = new QMenu(this);

    QAction actionEdit(tr("Edit"), contextMenu);
    connect(&actionEdit, &QAction::triggered, this, &AdvancedSettingsWidget::editCurrent);

    QAction actionReset(tr("Reset to Default"), contextMenu);
    connect(&actionReset, &QAction::triggered, this, &AdvancedSettingsWidget::resetSelected);

    contextMenu->addAction(&actionEdit);
    contextMenu->addSeparator();
    contextMenu->addAction(&actionReset);

    contextMenu->exec(QCursor::pos());
    contextMenu->deleteLater();
}

/******************************************************************************
 ******************************************************************************/
void AdvancedSettingsWidget::edit(const QModelIndex &index)
{
    QModelIndex sibling;
    sibling = index.siblingAtColumn(1);
    ui->treeWidget->edit(sibling);
}

void AdvancedSettingsWidget::editCurrent()
{
    edit(ui->treeWidget->currentIndex());
}

/******************************************************************************
 ******************************************************************************/
void AdvancedSettingsWidget::format(QTreeWidgetItem *item, int /*column*/)
{
    auto brush = palette().text();
    auto fnt = font();
    if (isModified(item)) {
        brush.setColor(Qt::red);
        fnt.setBold(true);
    }
    for (auto col : {0, 1}) {
        item->setForeground(col, brush);
        item->setFont(col, fnt);
    }
}

/******************************************************************************
 ******************************************************************************/
void AdvancedSettingsWidget::resetSelected()
{
    for (auto item : ui->treeWidget->selectedItems()) {
        resetToDefaultValue(item);
    }
}

/******************************************************************************
 ******************************************************************************/
inline bool AdvancedSettingsWidget::isModified(const QTreeWidgetItem *item) const
{
    return getValue(item) != getDefaultValue(item);
}

/******************************************************************************
 ******************************************************************************/
void AdvancedSettingsWidget::setFilter(const QString &/*str*/)
{
    filter();
}

void AdvancedSettingsWidget::showModifiedOnly(int /*state*/)
{
    filter();
}

void AdvancedSettingsWidget::filter()
{
    auto searchText = ui->searchLineEdit->text();
    auto mustBeFound = !searchText.isEmpty();
    auto mustBeModified = ui->modifiedOnlyCheckBox->isChecked();

    for (auto i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        auto item = ui->treeWidget->topLevelItem(i);
        auto text = item->data(0, Qt::DisplayRole).toString();
        auto isFound = text.contains(searchText, Qt::CaseInsensitive);

        auto found    = !mustBeFound || isFound;
        auto modified = !mustBeModified || isModified(item);
        auto visible  = found && modified;
        item->setHidden(!visible);
    }
}

/******************************************************************************
 ******************************************************************************/
void AdvancedSettingsWidget::setPresetDefault()
{
    setPreset(TorrentContext::getInstance().presetDefault());
}

void AdvancedSettingsWidget::setPresetMinCache()
{
    setPreset(TorrentContext::getInstance().presetMinCache());
}

void AdvancedSettingsWidget::setPresetHighPerf()
{
    setPreset(TorrentContext::getInstance().presetHighPerf());
}

void AdvancedSettingsWidget::setPreset(const QList<TorrentSettingItem> &params)
{
    for (auto param : params) {
        auto item = findItem(param.key);
        if (item) {
            setValue(item, param.value);
        }
    }
    filter();
}

/******************************************************************************
 ******************************************************************************/
void AdvancedSettingsWidget::populate()
{
    QSignalBlocker blocker(this);
    ui->treeWidget->clear();

    auto params = TorrentContext::getInstance().allSettingsKeysAndValues();
    for (auto param : params) {
        auto item = new QTreeWidgetItem(ui->treeWidget);
        setKey(item, param.key, param.displayKey);
        setValue(item, param.value);
        setDefaultValue(item, param.defaultValue);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->treeWidget->addTopLevelItem(item);
    }

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->sortItems(0, Qt::AscendingOrder);
    blocker.unblock();
    emit changed();
}

/******************************************************************************
 ******************************************************************************/
inline QString AdvancedSettingsWidget::getKey(const QTreeWidgetItem *item) const
{
    return item->data(0, Key).toString();
}

inline void AdvancedSettingsWidget::setKey(QTreeWidgetItem *item, const QString &key,
                                           const QString &display)
{
    item->setData(0, Key, key);
    item->setData(0, Qt::DisplayRole, display);
}

inline QVariant AdvancedSettingsWidget::getValue(const QTreeWidgetItem *item) const
{
    return item->data(1, Qt::DisplayRole);
}

inline void AdvancedSettingsWidget::setValue(QTreeWidgetItem *item, const QVariant &value)
{
    if (item) {
        auto oldValue = getValue(item);
        if (oldValue.isNull() || !oldValue.isValid() || oldValue != value) {
            item->setData(1, Qt::DisplayRole, value);
            emit changed();
        }
    }
}

inline QVariant AdvancedSettingsWidget::getDefaultValue(const QTreeWidgetItem *item) const
{
    return item->data(0, DefaultValue);
}

inline void AdvancedSettingsWidget::setDefaultValue(QTreeWidgetItem *item, const QVariant &defaultValue)
{
    item->setData(0, DefaultValue, defaultValue);
}

inline void AdvancedSettingsWidget::resetToDefaultValue(QTreeWidgetItem *item)
{
    setValue(item, getDefaultValue(item));
}

/******************************************************************************
 ******************************************************************************/
inline QTreeWidgetItem* AdvancedSettingsWidget::findItem(const QString &key) const
{
    for (auto i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        auto item = ui->treeWidget->topLevelItem(i);
        if (key == getKey(item)) {
            return item;
        }
    }
    qWarning("Can't find setting key '%s'.", key.toLatin1().data());
    return nullptr;
}

/******************************************************************************
 ******************************************************************************/
void AdvancedSettingsWidget::setupPresetToolTip()
{
    QList<QPair<QString, QString>> presets = {
        { tr("Default")              , tr("Settings optimized for a regular bittorrent client running on a desktop system.") },
        { tr("Minimize Memory Usage"), tr("Settings intended for embedded devices. It will significantly reduce memory usage.") },
        { tr("High Performance Seed"), tr("Settings optimized for a seed box, serving many peers and that doesn't do any downloading.") }
    };
    QString tooltip;
    tooltip += "<html><head/><body>";
    for (auto preset : presets) {
        tooltip +=
                "<p>"
                "<span style=\" font-weight:600;\">- "
                + preset.first +
                "</span><br/>"
                + preset.second +
                "</p>";
    }
    tooltip += "</body></html>";
    ui->presetHelp->setToolTip(tooltip);
}

/******************************************************************************
 ******************************************************************************/
void AdvancedSettingsWidget::restylizeUi()
{
    Theme::setIcons(this, { {ui->presetHelp, "help"} });
}
