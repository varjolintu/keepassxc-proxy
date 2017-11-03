#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/bind.hpp>
#include <sys/types.h>
#include <unistd.h>

enum { max_length = 4*1024 };

using namespace boost::asio;
using boost::asio::ip::udp;
using boost::system::error_code;
using boost::asio::local::stream_protocol;

class NativeMessagingHost
{
public:
	NativeMessagingHost(boost::asio::io_service& io_service, stream_protocol::endpoint endpoint) :
		m_socket(io_service),
		m_endpoint(endpoint),
		m_serverEndpoint("/tmp/kpxc_server"),
		m_sd(io_service, ::dup(STDIN_FILENO))
	{
		openSocket();
	}

	void readHeader()
	{
	    async_read(m_sd, buffer(m_headerBuf), boost::bind(&NativeMessagingHost::handleHeader, this,
	        boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	void handleHeader(const boost::system::error_code ec, const size_t br)
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

	void readBody(const size_t len)
	{
		std::array<char, max_length> buf;
	    async_read(m_sd, buffer(buf, len), transfer_at_least(1), [this, &buf](error_code ec, size_t br) {
	        if (!ec && br > 0) {
	        	std::string body(buf.data(), br);
				sendUnixMessage(body);
	            readHeader();
	        }
	    });
	}

	void openSocket()
	{
		try {
			m_socket.open();
			m_socket.bind(m_endpoint);

			boost::system::error_code ec;
			m_socket.connect(m_serverEndpoint, ec);
			if (ec) {
				//std::cout << "Cannot connect: " << ec.message() << std::endl;
			}
		} catch (std::exception& e) {

		}
	}

	void readUnixMessages()
	{
		m_socket.async_receive(boost::asio::buffer(m_buf), boost::bind(&NativeMessagingHost::handleRead, this,
                    boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	void handleRead(const error_code& ec, size_t br)
	{
		if (!ec && br > 0) {
			std::string reply(m_buf.begin(), br);
			sendReply(reply);
		}
		else { std::cout << "Error" << std::endl; }
	}

	void sendUnixMessage(const std::string& mes)
	{
		try {
			m_socket.send(boost::asio::buffer(mes, mes.length()));
			readUnixMessages();
		} catch (std::exception& e) {
			//std::cout << "Exception: " << e.what() << std::endl;
		}
	}

private:
	void sendReply(const std::string& reply)
	{
		unsigned int len = reply.length();
	    std::cout << char(((len>>0) & 0xFF)) << char(((len>>8) & 0xFF)) << char(((len>>16) & 0xFF)) << char(((len>>24) & 0xFF));
	    std::cout << reply << std::flush;
	}

private:
	boost::asio::io_service			m_io_service;
	stream_protocol::endpoint		m_endpoint;
	stream_protocol::endpoint		m_serverEndpoint;
	stream_protocol::socket			m_socket;
	std::array<char, 4>				m_headerBuf;
	std::array<char, max_length>	m_buf;
	std::array<char, max_length>	m_bodyBuf;
	boost::asio::posix::stream_descriptor	m_sd;
};


int main()
{
    io_service svc;

	// Generate client socket from pid
	pid_t pid = getpid();
	std::string client = "/tmp/kpxc_client." + std::to_string(pid);
    NativeMessagingHost host(svc, client);
    host.readHeader();
    svc.run();
}
