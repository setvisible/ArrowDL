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

#include "model.h"

#include <Core/ResourceModel>
#include <Core/ResourceItem>

#include <QtCore/QDebug>

Model::Model(QObject *parent) : QObject(parent)
    , m_linkModel(new ResourceModel(this))
    , m_contentModel(new ResourceModel(this))
{
    connect(m_linkModel, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
    connect(m_contentModel, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
}

/******************************************************************************
 ******************************************************************************/
void Model::setCurrentTab(Tab currentTab)
{
    if (m_currentTab != currentTab) {
        m_currentTab = currentTab;
        emit selectionChanged();
    }
}

QList<ResourceItem*> Model::selection() const
{
    return currentModel()->selectedResourceItems();
}

/******************************************************************************
 ******************************************************************************/
ResourceModel* Model::linkModel() const
{
    return m_linkModel;
}

ResourceModel* Model::contentModel() const
{
    return m_contentModel;
}

/******************************************************************************
 ******************************************************************************/
void Model::onSelectionChanged()
{
    emit selectionChanged();
}

/******************************************************************************
 ******************************************************************************/
void Model::setDestination(const QString &destination)
{
    m_linkModel->setDestination(destination);
    m_contentModel->setDestination(destination);
}

void Model::setMask(const QString &mask)
{
    m_linkModel->setMask(mask);
    m_contentModel->setMask(mask);
}

void Model::select(const QRegExp &regex)
{
    m_linkModel->select(regex);
    m_contentModel->select(regex);
}

/******************************************************************************
 ******************************************************************************/
ResourceModel* Model::currentModel() const
{
    switch (m_currentTab) {
    case LINK:
        return m_linkModel;
        break;
    case CONTENT:
        return m_contentModel;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    return Q_NULLPTR;
}
