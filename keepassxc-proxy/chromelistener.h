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

#ifndef CHROMELISTENER_H
#define CHROMELISTENER_H

#include <QDialog>
#include <QFuture>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

enum { max_length = 4069 };

class ChromeListener : public QDialog
{
    Q_OBJECT

public:
    ChromeListener(QWidget *parent = 0);
    ~ChromeListener();
    void run();

private:
    void launch();
    void readHeader(boost::asio::posix::stream_descriptor& sd);
    void readBody(boost::asio::posix::stream_descriptor& sd, const size_t len);
    void sendPacket(const QString message);
    void sendReply(const QString reply);
    void startReceive();
    void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
    void handle_send(const boost::system::error_code& /*error*/, std::size_t /*bytes_transferred*/);

private:
    QFuture<void>                           m_fut;
    boost::asio::io_service                 m_io_service;
    boost::asio::posix::stream_descriptor   m_sd;
    boost::asio::ip::udp::socket            m_udps;
    boost::asio::ip::udp::endpoint          m_endpoint;
    boost::asio::ip::udp::resolver          m_resolver;
    char                                    m_buffer[max_length];
};

#endif
