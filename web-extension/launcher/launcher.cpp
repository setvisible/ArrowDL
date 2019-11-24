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

#include <iostream>     /* std::cout, std::cin */
#include <sstream>      /* std::stringstream */
#include <string>       /* std::string */
#include <algorithm>    /* str.erase(std::remove(str.begin(), str.end(), 'a'), str.end()); */
#include <fcntl.h>      /* for _O_TEXT and _O_BINARY */
#include <math.h>       /* pow */
#include <windows.h>


/* Constants */
static const std::string C_LAUNCH("\"launch ");
static const std::string C_PROCESS = ".\\..\\DownZemAll.exe";

static const std::string C_COMPRESSION_PREFIX("[CURRENT_URL]");
static const std::string C_COMPRESSION_SUFFIX("[LINKS]");
static const std::string C_COMPRESSION_COMMAND("[OPEN_URL] ");
static const std::string C_COMPRESSION_ERROR("[ERROR_PARSE_URL] ");


static std::string quote(const std::string &str)
{
    return "\"" + str + "\"";
}

static std::string unquote(const std::string &str)
{
    std::string unquoted(str);
    unquoted.erase(std::remove(unquoted.begin(), unquoted.end(), '\"'), unquoted.end());
    return unquoted;
}

static void sendDataToExtension(const std::string &message)
{
    _setmode(_fileno(stdout), O_BINARY);
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

static std::string openStandardStreamIn()
{
    std::cout.setf(std::ios_base::unitbuf);
    _setmode(_fileno(stdin), _O_BINARY);
    int c = 0;
    unsigned int t = 0;
    std::size_t pos = 0;
    std::size_t m = 0;
    std::string inp;
    inp = "";
    t = 0;
    for (int i = 0; i <= 3; i++) {
        t += (unsigned int)pow(256.0f, i) * getchar();
    }
    for (int i = 0; i < (int)t; i++) {
        c = getchar();
        inp += (char) c;
    }
    return inp;
}

static boolean sendCommandToProcess(const std::string &process, const std::string &arguments)
{
    std::stringstream stream;
    stream << quote(process)
           << " "
           << arguments;

    const std::string fullCommand = stream.str();

    LPSTR szCmdline = const_cast<char *>(fullCommand.c_str());

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    int flags = CREATE_BREAKAWAY_FROM_JOB; // Start independent process

    BOOL ok = CreateProcess(
                NULL,           // No module name (use command line)
                szCmdline,      // Command line
                NULL,           // Process handle not inheritable
                NULL,           // Thread handle not inheritable
                FALSE,          // Set handle inheritance to FALSE
                flags,          // creation flags
                NULL,           // Use parent's environment block
                NULL,           // Use parent's starting directory
                &si,            // Pointer to STARTUPINFO structure
                &pi             // Pointer to PROCESS_INFORMATION structure
                );
    return ok == TRUE;
}

std::string compress(const std::string &command)
{
    const std::size_t start = command.find(C_COMPRESSION_PREFIX);
    const std::size_t end = command.find(C_COMPRESSION_SUFFIX);

    std::string compressed = C_COMPRESSION_COMMAND;

    if (start != std::string::npos && end != std::string::npos) {
        const std::size_t pos = start + C_COMPRESSION_PREFIX.length();
        const std::size_t n = end - pos;
        compressed += command.substr(pos, n);

    } else {
        compressed += C_COMPRESSION_ERROR;
    }
    return compressed;
}

int main(int argc, char* argv[])
{
    std::string input = "";
    while ((input = openStandardStreamIn()) != "") {

        try {
            if (input.compare(0, C_LAUNCH.length(), C_LAUNCH) == 0) {

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
    return 0;
}
