/* - ArrowDL - Copyright (C) 2019-present Sebastien Vavassori
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

#include "streammanager.h"

#include <Core/Settings>

#include <QtCore/QDebug>


StreamManager::StreamManager(QObject *parent) : QObject(parent)
{
}

/******************************************************************************
 ******************************************************************************/
Settings *StreamManager::settings() const
{
    return m_settings;
}

void StreamManager::setSettings(Settings *settings)
{
    if (m_settings) {
        disconnect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
    }
    m_settings = settings;
    if (m_settings) {
        connect(m_settings, SIGNAL(changed()), this, SLOT(onSettingsChanged()));
        onSettingsChanged();
    }
}

void StreamManager::onSettingsChanged()
{
    if (m_settings) {
        Stream::setConcurrentFragments(m_settings->concurrentFragments());
        Stream::setLastModifiedTimeEnabled(m_settings->isRemoteLastModifiedTimeEnabled());
        Stream::setUserAgent(m_settings->httpUserAgent());
        Stream::setConnectionProtocol(m_settings->connectionProtocol());
        Stream::setConnectionTimeout(m_settings->connectionTimeout());
    }
}

/******************************************************************************
 ******************************************************************************/
QString StreamManager::version()
{
    return Stream::version();
}

QString StreamManager::website()
{
    return Stream::website();
}
