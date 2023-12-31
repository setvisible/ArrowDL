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

#include "textedit.h"

#include <QtCore/QDebug>
#include <QtCore/QMimeData>
#include <QtCore/QRegularExpression>
#include <QtCore/QtMath>
#include <QtGui/QClipboard>
#include <QtGui/QPainter>
#include <QtGui/QTextBlock>
#include <QtGui/QTextCursor>
#include <QtWidgets/QAbstractSlider>
#include <QtWidgets/QApplication>
#include <QtWidgets/QScrollBar>


BlockSelector::BlockSelector(TextEdit *parent)
    : m_editor(parent)
{
}

void BlockSelector::movePosition(int diff_line, int diff_col, MoveMode anchor)
{
    setPosition(cursorLine + diff_line, cursorColumn + diff_col, anchor);
}

int BlockSelector::cursorPosition(int blockNumber) const
{
    if (blockNumber >= topLine() && blockNumber <= bottomLine()) {
        auto block = m_editor->document()->findBlockByLineNumber(blockNumber);
        return block.position() + cursorColumn;
    }
    return -1;
}

void BlockSelector::setPosition(int position, MoveMode anchor)
{
    Q_ASSERT(m_editor);
    QTextCursor cursor = m_editor->textCursor();
    cursor.setPosition(position);
    int line = cursor.blockNumber();
    int col = cursor.positionInBlock();
    setPosition( line, col, anchor);
}

void BlockSelector::setPosition(int line, int column, MoveMode anchor)
{
    Q_ASSERT(m_editor);

    setActive(true);

    cursorLine = qBound(0, line, m_editor->document()->blockCount() - 1);
    cursorColumn = qMax(0, column);

    if (anchor != KeepAnchor) {
        anchorLine = cursorLine;
        anchorColumn = cursorColumn;
    }

    // update the cursor position
    QTextCursor cursor = m_editor->textCursor();

    if (anchor == KeepAnchor) {
        auto blockAnchor = m_editor->document()->findBlockByNumber(anchorLine);
        cursor.setPosition(blockAnchor.position()
                           + qMin(anchorColumn, blockAnchor.length() - 1 ),
                           QTextCursor::MoveAnchor );
        auto blockCursor = m_editor->document()->findBlockByNumber(cursorLine);
        cursor.setPosition(blockCursor.position()
                           + qMin(cursorColumn, blockCursor.length() - 1 ),
                           QTextCursor::KeepAnchor );
    } else {
        // MoveAnchor
        auto blockCursor = m_editor->document()->findBlockByNumber(cursorLine);
        cursor.setPosition(blockCursor.position()
                           + qMin(cursorColumn, blockCursor.length() - 1 ),
                           QTextCursor::MoveAnchor );
    }

    m_editor->setTextCursor(cursor);
    m_editor->viewport()->update();
    emit m_editor->cursorPositionChanged();
}


/******************************************************************************
 ******************************************************************************/
TextEdit::TextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(onUpdateRequest(QRect,int)));

    setCursorWidth(2);
    setBlockModeEnabled(false);
}

/******************************************************************************
 ******************************************************************************/
void TextEdit::onUpdateRequest(const QRect &/*rect*/, int /*dy*/)
{
    if (m_blockSelector.isActive()) {
        viewport()->update(); // otherwise the cursor doesn't blink
    }
}

/******************************************************************************
 ******************************************************************************/
bool TextEdit::isBlockModeEnabled() const
{
    return m_blockSelector.isActive();
}

void TextEdit::setBlockModeEnabled(bool enabled)
{
    if (isBlockModeEnabled() != enabled) {
        if (enabled) {
            if (!m_blockSelector.isActive()) {
                auto cursor = textCursor();
                m_blockSelector.setPosition(cursor.anchor() );
                m_blockSelector.setPosition(cursor.position(), BlockSelector::KeepAnchor);
            }
        } else {
            m_blockSelector.setActive(false);
        }
        viewport()->update();
        emit blockModeEnabled(enabled);
    }
}

/******************************************************************************
 ******************************************************************************/
void TextEdit::cut()
{
    if (m_blockSelector.isActive()) {
        copyBlockSelection();
        removeBlockSelection();
    } else {
        QPlainTextEdit::cut();
    }
}

void TextEdit::copy()
{
    if (m_blockSelector.isActive()) {
        copyBlockSelection();
    } else {
        QPlainTextEdit::copy();
    }
}

