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

#include "globals.h"
#include "mainwindow.h"

#include <QtSingleApplication>

#include <Widgets/CustomStyle>

#include <QtCore/QCommandLineParser>
#include <QtCore/QMetaType>
#include <QtCore/QUrl>

Q_DECLARE_METATYPE(QList<int>)

int main(int argc, char *argv[])
{
    QtSingleApplication application(argc, argv);

    application.setStyle(new CustomStyle);

    QCoreApplication::setApplicationName(STR_APPLICATION_NAME);
    QCoreApplication::setOrganizationName(STR_APPLICATION_ORGANIZATION);
    QCoreApplication::setApplicationVersion(STR_APPLICATION_VERSION);

    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");

    QCommandLineParser parser;
    parser.setApplicationDescription(QT_TRANSLATE_NOOP("main", "\nAnother Download Manager"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("url", QT_TRANSLATE_NOOP("main", "target URL to proceed."));

    parser.process(application);

    const QStringList positionalArguments = parser.positionalArguments();
    const QString arg = !positionalArguments.isEmpty() ? positionalArguments.first() : QString();

    if (application.isRunning()) {
        qInfo("Another instance is running...");
        /*
         * Rem: Even if 'arg' is empty, the message is still sent to activates the window.
         */
        bool ok = application.sendMessage(arg, 2000);
        if (!ok) {
            qCritical("Message sending failed; the application may be frozen.");
        }
        return EXIT_SUCCESS;
    }

    MainWindow window;
    window.show();

    if (!arg.isEmpty()) {
        window.openWizard(QUrl(arg));
    }

    application.setActivationWindow(&window);
    QObject::connect(&application, SIGNAL(messageReceived(const QString&)),
                     &window, SLOT(handleMessage(const QString&)));

    return application.exec();
}
