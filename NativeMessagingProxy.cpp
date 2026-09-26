/*
 *  Copyright (C) 2026 KeePassXC Team <team@keepassxc.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "NativeMessagingProxy.h"

#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>

#include <iostream>

#ifdef Q_OS_WIN
#include <fcntl.h>
#include <winsock2.h>

#include <windows.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#endif
#if defined(Q_OS_MACOS)
#include <unistd.h>
#endif

namespace BrowserShared
{
    constexpr int SOCKET_BUFFER_SIZE = 1024 * 1024;

    // Get user's temporary directory. Bypasses $TMPDIR on macOS.
    QString getUserTemporaryDirectory()
    {
#if defined(Q_OS_MACOS)
        // In macOS QStandardPaths::TempLocation can be overridden with $TMPDIR. Use the location provided by the OS.
        // Otherwise the socket will be created to incorrect path, and connection with the browser extension breaks.
        const auto len = confstr(_CS_DARWIN_USER_TEMP_DIR, nullptr, 0);
        if (len > 0 && len <= 1024) {
            char rawPath[1024];
            confstr(_CS_DARWIN_USER_TEMP_DIR, rawPath, len);
            auto temporaryDirectory = QString::fromUtf8(rawPath);
            if (temporaryDirectory.endsWith("/")) {
                temporaryDirectory.chop(1);
            }
            return temporaryDirectory;
        }
#endif
        return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    }

    QString localServerPath()
    {
        const auto serverName = QStringLiteral("/org.keepassxc.KeePassXC.BrowserServer");
#if defined(KEEPASSXC_DIST_SNAP)
        return QProcessEnvironment::systemEnvironment().value("SNAP_USER_COMMON") + serverName;
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
        // This returns XDG_RUNTIME_DIR or else a temporary subdirectory.
        QString path = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);

        // Put the socket in a dedicated directory.
        // This directory will be easily mountable by sandbox containers.
        QString subPath = path + "/app/org.keepassxc.KeePassXC";
        QDir().mkpath(subPath);

        QString socketPath = subPath + serverName;
#ifndef KEEPASSXC_DIST_FLATPAK
        // Create a symlink at the legacy location for backwards compatibility.
        const auto origSocketPath = path + serverName;
        QFile::remove(origSocketPath);
        QFile::link(socketPath, origSocketPath);
#endif

        return socketPath;
#elif defined(Q_OS_WIN)
        // Windows uses named pipes
        return serverName + "_" + qgetenv("USERNAME");
#else // Q_OS_MACOS and others
        return getUserTemporaryDirectory() + serverName;
#endif
    }
} // namespace BrowserShared


NativeMessagingProxy::NativeMessagingProxy()
    : QObject()
{
    connect(this,
            &NativeMessagingProxy::stdinMessage,
            this,
            &NativeMessagingProxy::transferStdinMessage,
            Qt::QueuedConnection);

    setupStandardInput();
    setupLocalSocket();
}

void NativeMessagingProxy::setupStandardInput()
{
#ifdef Q_OS_WIN
#ifdef Q_CC_MSVC
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#else
    setmode(fileno(stdin), _O_BINARY);
    setmode(fileno(stdout), _O_BINARY);
#endif
#endif

    auto res = QtConcurrent::run([this] {
        while (std::cin.good()) {
            if (std::cin.peek() != EOF) {
                uint length = 0;
                for (uint i = 0; i < sizeof(uint); ++i) {
                    length |= getchar() << (i * 8);
                }

                QString msg;
                msg.reserve(length);
                for (uint i = 0; i < length; ++i) {
                    msg.append(QChar(getchar()));
                }

                if (msg.length() > 0) {
                    emit stdinMessage(msg);
                }
            }
            QThread::msleep(100);
        }
        QCoreApplication::quit();
    });
}

void NativeMessagingProxy::transferStdinMessage(const QString& msg)
{
    if (m_localSocket && m_localSocket->state() == QLocalSocket::ConnectedState) {
        m_localSocket->write(msg.toUtf8(), msg.length());
        m_localSocket->flush();
    }
}

void NativeMessagingProxy::setupLocalSocket()
{
    m_localSocket.reset(new QLocalSocket());
    m_localSocket->connectToServer(BrowserShared::localServerPath());
    m_localSocket->setReadBufferSize(BrowserShared::SOCKET_BUFFER_SIZE);
    int socketDesc = m_localSocket->socketDescriptor();
    if (socketDesc) {
        int max = BrowserShared::SOCKET_BUFFER_SIZE;
        setsockopt(socketDesc, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&max), sizeof(max));
    }

    connect(m_localSocket.data(), SIGNAL(readyRead()), this, SLOT(transferSocketMessage()));
    connect(m_localSocket.data(), SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
}

void NativeMessagingProxy::transferSocketMessage()
{
    auto msg = m_localSocket->readAll();
    if (!msg.isEmpty()) {
        // Explicitly write the message length as 1 byte chunks
        uint len = msg.size();
        std::cout.write(reinterpret_cast<char*>(&len), sizeof(len));

        // Write the message and flush the stream
        std::cout << msg.toStdString() << std::flush;
    }
}

void NativeMessagingProxy::socketDisconnected()
{
    // Shutdown the proxy when disconnected from the application
    QCoreApplication::quit();
}