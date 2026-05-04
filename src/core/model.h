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

#ifndef CORE_MODEL_H
#define CORE_MODEL_H

#include <QtCore/QObject>
#include <QtCore/QList>

class ResourceModel;
class ResourceItem;

class Model : public QObject
{
    Q_OBJECT
    
public:
    enum Tab{LINK, CONTENT};

    explicit Model(QObject *parent);
    ~Model() override = default;

    ResourceModel* currentModel() const;

    void setCurrentTab(Tab tab);

    QList<ResourceItem*> selection() const;

    ResourceModel* linkModel() const;
    ResourceModel* contentModel() const;

signals:
    void selectionChanged();

public slots:
    void setDestination(const QString &destination);
    void setMask(const QString &mask);
    void select(const QRegularExpression &regex);

private:
    ResourceModel *m_linkModel = nullptr;
    ResourceModel *m_contentModel = nullptr;
    Tab m_currentTab = LINK;
};

#endif // CORE_MODEL_H
