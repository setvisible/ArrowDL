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

#ifndef WIDGETS_FILTER_WIDGET_H
#define WIDGETS_FILTER_WIDGET_H

#include <QtCore/QRegularExpression>
#include <QtWidgets/QWidget>

class QCheckBox;

namespace Ui {
class FilterWidget;
}

class FilterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FilterWidget(QWidget *parent);
    ~FilterWidget() override;

    void clearFilters();
    void addFilter(const QString &name, const QString &regexp);

    QRegularExpression regex() const;

    uint state() const;
    void setState(uint code);

    QString currentFilter() const;
    void setCurrentFilter(const QString &text);

    QStringList filterHistory() const;
    void setFilterHistory(const QStringList &filters);

signals:
    void regexChanged(const QRegularExpression &regex);

private slots:
    void onFilterChanged(int);
    void onFilterChanged(const QString &);
    void onFilterTipToolReleased();
    void onFilterTipToolLinkActivated(const QString& link);

private:
    Ui::FilterWidget *ui;

    inline QList<QCheckBox*> allCheckBoxes() const;
};

#endif // WIDGETS_FILTER_WIDGET_H
