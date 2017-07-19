/*
 *  Copyright (C) 2017 Sami Vänttinen <sami.vanttinen@protonmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtWidgets>
#include <QJsonArray>
#include <QtConcurrent/QtConcurrent>
#include <iostream>
#include "chromelistener.h"

ChromeListener::ChromeListener(QWidget *parent) : QDialog(parent), m_sd(m_io_service, ::dup(STDIN_FILENO)),
    m_socket(m_io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 0)),
    m_resolver(m_io_service), m_remotePort(19700)
{
#ifdef QT_DEBUG
    textEdit = new QTextEdit();
    textEdit->append("Ready.");
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(textEdit, 0, 0);
    setLayout(mainLayout);
#endif
}

ChromeListener::~ChromeListener()
{
    if (m_sd.is_open()) {
        m_sd.cancel();
        m_sd.close();
    }

    if (!m_io_service.stopped()) {
        m_io_service.stop();
    }

    m_fut.waitForFinished();
}

void ChromeListener::run()
{
    m_fut = QtConcurrent::run(this, &ChromeListener::launch);
}

void ChromeListener::readHeader(boost::asio::posix::stream_descriptor& sd)
{
    std::array<char, 4> buf;
    async_read(sd, boost::asio::buffer(buf), boost::asio::transfer_at_least(1), [&](boost::system::error_code ec, size_t br) {
        if (!ec && br >= 1) {
            uint len = 0;
            for (int i = 0; i < 4; i++) {
                uint rc = buf[i];
                len = len | (rc << i*8);
            }
            readBody(sd, len);
        }
    });
}

void ChromeListener::readBody(boost::asio::posix::stream_descriptor& sd, const size_t len)
{
    std::array<char, max_length> buf;
    async_read(sd, boost::asio::buffer(buf), boost::asio::transfer_at_least(1), [&](boost::system::error_code ec, size_t br) {
        if (!ec && br > 0) {
            QByteArray arr(buf.data(), br);
            if (!arr.isEmpty()) {
#ifdef QT_DEBUG
                QJsonParseError err;
                QJsonDocument doc(QJsonDocument::fromJson(arr, &err));
                if (doc.isObject()) {
                    QJsonObject json = doc.object();
                    QString reply(QJsonDocument(json).toJson());
                    textEdit->append("JSON (" + QString::number(reply.length()) + "): " + reply);
                }
                if (err.error != QJsonParseError::NoError) {
                    textEdit->append("Json Error: " + err.errorString());
                }

                textEdit->append("Received NM (" + QString::number(br) + "): " + QString(arr));
#endif
                sendPacket(QString(arr));
            }
            readHeader(sd);
        }
    });
}

void ChromeListener::launch()
{
    // Read the message header
   std::array<char, 4> buf;
    async_read(m_sd, boost::asio::buffer(buf), boost::asio::transfer_at_least(1), [&](boost::system::error_code ec, size_t br) {
        if (!ec && br >= 1) {
            uint len = 0;
            for (int i = 0; i < 4; i++) {
                uint rc = buf[i];
                len = len | (rc << i*8);
            }
            readBody(m_sd, len);
        }
    });

    // Start receiving UDP packets from KeePassXC
    startReceive();
    m_io_service.run();
}

void ChromeListener::sendPacket(const QString packetData)
{
    boost::asio::ip::udp::endpoint endpoint = *m_resolver.resolve({boost::asio::ip::udp::v4(), "127.0.0.1", std::to_string(m_remotePort)});
    m_socket.async_send_to(boost::asio::buffer(packetData.toStdString(), packetData.length()), endpoint,
                boost::bind(&ChromeListener::handle_send, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
}

void ChromeListener::sendReply(const QString reply)
{
    uint len = reply.length();
    std::cout << char(((len>>0) & 0xFF))
                << char(((len>>8) & 0xFF))
                << char(((len>>16) & 0xFF))
                << char(((len>>24) & 0xFF));
    std::cout << reply.toStdString() << std::flush;
}

 void ChromeListener::startReceive()
 {
     m_socket.async_receive_from(boost::asio::buffer(m_buffer, max_length), m_endpoint,
                boost::bind(&ChromeListener::handle_receive, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
 }

 void ChromeListener::handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
 {
    if (!error && bytes_transferred > 0) {
        QByteArray arr(m_buffer, bytes_transferred);
        QString action(arr);

        // TODO: Read the remote port from change-public-keys request
        if (!action.isEmpty()) {
#ifdef QT_DEBUG
            textEdit->append("Received datagram: " + action);
#endif
            sendReply(action);
        }
    }
    startReceive();
 }

 void ChromeListener::handle_send(const boost::system::error_code& /*error*/, std::size_t /*bytes_transferred*/)
 {

 }
