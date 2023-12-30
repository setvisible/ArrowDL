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

#ifndef WIDGETS_STREAM_TOOl_BOX_H
#define WIDGETS_STREAM_TOOl_BOX_H

#include <Core/Stream>

#include <QtWidgets/QToolBox>

#define HACK_CLOSABLE_PAGES

class QAbstractButton;

namespace Ui {
class StreamToolBox;
}

class StreamToolBox : public QToolBox
{
    Q_OBJECT
public:
    enum Role {
        SubtitleLangRole = Qt::UserRole + 1, ///< String: 2-letter/4-letter language code
        SubtitleOriginRole ///< String: 'automatic', 'default' or None
    };

    explicit StreamToolBox(QWidget *parent, Qt::WindowFlags f = Qt::WindowFlags());
    ~StreamToolBox() override;

    void clear();
    void setData(const StreamObject::Data &data);

    StreamObject::Config config() const;
    void setConfig(const StreamObject::Config &config);

signals:
    void configChanged();


private slots:
    void onChanged();
    void onChangedBool(bool checked);
    void onChangedInt(int index);

    void onSubtitleToggled(bool toggled);
    void setAutoSubtitleHidden(bool hidden);

private:
    Ui::StreamToolBox *ui;

    void initializeSubtitles();
    void propagateIcons();
    bool isSubtitleAutoGenerated(const QVariant &origin) const;

#ifdef HACK_CLOSABLE_PAGES
signals:
    void _q_closeToolButton();
private slots:
    void _q_onToolButtonReleased();
    void _q_onToolButtonClosed();
private:
    QList<QAbstractButton *> m_buttons;
#endif
};

#endif // WIDGETS_STREAM_TOOl_BOX_H