void TextEdit::paste()
{
    if (m_blockSelector.isActive()) {
        pasteBlockSelection();
    } else {
        QPlainTextEdit::paste();
    }
}

void TextEdit::selectAll()
{
    setBlockModeEnabled(false);
    QPlainTextEdit::selectAll();
}


/******************************************************************************
 ******************************************************************************/
void TextEdit::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Up:
    case Qt::Key_Down:
        if (e->modifiers() & Qt::ControlModifier) {
            verticalScrollBar()->triggerAction(
                        e->key() == Qt::Key_Up
                        ? QAbstractSlider::SliderSingleStepSub
                        : QAbstractSlider::SliderSingleStepAdd);
            e->accept();
            return;
        }
        // fall through
    case Qt::Key_Right:
    case Qt::Key_Left:
        if ((e->modifiers()
             & (Qt::AltModifier | Qt::ShiftModifier)) == (Qt::AltModifier | Qt::ShiftModifier)) {
            int diff_row = 0;
            int diff_col = 0;
            if (e->key() == Qt::Key_Up)          diff_row = -1;
            else if (e->key() == Qt::Key_Down)   diff_row =  1;
            else if (e->key() == Qt::Key_Left)   diff_col = -1;
            else if (e->key() == Qt::Key_Right)  diff_col =  1;
            setBlockModeEnabled(true);
            m_blockSelector.movePosition(diff_row, diff_col, BlockSelector::KeepAnchor);
            e->accept();
            return;
        } else {
            // leave block selection mode
            setBlockModeEnabled(false);
        }
        break;
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
        if (e->modifiers() == Qt::ControlModifier) {
            verticalScrollBar()->triggerAction(
                        e->key() == Qt::Key_PageUp
                        ? QAbstractSlider::SliderPageStepSub
                        : QAbstractSlider::SliderPageStepAdd);
            e->accept();
            return;
        }
        break;
    case Qt::Key_Insert:
        if (e->modifiers() == Qt::NoModifier) {
            if (overwriteMode()) {
                setOverwriteMode(false);
                viewport()->update();
            } else {
                setOverwriteMode(true);
                viewport()->update();
            }
            e->accept();
            return;
        }
        break;

    default:
        break;
    }

    bool ro = isReadOnly();

    if (!ro && m_blockSelector.isActive()) {
        QString text = e->text();
        if (!text.isEmpty() && text.at(0).isPrint()) {
            removeBlockSelection(text);
            e->accept();
            return;
        }
    }

    if (!ro && m_blockSelector.isActive()) {
        if (e == QKeySequence::Cut) {
            cut();
            e->accept();
            return;

        } else if (e == QKeySequence::Copy) {
            copy();
            e->accept();
            return;

        } else if (e == QKeySequence::Paste) {
            paste();
            e->accept();
            return;

        } else if (e == QKeySequence::SelectAll) {
            selectAll();
            e->accept();
            return;

        } else if (e == QKeySequence::Delete || e->key() == Qt::Key_Backspace) {
            if (m_blockSelector.hasSelection()) {
                removeBlockSelection();
            } else {
                // no selection, then delete 1 char after (or before) the block selector
                deleteBlockSelection( (e==QKeySequence::Delete) );
            }
            e->accept();
            return;
        }
    }

    if (!ro && m_blockSelector.isActive()) {
        QString text = e->text();
        if (!text.isEmpty() && text.at(0).isPrint()) {
            removeBlockSelection(text);
            e->accept();
            return;
        }
    }

    QPlainTextEdit::keyPressEvent(e);
}


/******************************************************************************
 ******************************************************************************/
static void fillBackground(QPainter *p, const QRectF &rect, QBrush brush, const QRectF &gradientRect = QRectF())
{
    p->save();
    if (brush.style() >= Qt::LinearGradientPattern && brush.style() <= Qt::ConicalGradientPattern) {
        if (!gradientRect.isNull()) {
            QTransform m = QTransform::fromTranslate(gradientRect.left(), gradientRect.top());
            m.scale(gradientRect.width(), gradientRect.height());
            brush.setTransform(m);
            QGradient *gradient = const_cast<QGradient *>(brush.gradient());
            gradient->setCoordinateMode(QGradient::LogicalMode);
        }
    } else {
        p->setBrushOrigin(rect.topLeft());
    }
    p->fillRect(rect, brush);
    p->restore();
}

