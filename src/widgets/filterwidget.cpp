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

#include "filterwidget.h"
#include "ui_filterwidget.h"

#include <cstdarg> /* C++ Variadic arguments */

#include <QtCore/QtMath>
#include <QtCore/QDebug>

#define C_CHECKBOX_COUNT 7

static const uint encode(int count, ...)
{
    qDebug() << Q_FUNC_INFO;
    uint code = 0;
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i) {
        QCheckBox *checkBox = va_arg(args, QCheckBox*);
        if (checkBox->isChecked()) {
            code |= (1 << i);
        }
    }
    va_end(args);
    return code;
}

static void decode(uint code, int count, ...)
{
    qDebug() << Q_FUNC_INFO;

    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i) {
        QCheckBox *checkBox = va_arg(args, QCheckBox*);
        checkBox->setChecked(code & (1 << i));
    }
    va_end(args);
}


FilterWidget::FilterWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::FilterWidget)
{
    ui->setupUi(this);

    connect(ui->checkBox_all, SIGNAL(stateChanged(int)), this, SLOT(onFilterChanged(int)));
    connect(ui->checkBox_allImages, SIGNAL(stateChanged(int)), this, SLOT(onFilterChanged(int)));
    connect(ui->checkBox_allVideo, SIGNAL(stateChanged(int)), this, SLOT(onFilterChanged(int)));
    connect(ui->checkBox_gif, SIGNAL(stateChanged(int)), this, SLOT(onFilterChanged(int)));
    connect(ui->checkBox_jpeg, SIGNAL(stateChanged(int)), this, SLOT(onFilterChanged(int)));
    connect(ui->checkBox_png, SIGNAL(stateChanged(int)), this, SLOT(onFilterChanged(int)));
    connect(ui->fastFilteringOnlyCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onFilterChanged(int)));
    connect(ui->fastFilteringComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(onFilterChanged(QString)));
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
    return encode(C_CHECKBOX_COUNT,
                  ui->checkBox_all,
                  ui->checkBox_allImages,
                  ui->checkBox_allVideo,
                  ui->checkBox_gif,
                  ui->checkBox_jpeg,
                  ui->checkBox_png,
                  ui->fastFilteringOnlyCheckBox);
}

/*!
 * Sets the state of all the QCheckBox in the widget.
 *
 * It's a convenient method for (re)storing the QCheckBox's widget settings.
 *
 */
void FilterWidget::setState(uint code)
{
    decode(code,
           C_CHECKBOX_COUNT,
           ui->checkBox_all,
           ui->checkBox_allImages,
           ui->checkBox_allVideo,
           ui->checkBox_gif,
           ui->checkBox_jpeg,
           ui->checkBox_png,
           ui->fastFilteringOnlyCheckBox);
}

/******************************************************************************
 ******************************************************************************/
QString FilterWidget::text() const
{
    return ui->fastFilteringComboBox->currentText();
}

void FilterWidget::setText(const QString &text)
{
    if (ui->fastFilteringComboBox->currentText() != text) {
        ui->fastFilteringComboBox->setCurrentText(text);
        emit regexChanged(regex());
    }
}

/******************************************************************************
 ******************************************************************************/
void FilterWidget::onFilterChanged(int)
{
    emit regexChanged(regex());
}

void FilterWidget::onFilterChanged(const QString &)
{
    emit regexChanged(regex());
}

/******************************************************************************
 ******************************************************************************/
QRegExp FilterWidget::regex() const
{
    const QString text = ui->fastFilteringComboBox->currentText();
    QStringList parts = text.split(QRegExp("\\W+"), QString::SkipEmptyParts);

    if (!ui->fastFilteringOnlyCheckBox->isChecked()) {
        bool all = ui->checkBox_all->isChecked();
        if (all || ui->checkBox_allImages->isChecked()) {
            parts << ".gif" << ".jpg" << ".jpeg" << ".png" << ".bmp";
        }
        if (all || ui->checkBox_allVideo->isChecked()) {
            parts << ".mpeg" << ".avi" << ".divx" << ".mp4" << ".flv";
        }
        if (all || ui->checkBox_gif->isChecked()) {
            parts << ".gif";
        }
        if (all || ui->checkBox_jpeg->isChecked()) {
            parts << ".jpeg" << ".jpg";
        }
        if (all || ui->checkBox_png->isChecked()) {
            parts << ".png";
        }
    }

    QString filter;
    foreach (auto part, parts) {
        filter += QRegExp::escape(part) + "|";
    }
    if (filter.endsWith('|')) {
        filter.chop(1);
    }

    QRegExp regex(filter);
    regex.setPatternSyntax(QRegExp::RegExp);
    return regex;
}
