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

#ifndef WIDGETS_ADVANCED_SETTINGS_WIDGET_H
#define WIDGETS_ADVANCED_SETTINGS_WIDGET_H

#include <QtWidgets/QWidget>

#include <Core/TorrentContext>

class QTreeWidgetItem;

namespace Ui {
class AdvancedSettingsWidget;
}

class AdvancedSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    enum UserRole {
        Key = Qt::UserRole,
        DefaultValue,
        KeyType
    };

    explicit AdvancedSettingsWidget(QWidget *parent = Q_NULLPTR);
    ~AdvancedSettingsWidget() Q_DECL_OVERRIDE;

    QMap<QString, QVariant> torrentSettings() const;
    void setTorrentSettings(const QMap<QString, QVariant> &map);

    QVector<int> bandwidthSettings() const;
    void setBandwidthSettings(const QVector<int> &settings);

signals:
    void changed();

protected:
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void setPresetDefault();
    void setPresetMinCache();
    void setPresetHighPerf();

    void setFilter(const QString &str);
    void showModifiedOnly(int state);

    void showContextMenu(const QPoint &pos);
    void edit(const QModelIndex &index);
    void editCurrent();
    void resetSelected();
    void format(QTreeWidgetItem *item, int column);

private:
    Ui::AdvancedSettingsWidget *ui;

    void restylizeUi();

    inline void filter();
    inline bool isModified(const QTreeWidgetItem *item) const;

    inline QTreeWidgetItem* findItem(const QString &key) const;

    void setPreset(const QList<TorrentSettingItem> &params);

    void populate();

    inline QString getKey(const QTreeWidgetItem *item) const;
    inline void setKey(QTreeWidgetItem *item, const QString &key, const QString &display);
    inline QVariant getValue(const QTreeWidgetItem *item) const;
    inline void setValue(QTreeWidgetItem *item, const QVariant &value);
    inline QVariant getDefaultValue(const QTreeWidgetItem *item) const;
    inline void setDefaultValue(QTreeWidgetItem *item, const QVariant &defaultValue);
    inline void resetToDefaultValue(QTreeWidgetItem *item);

    inline int itemToInteger(QTreeWidgetItem* item) const ;

    void setupPresetToolTip();
};

#endif // WIDGETS_ADVANCED_SETTINGS_WIDGET_H