void TextEdit::paintEvent(QPaintEvent *e)
{
    if (m_blockSelector.isActive()) {
        paintBlockSelector(e);
    } else {
        QPlainTextEdit::paintEvent(e);
    }
}

void TextEdit::paintBlockSelector(QPaintEvent *e)
{
    QPainter painter(viewport());
    Q_ASSERT(qobject_cast<QPlainTextDocumentLayout*>(document()->documentLayout()));

    QPointF offset(contentOffset());

    QRect er = e->rect();
    QRect viewportRect = viewport()->rect();

    bool editable = !isReadOnly();

    QTextBlock block = firstVisibleBlock();
    qreal maximumWidth = document()->documentLayout()->documentSize().width();

    // Set a brush origin so that the WaveUnderline knows where the wave started
    painter.setBrushOrigin(offset);

    // keep right margin clean from full-width selection
    int maxX = qFloor(offset.x() + qMax(qreal(viewportRect.width()), maximumWidth)
                      - document()->documentMargin());
    er.setRight(qMin(er.right(), maxX));
    painter.setClipRect(er);

    QAbstractTextDocumentLayout::PaintContext context = getPaintContext();
    painter.setPen(context.palette.text().color());

    while (block.isValid()) {

        QRectF r = blockBoundingRect(block).translated(offset);
        QTextLayout *layout = block.layout();

        if (!block.isVisible()) {
            offset.ry() += r.height();
            block = block.next();
            continue;
        }

        if (r.bottom() >= er.top() && r.top() <= er.bottom()) {

            QTextBlockFormat blockFormat = block.blockFormat();

            QBrush bg = blockFormat.background();
            if (bg != Qt::NoBrush) {
                QRectF contentsRect = r;
                contentsRect.setWidth(qMax(r.width(), maximumWidth));
                fillBackground(&painter, contentsRect, bg);
            }

            QVector<QTextLayout::FormatRange> selections;
            const int blpos = block.position();
            const int bllen = block.length();
            const int blnum = block.blockNumber();

            int cursorPosition = -1;
            if (context.cursorPosition > -1) {
                cursorPosition = m_blockSelector.cursorPosition(blnum);
            }

            for (const auto &range: context.selections) {
                const int selStart = range.cursor.selectionStart() - blpos;
                const int selEnd = range.cursor.selectionEnd() - blpos;
                if (selStart < bllen && selEnd > 0
                        && selEnd > selStart) {
                    QTextLayout::FormatRange o;
                    o.start = m_blockSelector.leftColumn();
                    o.length = m_blockSelector.width();
                    o.format = range.format;
                    selections.append(o);

                } else if (!range.cursor.hasSelection()
                           && range.format.hasProperty(QTextFormat::FullWidthSelection)
                           && block.contains(range.cursor.position())) {
                    // for full width selections we don't require an actual selection, just
                    // a position to specify the line. that's more convenience in usage.
                    QTextLayout::FormatRange o;
                    QTextLine l = layout->lineForTextPosition(range.cursor.position() - blpos);
                    o.start = l.textStart();
                    o.length = l.textLength();
                    if (o.start + o.length == bllen - 1)
                        ++o.length; // include newline
                    o.format = range.format;
                    selections.append(o);
                }
            }

            bool drawCursor = ((editable  || (textInteractionFlags() & Qt::TextSelectableByKeyboard))
                               && cursorPosition >= blpos
                               && cursorPosition < blpos + bllen);

            bool drawCursorAsBlock = drawCursor && overwriteMode() ;

            if (drawCursorAsBlock) {
                if (cursorPosition == blpos + bllen - 1) {
                    drawCursorAsBlock = false;
                } else {
                    QTextLayout::FormatRange o;
                    o.start = cursorPosition - blpos;
                    o.length = 1;
                    o.format.setForeground(palette().base());
                    o.format.setBackground(palette().text());
                    selections.append(o);
                }
            }

            layout->draw(&painter, offset, selections, er);

            if ((drawCursor && !drawCursorAsBlock)
                    || (editable && cursorPosition < -1
                        && !layout->preeditAreaText().isEmpty())) {
                int cpos = cursorPosition;
                if (cpos < -1)
                    cpos = layout->preeditAreaPosition() - (cpos + 2);
                else
                    cpos -= blpos;
                layout->drawCursor(&painter, offset, cpos, cursorWidth());
            }
        }

        offset.ry() += r.height();
        if (offset.y() > viewportRect.height())
            break;
        block = block.next();
    }

    if (backgroundVisible() && !block.isValid() && offset.y() <= er.bottom()
            && (centerOnScroll() || verticalScrollBar()->maximum() == verticalScrollBar()->minimum())) {
        painter.fillRect(QRect(QPoint(er.left(), qCeil(offset.y())), er.bottomRight()), palette().window());
    }
}


