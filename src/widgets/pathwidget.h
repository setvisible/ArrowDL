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

#ifndef WIDGETS_PATH_WIDGET_H
#define WIDGETS_PATH_WIDGET_H

#include <QtWidgets/QWidget>

namespace Ui {
class PathWidget;
}

class PathWidget : public QWidget
{
    Q_OBJECT

public:
    enum PathType { File, Directory };

    explicit PathWidget(QWidget *parent = Q_NULLPTR);
    ~PathWidget();

    QString currentPath() const;
    void setCurrentPath(const QString &path);

    QStringList pathHistory() const;
    void setPathHistory(const QStringList &paths);

    PathType pathType() const;
    void setPathType(PathType type);

    QString suffix() const;
    void setSuffix(const QString &suffix);

    QString suffixName() const;
    void setSuffixName(const QString &suffixName);

    bool colorizeErrors() const;
    void setColorizeErrors(bool enabled);

signals:
    void currentPathChanged(QString path);

public slots:
    void clearHistory();
    void removePathfromHistory(const QString &path);

private slots:
    void onBrowseButtonReleased();
    void onCurrentTextChanged(const QString &text);
    void showContextMenu(const QPoint &pos);

private:
    Ui::PathWidget *ui;
    PathType m_pathType;
    QString m_suffix;
    QString m_suffixName;
    bool m_colorizeErrorsEnabled;

    inline void colorizeErrors(const QString &text);
};

#endif // WIDGETS_PATH_WIDGET_H
