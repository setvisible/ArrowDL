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

#ifndef WIDGETS_TEXT_EDIT_H
#define WIDGETS_TEXT_EDIT_H

#include <QtWidgets/QPlainTextEdit>

class TextEdit;

class BlockSelector
{
public:
    enum MoveMode{MoveAnchor = 0, KeepAnchor};

    explicit BlockSelector(TextEdit *parent);

    bool isActive() const { return m_isActive; }
    void setActive(const bool active) { m_isActive = active; }

    bool isEmpty() const  { return (anchorLine == cursorLine) && (anchorColumn == cursorColumn); }
    void clear() { anchorLine = cursorLine; anchorColumn = cursorColumn; }

    int cursorPosition(int blockNumber) const;
    void setPosition(int position , MoveMode anchor = MoveAnchor);
    void setPosition(int line, int column, MoveMode anchor = MoveAnchor);
    void movePosition(int diff_line, int diff_col, MoveMode anchor = MoveAnchor);

    int topLine() const     { return qMin(anchorLine, cursorLine); }
    int bottomLine() const  { return qMax(anchorLine, cursorLine); }

    int leftColumn() const  { return qMin(anchorColumn, cursorColumn); }
    int rightColumn() const { return qMax(anchorColumn, cursorColumn); }

    int width() const { return qAbs(cursorColumn - anchorColumn); }
    int height() const { return qAbs(cursorLine - anchorLine) + 1; }

    bool hasSelection() const { return (anchorColumn != cursorColumn); }

private:
    TextEdit *m_editor;
    bool m_isActive;

public:
    int anchorLine; // public for easy access
    int anchorColumn;
    int cursorLine;
    int cursorColumn;
};


class TextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit TextEdit(QWidget *parent = Q_NULLPTR);
    ~TextEdit() override = default;

    bool isBlockModeEnabled() const;

    static QString fragmentToPaste(const QString &input);

signals:
    void blockModeEnabled(bool enabled);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;

public slots:
    void setBlockModeEnabled(bool enabled);
    virtual void copy();
    virtual void paste();
    virtual void cut();
    virtual void selectAll();

private slots:
    void onUpdateRequest(const QRect &, int dy);

private:
    BlockSelector m_blockSelector = BlockSelector(this);

    void paintBlockSelector(QPaintEvent *e);

    void copyBlockSelection();
    void pasteBlockSelection();
    void removeBlockSelection(const QString &text = QString());
    void deleteBlockSelection(bool after = true);
    void clearBlockSelection();
};

#endif // WIDGETS_TEXT_EDIT_H
