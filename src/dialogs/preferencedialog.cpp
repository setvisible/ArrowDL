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

#include <Core/Settings>
#include <Widgets/PathWidget>

#include <QtCore/QSettings>
#include <QtGui/QCloseEvent>
#ifdef QT_DEBUG
#  include <QtCore/QDebug>
#endif

#define C_COLUMN_WIDTH  200

PreferenceDialog::PreferenceDialog(Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PreferenceDialog)
    , m_settings(settings)

{
    Q_ASSERT(m_settings);
    ui->setupUi(this);

    initializeGui();
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

/******************************************************************************
 ******************************************************************************/
void PreferenceDialog::initializeGui()
{
    // otherwise it's not white?
    this->setAutoFillBackground(true);
    ui->tabWidget->setAutoFillBackground(true);
    ui->tabWidget->tabBar()->setAutoFillBackground(true);

    ui->browseDatabaseFile->setPathType(PathWidget::File);
    ui->browseDatabaseFile->setSuffixName("Queue Database");
    ui->browseDatabaseFile->setSuffix(".json");

    ui->filterTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->filterTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->filterTableWidget->setHorizontalHeaderLabels(QStringList() << tr("Caption") << tr("Extensions"));
    ui->filterTableWidget->setColumnWidth(0, C_COLUMN_WIDTH);

    connect(ui->filterTableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(filterSelectionChanged()));
    connect(ui->filterCaptionLineEdit, SIGNAL(editingFinished()), this, SLOT(filterTextChanged()));
    connect(ui->filterRegexLineEdit, SIGNAL(editingFinished()), this, SLOT(filterTextChanged()));
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

    // Tab Interface
    ui->startMinimizedCheckBox->setChecked(m_settings->isStartMinimizedEnabled());
    ui->confirmRemovalCheckBox->setChecked(m_settings->isConfirmRemovalEnabled());

    // Tab Network

    // Tab Privacy
    ui->browseDatabaseFile->setCurrentPath(m_settings->database());

    // Tab Filters
    setFilters(m_settings->filters());

    // Tab Schedule

    // Tab Advanced
}

void PreferenceDialog::write()
{
    // Tab General

    // Tab Interface
    m_settings->setStartMinimizedEnabled(ui->startMinimizedCheckBox->isChecked());
    m_settings->setConfirmRemovalEnabled(ui->confirmRemovalCheckBox->isChecked());

    // Tab Network

    // Tab Privacy
    m_settings->setDatabase(ui->browseDatabaseFile->currentPath());

    // Tab Filters
    m_settings->setFilters(filters());

    // Tab Schedule

    // Tab Advanced
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
        QTableWidgetItem *item0 = new QTableWidgetItem(filter.title);
        QTableWidgetItem *item1 = new QTableWidgetItem(filter.regexp);

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
    resize(settings.value("DialogSize", QSize(350, 350)).toSize());
    const int index = settings.value("TabIndex", 0).toInt();
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
}
