/* - ArrowDL - Copyright (C) 2019-present Sebastien Vavassori
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * Version 3, 29 June 2007, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WIDGETS_STREAM_WIDGET_H
#define WIDGETS_STREAM_WIDGET_H

#include <Core/Stream>

#include <QtWidgets/QWidget>


namespace Ui {
class StreamWidget;
}

class StreamWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StreamWidget(QWidget *parent);
    ~StreamWidget() override;

    void setStreamObject(const StreamObject &streamObject);

signals:
    void streamObjectChanged(StreamObject);

private slots:
    void onFormatSelected(StreamFormatId formatId);
    void onConfigChanged();
    void onTitleChanged(QString title);
    void onSuffixChanged(QString suffix);

private:
    Ui::StreamWidget *ui = nullptr;
    StreamObject m_streamObject = {};

    void updateEstimatedSize();
};

#endif // WIDGETS_STREAM_WIDGET_H
