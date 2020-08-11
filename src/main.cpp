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

#include "globals.h"
#include "mainwindow.h"

#include <QtSingleApplication>
#include <Ipc/InterProcessCommunication>
#include <Widgets/CustomStyle>

#include <QtCore/QCommandLineParser>

Q_DECLARE_METATYPE(QList<int>)

#ifndef QT_DEBUG
void releaseMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        // In release mode, ignore debug messages but show fatal and warning.
        break;
    case QtInfoMsg: fprintf(stderr, "Info: %s\n", localMsg.constData()); break;
    case QtWarningMsg: fprintf(stderr, "Warning: %s\n", localMsg.constData()); break;
    case QtCriticalMsg: fprintf(stderr, "Critical: %s\n", localMsg.constData()); break;
    case QtFatalMsg: fprintf(stderr, "Fatal: %s\n", localMsg.constData()); break;
    }
}
#endif

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);
#ifndef QT_DEBUG
    qInstallMessageHandler(releaseMessageHandler);
#else
    qInstallMessageHandler(Q_NULLPTR); // default handler
#endif

    QtSingleApplication application(argc, argv);

    application.setStyle(new CustomStyle);

    QCoreApplication::setApplicationName(STR_APPLICATION_NAME);
    QCoreApplication::setOrganizationName(STR_APPLICATION_ORGANIZATION);
    QCoreApplication::setApplicationVersion(STR_APPLICATION_VERSION);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");

    QCommandLineParser parser;
    parser.setApplicationDescription(QString("\n%0").arg(QT_TRANSLATE_NOOP("main", "Another Download Manager")));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption interactiveOption(QStringList() << "i" << "interactive", "Interactive mode.");
    parser.addOption(interactiveOption);

    parser.addPositionalArgument("url", QT_TRANSLATE_NOOP("main", "target URL to proceed."));

    parser.process(application);

    QString message;
    const bool interactive = parser.isSet(interactiveOption);
    if (interactive) {

        message = InterProcessCommunication::readMessageFromLauncher();

    } else {
        const QStringList positionalArguments = parser.positionalArguments();
        foreach (auto positionalArgument, positionalArguments) {
            message += positionalArgument;
            message += QChar::Space;
        }
    }

    if (application.isRunning()) {
        qWarning("Another instance is running...");
        // Rem: Even if is empty, the message is still sent to activate the window.
        bool ok = application.sendMessage(message, 2000);
        if (!ok) {
            qCritical("Message sending failed; the application may be frozen.");
        }
        return EXIT_SUCCESS;
    }


    MainWindow window;
    window.show();

    if (!message.isEmpty()) {
        window.handleMessage(message);
    }

    application.setActivationWindow(&window);
    QObject::connect(&application, SIGNAL(messageReceived(const QString&)),
                     &window, SLOT(handleMessage(const QString&)));

    return application.exec();
}
