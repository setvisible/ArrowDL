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

#include <QtCore/QCommandLineParser>

/* Enable the type to be used with QVariant. */
Q_DECLARE_METATYPE(QList<int>)

constexpr int msec_message_timeout = 2000;


#ifndef QT_DEBUG
void releaseVerboseMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg: fprintf(stderr, "Debug: %s\n", localMsg.constData()); break;
    case QtInfoMsg: fprintf(stderr, "Info: %s\n", localMsg.constData()); break;
    case QtWarningMsg: fprintf(stderr, "Warning: %s\n", localMsg.constData()); break;
    case QtCriticalMsg: fprintf(stderr, "Critical: %s\n", localMsg.constData()); break;
    case QtFatalMsg: fprintf(stderr, "Fatal: %s\n", localMsg.constData()); break;
    }
}
void releaseDefaultMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
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

    QtSingleApplication application(argc, argv);

    QCoreApplication::setApplicationName(STR_APPLICATION_NAME);
    QCoreApplication::setOrganizationName(STR_APPLICATION_ORGANIZATION);
    QCoreApplication::setApplicationVersion(STR_APPLICATION_VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(QString("\n%0").arg(QT_TRANSLATE_NOOP("main", "Another Download Manager")));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption verboseOption(QStringList() << "V" << "verbose", "Verbose (debug) mode");
    parser.addOption(verboseOption);

    QCommandLineOption interactiveOption(QStringList() << "i" << "interactive", "Interactive mode");
    parser.addOption(interactiveOption);

    parser.addPositionalArgument("url", QT_TRANSLATE_NOOP("main", "target URL to proceed"));

    parser.process(application);

    // Fix missing Title Bar icon on KDE Plasma's Wayland session.
    application.setDesktopFileName("downzemall");

#ifndef QT_DEBUG
    if (parser.isSet(verboseOption)) {
        qInstallMessageHandler(releaseVerboseMessageHandler);
    } else {
        qInstallMessageHandler(releaseDefaultMessageHandler);
    }
#else
    qInstallMessageHandler(Q_NULLPTR); // default handler (show all messages)
#endif

    QString message;
    if (parser.isSet(interactiveOption)) {
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
        bool ok = application.sendMessage(message, msec_message_timeout);
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

    return QtSingleApplication::exec();
}
