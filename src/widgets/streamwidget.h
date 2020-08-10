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

#include <QtCore/QMap>
#include <QtWidgets/QWidget>

class QRadioButton;
class QToolButton;
class QComboBox;

namespace Ui {
class StreamWidget;
}

class StreamWidget : public QWidget
{
    Q_OBJECT
public:
    enum State {
        Empty,
        Downloading,
        Normal,
        Error
    };

    explicit StreamWidget(QWidget *parent);
    ~StreamWidget() Q_DECL_OVERRIDE;

    void clear();

    State state() const;
    void setState(State state);

    void setErrorMessage(QString errorMessage);
    void setStreamInfo(StreamInfoPtr streamInfo);

    QString selectedFormatId() const;
    void setSelectedFormatId(const QString &format_id);

    QString fileName() const;
    qint64 fileSize() const;
    QString fileExtension() const;

private slots:
    void updateButtonBar();
    void onCurrentIndexChanged(int index);
    void onChanged();

private:
    Ui::StreamWidget *ui;
    State m_state = Empty;

    StreamInfoPtr m_streamInfo;

    void clearDetectedFormat();
    void populateDefaultFormats(const QList<StreamFormat*> &formats);
    QRadioButton* appendDetectedFormat(const QString &text);

    void populateComboBox(const QList<StreamFormat*> &formats, QComboBox *comboBox);

    void selectRadio(const QString &id);
    void selectAudioComboBoxItem(const QString &id);
    void selectVideoComboBoxItem(const QString &id);

    QString selectedRadio() const;
    QString selectedAudioComboBoxItem() const;
    QString selectedVideoComboBoxItem() const;
};

#endif // WIDGETS_STREAM_WIDGET_H
