#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "protocol.h"

using namespace std;
using namespace boost::asio;
using boost::asio::ip::udp;

namespace mentics { namespace network {

class NetworkBase {
public:
	NetworkBase(string name, unsigned int localPort) :
		name(name), socket(netio, udp::endpoint(udp::v4(), localPort)) {}
	~NetworkBase() {
		if (!netio.stopped()) {
			stop();
		}
	}
	virtual void start() = 0;
	void stop() {
		netio.stop();
	}

protected:
	string name;
	boost::asio::io_service netio;
	udp::socket socket;
	udp::endpoint currentEndpoint;
	boost::array<byte, MAX_MESSAGE_SIZE> currentInput;

	void log(string message) {
		logger(name + ": " +message);
	}
	void listen();
	virtual void handleReceive(const boost::system::error_code& error, size_t numBytes) = 0;
	template <typename ConstBufferSequence>
	void send(udp::endpoint& target, const Command cmd, const ConstBufferSequence& buffers);
};

}}