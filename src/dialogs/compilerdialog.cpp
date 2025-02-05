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

#include "compilerdialog.h"
#include "ui_compilerdialog.h"

#include "../version_3rd.h"
#include <Constants>
#include <Core/Stream>

#include <QtCore/QDebug>
#include <QtCore/QLibrary>
#include <QtCore/QLibraryInfo>
#include <QtCore/QString>
#include <QtNetwork/QSslSocket>


#ifdef Q_OS_WIN
#  include <windows.h>
#else /* POSIX */
// \todo
#endif

CompilerDialog::CompilerDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CompilerDialog)
{
    ui->setupUi(this);

    setWindowTitle(QString("%0 - %1").arg(STR_APPLICATION_NAME, tr("Info")));

    adjustSize();
    //setFixedSize(size());

    connect(ui->okButton, &QPushButton::released, this, &CompilerDialog::onOkButtonReleased);

    auto version = tr("%0 %1 version %2").arg(STR_APPLICATION_NAME,
                                              STR_COMPILER_WORDSIZE,
                                              STR_APPLICATION_VERSION);
    ui->version->setText(version);

    ui->link->setText(QString("<a href=\"%0\">%0</a>").arg(STR_APPLICATION_WEBSITE));
    ui->link->setTextFormat(Qt::RichText);
    ui->link->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->link->setOpenExternalLinks(true);

    ui->link_github->setText(QString("<a href=\"%0\">%0</a>").arg(STR_GITHUB_REPO_ADDRESS));
    ui->link_github->setTextFormat(Qt::RichText);
    ui->link_github->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->link_github->setOpenExternalLinks(true);

    ui->compilerName->setText(STR_COMPILER_NAME);
    ui->compilerVersion->setText(STR_COMPILER_BUILD_ABI);
    ui->compilerCpuArchitecture->setText(STR_COMPILER_BUILD_CPU);
    ui->compilerBuildDate->setText(STR_APPLICATION_BUILD);

    ui->currentOS->setText(STR_CURRENT_PLATFORM);
    ui->currentVersion->setText(STR_CURRENT_VERSION);
    ui->currentCpuArchitecture->setText(STR_CURRENT_CPU);

#ifdef USE_QT_WEBENGINE
    ui->QtVersion->setText(tr("%0 with Qt WebEngine based on Chromium %1")
                           .arg(QT_VERSION_STR)
                           .arg("73.0.3683"));
#else
    ui->QtVersion->setText(QT_VERSION_STR);
#endif
    ui->boostVersion->setText(BOOST_VERSION_STR);
    ui->googleGumboVersion->setText(GOOGLE_GUMBO_VERSION_STR);
    ui->libtorrentVersion->setText(LIBTORRENT_VERSION_STR);
    ui->youtubeDLVersion->setText(tr("Reading..."));
    askStreamVersionAsync();

    if (!QSslSocket::supportsSsl()) {

        ui->description->setText(
                    tr("This application can't find SSL or a compatible "
                       "version (SSL %0), the application will fail to "
                       "download with secure sockets (HTTPS, FTPS).")
                    .arg(STR_COMPILER_WORDSIZE));

        const QString undefined(tr("not found"));
        ui->sslLibraryVersion->setText(undefined);
        ui->sslLibraryBuildVersion->setText(undefined);

        const QString stylesheet("QLabel { font-weight:bold; color:red; }");
        ui->description->setStyleSheet(stylesheet);
        ui->sslLibraryVersion->setStyleSheet(stylesheet);
        ui->sslLibraryBuildVersion->setStyleSheet(stylesheet);

    } else {
        ui->description->setText(tr("This application supports SSL."));

        ui->sslLibraryVersion->setText(QSslSocket::sslLibraryVersionString());
        ui->sslLibraryBuildVersion->setText(QSslSocket::sslLibraryBuildVersionString());
    }

    try {
        populateOpenSSL();
    } catch (...) {
        qWarning("CompilerDialog: OpenSSL library fatal error.");
    }
}

