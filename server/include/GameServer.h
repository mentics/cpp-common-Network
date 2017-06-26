#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "NetworkBase.h"

namespace mentics { namespace network {

using namespace std;
using namespace boost::asio;
using boost::asio::ip::udp;

struct ClientInfo {
	ThatType id;
	udp::endpoint clientEndpoint;

	ClientInfo(ThatType clientId, udp::endpoint& target) : id(clientId), clientEndpoint(target) {}
};

class GameServer : public NetworkBase {
public:
	explicit GameServer(unsigned short localPort)
		: NetworkBase("Server", localPort), clients()
	{}

	void start() override;

private:
	unsigned short port;
	boost::array<ClientInfo*, 1024> clients;

	void handleReceive(const boost::system::error_code& error, size_t numBytes) override;
	ClientInfo* findClientInfo(udp::endpoint& endpoint, boost::array<byte, MAX_MESSAGE_SIZE> in);

	void sendMessageAll(const string message, const ThatType clientId);

	void handleSubscribe(udp::endpoint& endpoint);
	void handleMessage(ClientInfo& clientInfo, string message);
	void handleAction(ClientInfo& clientInfo, string action);
};

}}