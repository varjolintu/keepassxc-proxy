#include <boost/asio.hpp>
#include <iostream>
#ifndef _WIN32
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/bind.hpp>
#endif
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

using namespace boost::asio;
using boost::asio::ip::udp;
using boost::system::error_code;

class NativeMessagingHost
{
public:
	NativeMessagingHost(boost::asio::io_service& io_service);
	void readMessages();
	void readUDP();
#ifndef _WIN32
	void readHeader();
	void readBody(const size_t len);
	void handleHeader(const boost::system::error_code ec, const size_t br);
#endif

private:
	void sendReply(const std::string& reply);
	void sendUDP(const std::string& reply, const std::size_t length);

private:
	enum { max_length = 4*1024 };
	std::atomic_bool						m_interrupted;
	udp::socket								m_socket;
	udp::endpoint							m_senderEndpoint;
	udp::endpoint 							m_endpoint;
	std::array<char, 4>						m_headerBuf;
	boost::asio::posix::stream_descriptor	m_sd;
};

NativeMessagingHost::NativeMessagingHost(boost::asio::io_service& io_service) :
#ifndef Q_OS_WIN
    m_sd(io_service, ::dup(STDIN_FILENO)),
#endif
	m_interrupted(false),
	m_socket(io_service, udp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 0)),
	m_endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 19700)
{
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif
}

void NativeMessagingHost::readUDP()
{
	std::array<char, max_length> receiveBuffer;
	m_socket.async_receive_from(boost::asio::buffer(receiveBuffer, max_length), m_senderEndpoint, [&](error_code ec, std::size_t br) {
		if (!ec && br > 0) {
			std::string reply(receiveBuffer.data(), br);
			sendReply(reply);
		}
		readUDP();
	});
}

void NativeMessagingHost::sendUDP(const std::string& reply, const std::size_t length)
{
	m_socket.async_send_to(boost::asio::buffer(reply, length), m_endpoint, [this](error_code, std::size_t) {
		readUDP();
	});
}

// Windows only
void NativeMessagingHost::readMessages()
{
    unsigned int length = 0;
    unsigned int c = 0;
    std::string message;
    while (!m_interrupted) {
    	message.clear();
        length = 0;
        std::cin.read(reinterpret_cast<char*>(&length), 4);
        if (length > 0) {
        	for (int i = 0; i < length; i++) {
        		c = getchar();
        		message += c;
        	}
        	sendUDP(message, length);
        }
    }
}

#ifndef _WIN32
void NativeMessagingHost::readHeader()
{
	async_read(m_sd, buffer(m_headerBuf), boost::bind(&NativeMessagingHost::handleHeader, this,
        boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void NativeMessagingHost::readBody(const size_t len)
{
    std::array<char, max_length> buf;
    async_read(m_sd, buffer(buf, len), transfer_at_least(1), [this, &buf](error_code ec, size_t br) {
        if (!ec && br > 0) {
        	std::string body(buf.data(), br);
            sendUDP(body, body.length());
            readHeader();
        }
    });
}

void NativeMessagingHost::handleHeader(const boost::system::error_code ec, const size_t br)
{
    if (!ec && br >= 1) {
        uint len = 0;
        for (int i = 0; i < 4; i++) {
            uint rc = m_headerBuf.at(i);
            len = len | (rc << i*8);
        }
        readBody(len);
    }
}
#endif

void NativeMessagingHost::sendReply(const std::string& reply)
{
    unsigned int len = reply.length();
    std::cout << char(((len>>0) & 0xFF)) << char(((len>>8) & 0xFF)) << char(((len>>16) & 0xFF)) << char(((len>>24) & 0xFF));
    std::cout << reply << std::flush;
}

int main()
{
    io_service svc;
    NativeMessagingHost host(svc);
#ifdef _WIN32
    host.readMessages();
#else
	host.readHeader();
#endif
    host.readUDP();
    svc.run();
}