/******************************************************************************
 ******************************************************************************/
void TextEdit::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        auto cursorLastPosition = textCursor();
        auto cursorUnderMouse = cursorForPosition(e->pos());
        if (e->modifiers() == Qt::NoModifier) {
            // Nothing

        } else if ((e->modifiers() & Qt::AltModifier)
                   && !(e->modifiers() & Qt::ShiftModifier)) {
            // Mouse Left + Alt without Shift = block start selection

            auto line = cursorUnderMouse.blockNumber();
            auto column = qCeil(static_cast<qreal>(e->pos().x()) / QFontMetricsF(font()).horizontalAdvance(QLatin1Char(' ')));
            m_blockSelector.setPosition(line, column);
            e->accept();
            return;

        } else if ( (e->modifiers() & Qt::AltModifier)
                    && (e->modifiers() & Qt::ShiftModifier) ) {
            // Mouse Left + Alt + Shift = block selection

            QTextCursor selection = cursorLastPosition;
            selection.setPosition( cursorLastPosition.anchor() );
            if (!m_blockSelector.isActive()) {
                auto line = selection.blockNumber();
                auto column = qMin(static_cast<int>(selection.block().text().size()), selection.positionInBlock());
                m_blockSelector.setPosition(line, column);
            }
            selection.setPosition( cursorUnderMouse.position(), QTextCursor::KeepAnchor );
            auto line = cursorUnderMouse.blockNumber();
            auto column = qCeil(static_cast<qreal>(e->pos().x()) / QFontMetricsF(font()).horizontalAdvance(QLatin1Char(' ')));
            m_blockSelector.setPosition(line, column, BlockSelector::KeepAnchor);
            return;

        } else if ( !(e->modifiers() & Qt::AltModifier)
                    && (e->modifiers() & Qt::ShiftModifier) ) {
            // Mouse Left + Shift = normal selection

            setBlockModeEnabled(false);
            QTextCursor selection = cursorLastPosition;
            selection.setPosition( cursorLastPosition.anchor() );
            selection.setPosition( cursorUnderMouse.position(), QTextCursor::KeepAnchor );
            setTextCursor(selection);
            viewport()->update();
            e->accept();
            return;

        } else {
            // Mouse Left + other modifiers (Ctrl, Ctrl+Shift, ...)
            setBlockModeEnabled(false);
        }
    } else if (e->button() == Qt::RightButton) {
        auto eventCursorPosition = cursorForPosition(e->pos()).position();
        if (eventCursorPosition < textCursor().selectionStart()
                || eventCursorPosition > textCursor().selectionEnd()) {
            setTextCursor(cursorForPosition(e->pos()));
        }
    }

    QPlainTextEdit::mousePressEvent(e);
}

void TextEdit::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() == Qt::NoButton) {
        // Nothing

    } else {
        QPlainTextEdit::mouseMoveEvent(e);

        if (e->modifiers() & Qt::AltModifier) {

            setBlockModeEnabled(true);

            auto cursorUnderMouse = cursorForPosition(e->pos());
            auto line = cursorUnderMouse.blockNumber();
            auto column = qCeil(static_cast<qreal>(e->pos().x()) / QFontMetricsF(font()).horizontalAdvance(QLatin1Char(' ')));
            m_blockSelector.setPosition(line, column, BlockSelector::KeepAnchor);

        } else {
            setBlockModeEnabled(false);
        }
    }
    if (viewport()->cursor().shape() != Qt::IBeamCursor) {
        viewport()->setCursor(Qt::IBeamCursor);
    }
}

void TextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    QPlainTextEdit::mouseReleaseEvent(e);
}

void TextEdit::mouseDoubleClickEvent(QMouseEvent *e)
{  
    QPlainTextEdit::mouseDoubleClickEvent(e);
}

/******************************************************************************
 ******************************************************************************/
