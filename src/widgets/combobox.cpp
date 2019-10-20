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

#include "combobox.h"

#include <QtWidgets/QAction>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>

#include <QtCore/QDebug>

#define MAX_HISTORY_COUNT 10

/*!
 * \class ComboBox
 *
 * An extended QComboBox with:
 * \li Button 'Clean History' in contextual menu
 * \li Error highlighting (colorization)
 *
 */
ComboBox::ComboBox(QWidget *parent) : QComboBox(parent)
  , m_colorizePtr(Q_NULLPTR)
{
    setDuplicatesEnabled(false);
    setMaxCount(MAX_HISTORY_COUNT);

    connect(this, SIGNAL(currentTextChanged(QString)), this, SLOT(onCurrentTextChanged(QString)));
    connect(this, SIGNAL(currentIndexChanged(QString)), this, SLOT(onCurrentTextChanged(QString)));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));
}

ComboBox::~ComboBox()
{
}

/******************************************************************************
 ******************************************************************************/
QStringList ComboBox::history() const
{
    QStringList history;
    for (int i = 0; i < count(); ++i) {
        history.append(itemText(i));
    }
    return history;
}

void ComboBox::setHistory(const QStringList &history)
{
    const QString text = currentText();
    clear();
    int i = qMin(MAX_HISTORY_COUNT, history.count());
    while (i > 0) {
        i--;
        const QString item = history.at(i);
        addToHistory(item);
    }
    setCurrentText(text);
}

/******************************************************************************
 ******************************************************************************/
void ComboBox::addToHistory(const QString &text)
{
    removeFromHistory(text);
    if (!text.isEmpty()) {
        insertItem(0, text);
    }
}

void ComboBox::removeFromHistory(const QString &text)
{
    int i = count();
    while (i > 0) {
        i--;
        if (itemText(i) == text) {
            removeItem(i);
        }
    }
}

/******************************************************************************
 ******************************************************************************/
QString ComboBox::currentText() const
{
    return QComboBox::currentText();
}

void ComboBox::setCurrentText(const QString &text)
{
    if (text.isEmpty() && count() > 0) {
        setCurrentIndex(0);
    } else {
        addToHistory(text);
        QComboBox::setCurrentText(text);
    }
}

/******************************************************************************
 ******************************************************************************/
/**
 * Set the given pointer (functor) to the callback method for colorization.
 *
 * The callback method defines the colorization criteria:
 * \li If it returns true, the combobox hightlights an error.
 * \li If it returns false, the combobox doesn't hightlight any error.
 *
 * To disable the coloration, just pass a null pointer (Q_NULLPTR) as argument,
 * or a functor to a method that always returns false.
 *
 * @param functor The callback method functor, or null.
 */
void ComboBox::setColorizeErrorWhen(ColorizePtr functor)
{
    m_colorizePtr = functor;
}

/******************************************************************************
 ******************************************************************************/
/**
 * Don't use this method directly. Use setColorizeErrorWhen() instead.
 */
void ComboBox::setStyleSheet(const QString& /*styleSheet*/)
{
    Q_UNREACHABLE();
}

/******************************************************************************
 ******************************************************************************/
void ComboBox::onCurrentTextChanged(const QString &text)
{
    colorizeErrors(text);
}

/******************************************************************************
 ******************************************************************************/
void ComboBox::showContextMenu(const QPoint &/*pos*/)
{
    QMenu *contextMenu = lineEdit()->createStandardContextMenu();

    contextMenu->addSeparator();
    QAction *clearAction = contextMenu->addAction(tr("Clear History"));
    connect(clearAction, SIGNAL(triggered()), this, SLOT(clearHistory()));

    contextMenu->exec(QCursor::pos());
    contextMenu->deleteLater();
}

void ComboBox::clearHistory()
{
    const QString text = currentText();
    clear();
    setCurrentText(text);
}

/******************************************************************************
 ******************************************************************************/
inline void ComboBox::colorizeErrors(const QString &text)
{
    if (m_colorizePtr != Q_NULLPTR && m_colorizePtr(text)) {
        QComboBox::setStyleSheet(QLatin1String("QComboBox { background-color: rgb(255, 100, 100); }"));
    } else {
        QComboBox::setStyleSheet(QString());
    }
}