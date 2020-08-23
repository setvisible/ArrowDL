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
    ~StreamWidget() Q_DECL_OVERRIDE;

    void setStreamInfo(StreamInfo streamInfo);

signals:
    void streamInfoChanged(StreamInfo);

private slots:
    void onFormatSelected(StreamFormatId formatId);
    void onTitleChanged(const QString &);
    void onSuffixChanged(const QString &);

private:
    Ui::StreamWidget *ui;
    StreamInfo m_streamInfo;
};

#endif // WIDGETS_STREAM_WIDGET_H
