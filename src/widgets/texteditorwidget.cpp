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

#include "texteditorwidget.h"
#include "ui_texteditorwidget.h"

#include <Widgets/TextEdit>

#include <QtCore/QDebug>
#include <QtGui/QTextBlock>
#include <QtWidgets/QToolButton>


TextEditorWidget::TextEditorWidget(QWidget *parent) : QWidget(parent)
  , ui(new Ui::TextEditorWidget)
{
    ui->setupUi(this);

    /* ToolButtons connections */
    connect(ui->editundo, SIGNAL(released()), ui->textEdit, SLOT(undo()));
    connect(ui->editredo, SIGNAL(released()), ui->textEdit, SLOT(redo()));
    connect(ui->editcut, SIGNAL(pressed()), ui->textEdit, SLOT(cut()));
    connect(ui->editcopy, SIGNAL(released()), ui->textEdit, SLOT(copy()));
    connect(ui->editpaste, SIGNAL(released()), ui->textEdit, SLOT(paste()));
    connect(ui->editblockmode, SIGNAL(toggled(bool)), this, SLOT(onBlockModeToggled(bool)));

    /* Editor connections */
    connect(ui->textEdit, SIGNAL(textChanged()), this, SIGNAL(textChanged()), Qt::DirectConnection);
    connect(ui->textEdit, SIGNAL(undoAvailable(bool)), ui->editundo, SLOT(setEnabled(bool)));
    connect(ui->textEdit, SIGNAL(redoAvailable(bool)), ui->editredo, SLOT(setEnabled(bool)));
    connect(ui->textEdit, SIGNAL(copyAvailable(bool)), ui->editcut, SLOT(setEnabled(bool)));
    connect(ui->textEdit, SIGNAL(copyAvailable(bool)), ui->editcopy, SLOT(setEnabled(bool)));
    connect(ui->textEdit, SIGNAL(blockModeEnabled(bool)), ui->editblockmode, SLOT(setChecked(bool)));

    /* Setup */
    ui->editundo->setEnabled(ui->textEdit->document()->isUndoAvailable());
    ui->editredo->setEnabled(ui->textEdit->document()->isRedoAvailable());
    ui->editcut->setEnabled(false);
    ui->editcopy->setEnabled(false);
}

TextEditorWidget::~TextEditorWidget()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void TextEditorWidget::clear()
{
    ui->textEdit->clear();
}

void TextEditorWidget::append(const QString &text)
{
    ui->textEdit->appendPlainText(text);
}

int TextEditorWidget::count() const
{
    return ui->textEdit->document()->blockCount();
}

QString TextEditorWidget::at(int lineNumber) const
{
    if (lineNumber >= 0 && lineNumber < count()) {
        QTextBlock textBlock = ui->textEdit->document()->findBlockByLineNumber(lineNumber);
        return textBlock.text().simplified();
    }
    return QString();
}

/******************************************************************************
 ******************************************************************************/
bool TextEditorWidget::isModified()
{
    return ui->textEdit->document()->isModified();
}

void TextEditorWidget::setModified(bool modified)
{
    ui->textEdit->document()->setModified(modified);
}

/******************************************************************************
 ******************************************************************************/
void TextEditorWidget::onBlockModeToggled(bool checked)
{
    ui->textEdit->setBlockModeEnabled(checked);
}
