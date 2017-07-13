#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "NetworkBase.h"

namespace mentics { namespace network {

namespace lvl = boost::log::trivial;
namespace asio = boost::asio;
using boost::asio::ip::udp;

struct ClientInfo {
	ClientIdType id;
	udp::endpoint clientEndpoint;

	ClientInfo() : id(INVALID_CLIENT_ID), clientEndpoint(NULL_ENDPOINT) {}
	ClientInfo(ClientIdType clientId, udp::endpoint& target) : id(clientId), clientEndpoint(target) {}
};

class NetworkServer : public NetworkBase {
public:
	explicit NetworkServer(unsigned short localPort, NetworkHandler* handler) :
		NetworkBase("Server", localPort, handler) {}

	void start() override;

private:
	//unsigned short port;
	//boost::array<ClientInfo, 1024> clients;

	//udp::endpoint endpointFor(NetworkMessage message) override;
	//void handleReceive(const boost::system::error_code& error, size_t numBytes) override;
	//ClientIdType findClientInfo(udp::endpoint& endpoint, boost::array<byte, MAX_MESSAGE_SIZE> in);

	//void sendMessageAll(const std::string message, const ClientIdType clientId);
};

}}