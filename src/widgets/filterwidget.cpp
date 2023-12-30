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

#include "filterwidget.h"
#include "ui_filterwidget.h"

#include <Core/Theme>
#include <Widgets/AutoCloseDialog>
#include <Widgets/FilterTip>

#include <QtCore/QDebug>
#include <QtCore/QtMath>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QMessageBox>


static uint encode(const QList<QCheckBox*> &checkboxes)
{
    auto code = 0;
    for (auto i = 0; i < checkboxes.count(); ++i) {
        auto checkBox = checkboxes.at(i);
        if (checkBox->isChecked()) {
            code |= (1 << i);
        }
    }
    return code;
}

static void decode(const uint code, const QList<QCheckBox*> &checkboxes)
{
    for (auto i = 0; i < checkboxes.count(); ++i) {
        auto checkBox = checkboxes.at(i);
        checkBox->setChecked(code & (1 << i));
    }
}

FilterWidget::FilterWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::FilterWidget)
{
    ui->setupUi(this);

    Theme::setIcons(this, { {ui->fastFilteringTipToolButton, "help"} });

    auto inputValidityPtr = [](const QString &t) {
        QRegularExpression regex(t);
        return regex.isValid();
    };
    ui->fastFilteringComboBox->setInputIsValidWhen( inputValidityPtr );

    clearFilters();

    connect(ui->fastFilteringOnlyCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onFilterChanged(int)));
    connect(ui->fastFilteringComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onFilterChanged(QString)));
    connect(ui->fastFilteringTipToolButton, SIGNAL(released()), this, SLOT(onFilterTipToolReleased()));
}

FilterWidget::~FilterWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
/*!
 * Returns an integer representing the current state of all the QCheckBox
 * in the widget.
 *
 * It's a convenient method for (re)storing the QCheckBox's widget settings.
 *
 */
uint FilterWidget::state() const
{
    return encode(allCheckBoxes());
}

/*!
 * Sets the state of all the QCheckBox in the widget.
 *
 * It's a convenient method for (re)storing the QCheckBox's widget settings.
 *
 */
void FilterWidget::setState(uint code)
{
    decode(code, allCheckBoxes());
}

inline QList<QCheckBox*> FilterWidget::allCheckBoxes() const
{
    auto checkboxes = ui->checkBoxGroup->findChildren<QCheckBox*>();
    checkboxes.prepend(ui->fastFilteringOnlyCheckBox);
    return checkboxes;
}

/******************************************************************************
 ******************************************************************************/
QString FilterWidget::currentFilter() const
{
    return ui->fastFilteringComboBox->currentText();
}

void FilterWidget::setCurrentFilter(const QString &text)
{
    if (ui->fastFilteringComboBox->currentText() != text) {
        ui->fastFilteringComboBox->setCurrentText(text);
    }
}

/******************************************************************************
 ******************************************************************************/
QStringList FilterWidget::filterHistory() const
{
    return ui->fastFilteringComboBox->history();
}

void FilterWidget::setFilterHistory(const QStringList &filters)
{
    ui->fastFilteringComboBox->setHistory(filters);
}

/******************************************************************************
 ******************************************************************************/
void FilterWidget::onFilterChanged(int state)
{
    Q_UNUSED(state)
    emit regexChanged(regex());
}

void FilterWidget::onFilterChanged(const QString &text)
{
    Q_UNUSED(text)
    emit regexChanged(regex());
}

/******************************************************************************
 ******************************************************************************/
void FilterWidget::onFilterTipToolReleased()
{
    auto tip = new FilterTip(this);

    connect(tip, SIGNAL(linkActivated(QString)), this, SLOT(onFilterTipToolLinkActivated(QString)));

    AutoCloseDialog dialog(tip, ui->fastFilteringTipToolButton);
    dialog.exec();
}

void FilterWidget::onFilterTipToolLinkActivated(const QString& link)
{
    ui->fastFilteringComboBox->setCurrentText(link);
}

/******************************************************************************
 ******************************************************************************/
void FilterWidget::clearFilters()
{
    auto checkboxes = ui->checkBoxGroup->findChildren<QCheckBox*>();
    for (auto checkbox : checkboxes) {
        ui->checkBoxGroup->layout()->removeWidget(checkbox);
        checkbox->setParent(nullptr);
        delete checkbox;
    }
}

void FilterWidget::addFilter(const QString &name, const QString &regexp)
{
    auto checkboxes = ui->checkBoxGroup->findChildren<QCheckBox*>();
    auto count = static_cast<int>(checkboxes.count());
    auto division = std::div(count, 3);
    auto row = division.quot;
    auto column = division.rem;

    auto checkbox = new QCheckBox(name, ui->checkBoxGroup);
    checkbox->setToolTip(QString("%0\n%1").arg(name, regexp));
    checkbox->setProperty("regexp", regexp);

    connect(checkbox, SIGNAL(stateChanged(int)), this, SLOT(onFilterChanged(int)));

    auto layout = dynamic_cast<QGridLayout*>(ui->checkBoxGroup->layout());
    layout->addWidget(checkbox, row, column);
}

/******************************************************************************
 ******************************************************************************/
QRegularExpression FilterWidget::regex() const
{
    QString filter;

    auto text = ui->fastFilteringComboBox->currentText();
    if (!text.isEmpty()) {
        filter += "(" + text + ")";
    }

    if (!ui->fastFilteringOnlyCheckBox->isChecked()) {
        QStringList parts;
        auto checkboxes = ui->checkBoxGroup->findChildren<QCheckBox*>();
        for (auto checkbox : checkboxes) {
            if (checkbox->isChecked()) {
                parts << checkbox->property("regexp").toString();
            }
        }
        if (!parts.isEmpty()) {
            if (!filter.isEmpty()) {
                filter += "|";
            }
            for (auto part : parts) {
                filter += "(";
                filter += part;
                filter += ")|";
            }
            if (filter.endsWith('|')) {
                filter.chop(1);
            }
        }
    }
    return QRegularExpression(filter, QRegularExpression::CaseInsensitiveOption);
}
