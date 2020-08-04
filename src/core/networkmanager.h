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

#ifndef CORE_NETWORK_MANAGER_H
#define CORE_NETWORK_MANAGER_H

#include <QtCore/QObject>
#include <QtCore/QString>

class Settings;

class QNetworkAccessManager;
class QNetworkReply;

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent);
    ~NetworkManager() Q_DECL_OVERRIDE;

    Settings* settings() const;
    void setSettings(Settings *settings);

    QNetworkReply* get(const QUrl &url);

    static QStringList proxyTypeNames();

private slots:
    void onSettingsChanged();
    void onMetaDataChanged();
    void onRedirected(const QUrl &url);

private:
    /* Network parameters (SSL, Proxy, UserAgent...) */
    QNetworkAccessManager *m_networkAccessManager;
    Settings *m_settings;

    void setProxySettings(Settings *settings);
};

#endif // CORE_NETWORK_MANAGER_H
