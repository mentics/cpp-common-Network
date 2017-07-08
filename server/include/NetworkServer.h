#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "NetworkBase.h"

namespace mentics { namespace network {

using namespace std;
using namespace boost::asio;
using boost::asio::ip::udp;

struct ClientInfo {
	ClientIdType id;
	udp::endpoint clientEndpoint;

	ClientInfo() : id(INVALID_CLIENT_ID), clientEndpoint(NULL_ENDPOINT) {}
	ClientInfo(ClientIdType clientId, udp::endpoint& target) : id(clientId), clientEndpoint(target) {}
};

class NetworkServer : public NetworkBase {
public:
	explicit NetworkServer(unsigned short localPort)
		: NetworkBase("Server", localPort), clients()
	{}

	void start() override;

private:
	unsigned short port;
	boost::array<ClientInfo, 1024> clients;

	void handleReceive(const boost::system::error_code& error, size_t numBytes) override;
	ClientIdType findClientInfo(udp::endpoint& endpoint, boost::array<byte, MAX_MESSAGE_SIZE> in);

	void sendMessageAll(const string message, const ClientIdType clientId);

	void handleSubscribe(udp::endpoint& endpoint);
	void handleMessage(ClientIdType clientId, string message);
	void handleAction(ClientIdType clientId, string action);
};

}}