CompilerDialog::~CompilerDialog()
{
    delete ui;
}

/******************************************************************************
 ******************************************************************************/
void CompilerDialog::onOkButtonReleased()
{
    QDialog::accept();
}

/******************************************************************************
 ******************************************************************************/
void CompilerDialog::askStreamVersionAsync()
{
    auto w = new StreamVersion();
    connect(this, &QObject::destroyed, w, &StreamVersion::stop);
    connect(w, &StreamVersion::resultReady, ui->youtubeDLVersion, &QLabel::setText);
    connect(w, &StreamVersion::finished, w, &QObject::deleteLater);
    w->start();
}

/******************************************************************************
 ******************************************************************************/
void CompilerDialog::populateOpenSSL()
{
    QString libSsl;
    QString libCrypto;

#if defined(Q_OS_WIN32) && !defined(Q_OS_WIN64) // 32-bit only
    // libSsl = QLatin1String("libssl-1_1");
    // libCrypto = QLatin1String("libcrypto-1_1");
    libSsl = QLatin1String("ssleay32");
    libCrypto = QLatin1String("libeay32");

#elif defined(Q_OS_WIN32) && defined(Q_OS_WIN64) // 32-bit or 64-bit
    libSsl = QLatin1String("libssl-3-x64");
    libCrypto = QLatin1String("libcrypto-3-x64");

#elif defined(Q_OS_UNIX)
    libSsl = QLatin1String("libssl");
    libCrypto = QLatin1String("libcrypto");

#else /* POSIX */
#endif

    ui->libSsl->setText(getLibraryInfo(libSsl));
    ui->libCrypto->setText(getLibraryInfo(libCrypto));
}

QString CompilerDialog::getLibraryInfo(const QString &libraryName)
{
    QLibrary library(libraryName);
    QString libraryVersion = getVersionString(library.fileName());
    library.load();
    if (library.isLoaded()) {
        return QString("%0, version %1").arg(library.fileName(), libraryVersion);
    }
    return QString("not found");
}

QString CompilerDialog::getVersionString(const QString &fName)
{
    QString ret;
#ifdef Q_OS_WIN

    // first of all, GetFileVersionInfoSize
    DWORD dwHandle;
    DWORD dwLen = GetFileVersionInfoSize(fName.toStdWString().c_str(), &dwHandle);
    if (dwLen == 0) {
        return "";
    }
    // GetFileVersionInfo
    LPVOID lpData = new BYTE[dwLen];
    if(!GetFileVersionInfo(fName.toStdWString().c_str(), dwHandle, dwLen, lpData)) {
        delete[] reinterpret_cast<BYTE*>(lpData);
        return "";
    }

    // VerQueryValue
    VS_FIXEDFILEINFO *lpBuffer = nullptr;
    dwLen = sizeof( VS_FIXEDFILEINFO );
    // UINT uLen;
    if (VerQueryValue(
                lpData,
                QString("\\").toStdWString().c_str(),
                reinterpret_cast<LPVOID*>(&lpBuffer),
                reinterpret_cast<unsigned int*>(&dwLen /*&uLen*/)) ) {
        ret = QString::number(( lpBuffer->dwFileVersionMS >> 16 ) & 0xffff ) + "." +
                QString::number( ( lpBuffer->dwFileVersionMS) & 0xffff ) + "." +
                QString::number( ( lpBuffer->dwFileVersionLS >> 16 ) & 0xffff ) + "." +
                QString::number( ( lpBuffer->dwFileVersionLS) & 0xffff );
    } else {
        ret = QString("?.?.??");
    }
    delete[] reinterpret_cast<BYTE*>(lpData);

#else /* POSIX */
    // \todo
    Q_UNUSED(fName)
    ret = QString("?.?.??");
#endif
    return ret;
}
