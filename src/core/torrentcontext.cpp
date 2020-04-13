/* - DownZemAll! - Copyright (C) 2019-2020 Sebastien Vavassori
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

#include "torrentcontext_p.h"

#include <Core/Settings>
#include <Core/Settings>

#include <QtCore/QObject>
#include <QtCore/QDebug>

#include "libtorrent/version.hpp"
#include "libtorrent/settings_pack.hpp"


TorrentContext& TorrentContext::getInstance()
{
    static TorrentContext instance; // lazy singleton, instantiated on first use
    return instance;
}

TorrentContext::TorrentContext() : QObject()
  , d(new TorrentContextPrivate(this))
{
}

TorrentContext::~TorrentContext()
{
    qDebug() << Q_FUNC_INFO;
}

/******************************************************************************
 ******************************************************************************/
static inline QString get_setting_key(int s)
{
    const char* name = lt::name_for_setting(s);
    if (name) {
        return QString::fromUtf8(name);
    }
    return QString();
}

QString TorrentContext::upload_rate_limit()
{
    return get_setting_key(lt::settings_pack::upload_rate_limit);
}

QString TorrentContext::download_rate_limit()
{
    return get_setting_key(lt::settings_pack::download_rate_limit);
}

QString TorrentContext::connections_limit()
{
    return get_setting_key(lt::settings_pack::connections_limit);
}

QString TorrentContext::unchoke_slots_limit()
{
    return get_setting_key(lt::settings_pack::unchoke_slots_limit);
}

/******************************************************************************
 ******************************************************************************/
QString TorrentContext::version()
{
    return QString::fromUtf8(libtorrent::version());
}

QString TorrentContext::website()
{
    return QString("libtorrent");
}

/******************************************************************************
 ******************************************************************************/
Settings* TorrentContext::settings() const
{
    return d->settings;
}

void TorrentContext::setSettings(Settings *settings)
{
    if (d->settings) {
        disconnect(d->settings, &Settings::changed, d, &TorrentContextPrivate::onSettingsChanged);
        disconnect(d->settings, SIGNAL(changed()), this, SIGNAL(changed()));
    }
    d->settings = settings;
    if (d->settings) {
        connect(d->settings, &Settings::changed, d, &TorrentContextPrivate::onSettingsChanged);
        connect(d->settings, SIGNAL(changed()), this, SIGNAL(changed()));

        d->onSettingsChanged();
    }
}

/******************************************************************************
 ******************************************************************************/
QList<TorrentSettingItem> TorrentContext::allSettingsKeysAndValues() const
{
    return d->allSettingsKeysAndValues();
}

QList<TorrentSettingItem> TorrentContext::presetDefault() const
{
    return d->presetDefault();
}

QList<TorrentSettingItem> TorrentContext::presetMinCache() const
{
    return d->presetMinCache();
}

QList<TorrentSettingItem> TorrentContext::presetHighPerf() const
{
    return d->presetHighPerf();
}

/******************************************************************************
 ******************************************************************************/
bool TorrentContext::isEnabled() const
{
    return d->settings && d->settings->isTorrentEnabled();
}

void TorrentContext::setEnabled(bool enabled)
{
    if (d->settings) {
        d->settings->setTorrentEnabled(enabled);
    }
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::addTorrent(DownloadTorrentItem *item)
{
    qDebug() << Q_FUNC_INFO;
    d->addTorrentUrl(item);
}

void TorrentContext::removeTorrent(DownloadTorrentItem *item)
{
    d->removeTorrent(item);
}

/******************************************************************************
 ******************************************************************************/
bool TorrentContext::hasTorrent(DownloadTorrentItem *item)
{
    return d->hasTorrent(item);
}

/******************************************************************************
 ******************************************************************************/
void TorrentContext::resumeTorrent(DownloadTorrentItem *item)
{
    d->resumeTorrent(item);
}

void TorrentContext::pauseTorrent(DownloadTorrentItem *item)
{
    d->pauseTorrent(item);
}
