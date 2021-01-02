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

/* ********************************************************* */
/* To debug the Launcher, uncomment one of these lines:      */
//#define DEBUG_WEB_ADDON_TO_LAUNCHER 1
//#define DEBUG_LAUNCHER_TO_APP 1
/* ********************************************************* */


/* C Standard Library */
#include <iostream>     /* std::cout, std::cin */
#include <sstream>      /* std::stringstream */
#include <string>       /* std::string */
#include <algorithm>    /* str.erase(std::remove(str.begin(), str.end(), 'a'), str.end()); */
#include <fcntl.h>      /* for _O_TEXT and _O_BINARY */
#include <math.h>       /* pow */

/* Qt */
#include <QtCore/QtGlobal>
#include <QtCore/QProcess>
#include <QtCore/QSharedMemory>

#if defined(DEBUG_WEB_ADDON_TO_LAUNCHER)
#  include <QtCore/QDebug>
#  include <QtCore/QFile>
#  include <QtCore/QFileInfo>
#endif

#if defined(DEBUG_LAUNCHER)
#  include <QtCore/QDebug>
#endif

#ifdef Q_OS_WIN
#  include <qt_windows.h>   /* CREATE_BREAKAWAY_FROM_JOB */
#endif

#if defined(Q_CC_MSVC)
#  include <io.h>           /* _setmode */
#endif

// forward declarations
void log(const QString &message);
void log(const char* label, const std::string &message);
void log(const char* label,
         const QString &message1 = QString(),
         const QString &message2 = QString());
void log(const QString &label, const std::string &message);


// Constants: Launcher <-> Application
#include "./../../src/ipc/constants.h"

// Constants: Launcher <-> Browser
#ifdef Q_OS_WIN
static const std::string C_PROCESS              ("./DownZemAll.exe");
#elif defined(Q_OS_UNIX)
static const std::string C_PROCESS              ("./DownZemAll");
#else
#endif
static const std::string C_HAND_SHAKE_QUESTION  ("\"areyouthere");
static const std::string C_HAND_SHAKE_ANSWER    ("somewhere");
static const std::string C_LAUNCH               ("\"launch ");
static const std::string C_CHROMIUM_HEADER      ("{\"text\":");
static const std::string C_CHROMIUM_FOOTER      ("}");


static std::string unquote(const std::string &str)
{
    std::string unquoted(str);
    unquoted.erase(std::remove(unquoted.begin(), unquoted.end(), '\"'), unquoted.end());
    std::replace(unquoted.begin(), unquoted.end(), '\\', '/');
    return unquoted;
}