void TextEdit::copyBlockSelection()
{
    if (m_blockSelector.isEmpty()) {
        return;
    }

    QString text;
    for (auto i = m_blockSelector.topLine(); i <= m_blockSelector.bottomLine(); ++i) {
        auto block = document()->findBlockByLineNumber(i).text();
        block = block.leftJustified(m_blockSelector.leftColumn() + m_blockSelector.width() + 1, ' ');
        auto fragment = block.mid(m_blockSelector.leftColumn(), m_blockSelector.width());
        text.append(fragment);
        text.append("\n");
    }
    auto mimeData = new QMimeData();
    mimeData->setText(text);
    QApplication::clipboard()->setMimeData(mimeData);
}

void TextEdit::pasteBlockSelection()
{
    if (m_blockSelector.isEmpty()) {
        return;
    }

    auto clipboard = QApplication::clipboard();
    auto mimeData = clipboard->mimeData();

    QString fragment;
    if (mimeData->hasHtml()) {
        fragment = fragmentToPaste(mimeData->text());
    } else {
        fragment = fragmentToPaste(mimeData->text());
    }
    removeBlockSelection(fragment);
}

QString TextEdit::fragmentToPaste(const QString &input)
{
    static QRegularExpression reLineCarriage("[\\r\\n]");
    auto list = input.split(reLineCarriage, Qt::KeepEmptyParts);
    if (!list.isEmpty()) {
        return list.first();
    }
    return {};
}

void TextEdit::removeBlockSelection(const QString &text)
{
    if (m_blockSelector.isEmpty())
        return;

    auto cursor = textCursor();
    cursor.beginEditBlock();

    auto topLine     = m_blockSelector.topLine();
    auto bottomLine  = m_blockSelector.bottomLine();
    auto leftColumn  = m_blockSelector.leftColumn();
    auto rightColumn = m_blockSelector.rightColumn();

    auto block     = document()->findBlockByNumber( topLine );
    auto lastBlock = document()->findBlockByNumber( bottomLine );

    for (;;) { // for each selected line
        auto endPos = qMin( rightColumn, block.length() - 1 ) ;
        if (leftColumn < block.length() - 1 ) {
            cursor.setPosition( block.position() + leftColumn, QTextCursor::MoveAnchor);
            cursor.setPosition( block.position() + endPos, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        } else {
            cursor.setPosition( block.position()+block.length()-1, QTextCursor::MoveAnchor);
            QString str = "";
            str.fill(' ', leftColumn-block.length()+1 ); // str == "          "
            cursor.insertText( str );
        }
        if (!text.isEmpty()) {
            cursor.insertText( text );
        }
        if (block == lastBlock) {
            break;
        }
        block = block.next();
    }

    if (!text.isEmpty()) {
        leftColumn = leftColumn + static_cast<int>(text.length());
    }
    auto anchorLine = m_blockSelector.anchorLine;
    auto cursorLine = m_blockSelector.cursorLine;
    m_blockSelector.setPosition(anchorLine, leftColumn, BlockSelector::MoveAnchor);
    m_blockSelector.setPosition(cursorLine, leftColumn, BlockSelector::KeepAnchor);

    cursor.endEditBlock();
}


void TextEdit::deleteBlockSelection(bool after)
{
    if (m_blockSelector.isEmpty())
        return;

    auto cursor = textCursor();
    cursor.beginEditBlock();

    auto topLine     = m_blockSelector.topLine();
    auto bottomLine  = m_blockSelector.bottomLine();
    auto leftColumn  = m_blockSelector.leftColumn();

    auto block     = document()->findBlockByNumber( topLine );
    auto lastBlock = document()->findBlockByNumber( bottomLine );

    for (;;) { // for each selected line
        if (leftColumn < block.length() - 1 ) {
            cursor.setPosition( block.position() + leftColumn, QTextCursor::MoveAnchor);
            if (after){
                cursor.deleteChar();
            } else {
                if (leftColumn > 0) {
                    cursor.deletePreviousChar();
                }
            }
        }
        if (block == lastBlock) {
            break;
        }
        block = block.next();
    }
    if (!after) {
        --leftColumn;
    }

    auto anchorLine = m_blockSelector.anchorLine;
    auto cursorLine = m_blockSelector.cursorLine;
    m_blockSelector.setPosition(anchorLine, leftColumn, BlockSelector::MoveAnchor);
    m_blockSelector.setPosition(cursorLine, leftColumn, BlockSelector::KeepAnchor);

    cursor.endEditBlock();
}

void TextEdit::clearBlockSelection()
{
    if (m_blockSelector.isEmpty()) {
        return;
    }

    m_blockSelector.clear();
    auto cursor = textCursor();
    cursor.clearSelection();
    setTextCursor(cursor);
}
