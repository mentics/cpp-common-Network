#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "MenticsCommon.h"
#include "protocol.h"

namespace mentics { namespace network {

namespace cmn = mentics::common;
namespace asio = boost::asio;
using boost::asio::ip::udp;

class NetworkBase : public cmn::CanLog {
public:
	NetworkBase(std::string name, unsigned int localPort) :
		cmn::CanLog(name),
		socket(netio, udp::endpoint(udp::v4(), localPort)) {}
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
	boost::asio::io_service netio;
	udp::socket socket;
	udp::endpoint currentEndpoint;
	boost::array<byte, MAX_MESSAGE_SIZE> currentInput;

	void listen();
	virtual void handleReceive(const boost::system::error_code& error, size_t numBytes) = 0;
	template <typename ConstBufferSequence>
	void send(udp::endpoint& target, const Command cmd, const ConstBufferSequence& buffers);
};

}}