void mSleep(int ms)
{
    Q_ASSERT(ms > 0);
#if defined(Q_OS_WINRT)
    WaitForSingleObjectEx(GetCurrentThread(), ms, true);
#elif defined(Q_OS_WIN)
    Sleep(uint(ms));
#else
    struct timespec ts = { time_t(ms / 1000), (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif
}

void log(const char* label, const std::string &message)
{
    log(QString::fromUtf8(label), message);
}

void log(const char* label, const QString &message1, const QString &message2)
{
    log(QString("%0: %1 %2").arg(label).arg(message1).arg(message2));
}

void log(const QString &label, const std::string &message)
{
    log(QString("%0: %1").arg(label).arg(QString::fromStdString(message)));
}

void log(const QString &message)
{
#if defined(DEBUG_WEB_ADDON_TO_LAUNCHER)
    QFile file("log.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        file.write(message.toUtf8());
        file.write(QString('\n').toUtf8());
        file.close();
    }
    QFileInfo fi(file);
    qDebug() << Q_FUNC_INFO << fi.canonicalFilePath();
#else
    Q_UNUSED(message)
#endif
}


static void sendDataToExtension(const std::string &message)
{
    log(Q_FUNC_INFO, message);
#if defined(Q_OS_WIN)
    _setmode(_fileno(stdout), O_BINARY);
#elif defined(Q_OS_UNIX)
    // Nothing
#else
    // freopen("file_out.txt", "wb", stdout); /* w = write, b = binary */
#endif
    try {
        std::string json = "{\"text\": \"" + message + "\"}";
        std::string::size_type length = json.length();

        // 32-bit value containing the message length in native byte order
        std::cout
                << char(((length >> 0) & 0xFF))
                << char(((length >> 8) & 0xFF))
                << char(((length >> 16) & 0xFF))
                << char(((length >> 24) & 0xFF));

        // Serialized message using JSON, UTF-8 encoded
        std::cout << json.c_str();

        std::cout.flush();
    }
    catch (...) {
        throw;
    }
}

/*!
 * Firefox messages are of type:
 *
 *      ""launch [CURRENT_URL] https://develop/ ""
 *
 * ...but Chrome messages are of type:
 *
 *      "{"text":"launch [CURRENT_URL] https://develop/ "}"
 *       ^^^^^^^^                                        ^
 *        header                                        footer
 * So here we remove the Chrome's header and footer.
 */
static std::string cleanChromeMessage(const std::string &message)
{
    log(Q_FUNC_INFO, message);
    std::string cleaned(message);
    if (cleaned.compare(0, C_CHROMIUM_HEADER.length(), C_CHROMIUM_HEADER) == 0) {
        cleaned.erase(0, C_CHROMIUM_HEADER.length());
        cleaned.erase(cleaned.length() - C_CHROMIUM_FOOTER.length(), C_CHROMIUM_FOOTER.length());
    }
    return cleaned;
}

static std::string openStandardStreamIn()
{
    log(Q_FUNC_INFO);

    std::cout.setf(std::ios_base::unitbuf);
#if defined(Q_OS_WIN)
    _setmode(_fileno(stdin), _O_BINARY); /* Keep \n instead of \r\n */
#elif defined(Q_OS_UNIX)
    // Nothing
#else
    // freopen("file_in.txt", "rb", stdin); /* r = read, b = binary */
#endif

    int size = 0;
    for (int i = 0; i <= 3; i++) {
        size += static_cast<int>(pow(256.0f, i) * getchar());
    }

    std::string input = "";
    for (int i = 0; i < size; i++) {
        input += static_cast<char>(getchar());
    }

    input = cleanChromeMessage(input);
    return input;
}

static bool startInteractiveMode(const QString &program)
{
    log(Q_FUNC_INFO, program);

    QProcess process;
    process.setProgram(program);
    process.setArguments(QStringList() << "-i");

    /// \todo Add process.setWorkingDirectory(<current dir>); ?

    process.setStandardOutputFile(QProcess::nullDevice());
    process.setStandardErrorFile(QProcess::nullDevice());

#if defined(Q_OS_WIN)
    // Rem: Necessary on Windows
    process.setCreateProcessArgumentsModifier(
                [] (QProcess::CreateProcessArguments *args)
    {
        args->flags |= CREATE_BREAKAWAY_FROM_JOB;
    });
#endif

    // Start the Application, detached from the Launcher
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    return process.startDetached();
#else
    return process.startDetached(program, QStringList() << "-i");
#endif
}

static bool sendCommandToProcess(const QString &program, const QString &arguments)
{
    log(Q_FUNC_INFO, program, arguments);

    QSharedMemory sharedMemory;
    sharedMemory.setKey(C_SHARED_MEMORY_KEY);

    /*
     * The shared memory must be large enough to contain the
     * message and also the future reply.
     * The reply is assumed to be less than 1024 chars.
     */
    int ideal_size = qMax(1024, arguments.toUtf8().size() + 10);

    if (sharedMemory.create(ideal_size)) {

        if (sharedMemory.isAttached()) {

            if (sharedMemory.lock()) {
                // Write the message into the shared memory
                QByteArray bytes = arguments.toUtf8();

                const char *from = bytes.constData();
                void *to = sharedMemory.data();
                size_t size = static_cast<size_t>(qMin(bytes.size() + 1, sharedMemory.size()));
                memcpy(to, from, size);

                char *d_ptr = static_cast<char*>(sharedMemory.data());
                d_ptr[sharedMemory.size() - 1] = '\0';

                sharedMemory.unlock();
            }

        } else {
            log(Q_FUNC_INFO, QString("Unable to attach to the shared memory segment"));
            log(Q_FUNC_INFO, sharedMemory.errorString());
            return false;
        }
    } else {
        log(Q_FUNC_INFO, QString("Unable to create shared memory segment"));
        log(Q_FUNC_INFO, sharedMemory.errorString());

#if defined(Q_OS_UNIX)
        // Unix: QSharedMemory "owns" the shared memory segment.
        // When the last thread or process that has an instance
        // of QSharedMemory attached to a particular shared memory
        // segment detaches from the segment by destroying its
        // instance of QSharedMemory, the Unix kernel release
        // the shared memory segment. But if that last thread or
        // process crashes without running the QSharedMemory destructor,
        // the shared memory segment survives the crash.

        // Trick to detach properly
        sharedMemory.attach();
        sharedMemory.detach(); // This should delete the shm if no process use it
#endif
        return false;
    }

    // Start the Application in -i (--interactive) mode.
    bool started = startInteractiveMode(program);
    if (!started) {
        return false;
    }

    // Wait during 10 seconds for the Application replies the ACK message
    int counter = 0;
    while (counter < 10) {
        counter++;

        mSleep(1000);
        QByteArray answer;

        if (sharedMemory.lock()) {
            // Reads the shared memory
            const char* ptr = static_cast<const char*>(sharedMemory.constData());
            uint n = static_cast<uint>(sharedMemory.size());
            answer.setRawData(ptr, n);
            sharedMemory.unlock();
        }

        if (QString(answer) == C_SHARED_MEMORY_ACK_REPLY) {
            sharedMemory.detach();
            return true;
        }
    }

    sharedMemory.detach();
    return false;
}


static bool sendCommandToProcess(const std::string &program, const std::string &arguments)
{
    const QString programQt = QString::fromUtf8(program.c_str());
    const QString argumentsQt = QString::fromUtf8(arguments.c_str());
    return sendCommandToProcess(programQt, argumentsQt);
}


std::string compress(const std::string &command)
{
    log(Q_FUNC_INFO, command);
    /*
     * Try to retrieve the current Url, normally
     * between blocks C_KEYWORD_CURRENT_URL and C_KEYWORD_LINKS
     */
    const std::size_t start = command.find(C_STR_CURRENT_URL);
    const std::size_t end = command.find(C_STR_LINKS);

    std::string compressed = C_STR_OPEN_URL;

    if (start != std::string::npos && end != std::string::npos) {
        const std::size_t pos = start + C_STR_CURRENT_URL.length();
        const std::size_t n = end - pos;
        compressed += command.substr(pos, n);

    } else {
        compressed += C_STR_ERROR;
    }
    return compressed;
}

int main(int argc, char* argv[])
{
#if defined(DEBUG_LAUNCHER)
    /*
     * The code below permits to step-by-step debug the Launcher,
     * launching the Application and passing a dummy message
     * through the Shared Memory communication.
     *
     * Note that the Application must be started before the Launcher
     * with -i (--interactive) argument.
     *
     */
    std::string arguments("[LINKS] Here is a [MEDIA] dummy message [END]");
    bool ok = sendCommandToProcess(C_PROCESS, arguments);

    /*
     * The code below sends the status back to the Browser
     * and permit to verify the communication between
     * the Launcher and the Browser.
     */
    if (ok) {
        sendDataToExtension("Done");
    } else {
        sendDataToExtension("Error");
    }

    return 0;
#endif

    log(QLatin1String("******************** start ********************"));
    for (int i = 0; i < argc; ++i) {
        const std::string unquoted = unquote(argv[i]);
        log(QString("arg[%0]").arg(QString::number(i)), unquoted);
    }
    std::string input = "";
    while ((input = openStandardStreamIn()) != "") {
        log(QLatin1String("input"), input);

        try {
            if (input.compare(0, C_HAND_SHAKE_QUESTION.length(), C_HAND_SHAKE_QUESTION) == 0) {

                if (argc > 0) {
                    const std::string unquoted = unquote(argv[0]);
                    sendDataToExtension(unquoted);
                } else {
                    sendDataToExtension(C_HAND_SHAKE_ANSWER);
                }

            } else if (input.compare(0, C_LAUNCH.length(), C_LAUNCH) == 0) {

                std::string arguments(input);
                arguments.erase(0, C_LAUNCH.length());

                if (sendCommandToProcess(C_PROCESS, arguments)) {

                    const std::string unquoted = unquote(arguments);
                    sendDataToExtension("Launcher [OK] Sent: " + unquoted);

                } else {

                    const std::string compressed = compress(arguments);
                    if (sendCommandToProcess(C_PROCESS, compressed)) {
                        const std::string unquoted = unquote(compressed);
                        const std::string errorMsg("Launcher [OK] Compressed and Sent: " + unquoted);
                        sendDataToExtension(errorMsg);

                    } else {
                        const std::string errorMsg("Launcher [ERROR] Cannot find the application '"+ C_PROCESS + "'");
                        sendDataToExtension(errorMsg);
                    }
                }

            } else {
                const std::string errorMsg("Launcher [ERROR] Unknown command: " + input);
                sendDataToExtension(errorMsg);
            }

        }
        catch (...) {
            throw;
        }
    }

    log(QLatin1String("******************** end ********************"));
    return 0;
}
