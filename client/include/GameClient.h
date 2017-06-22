#pragma once

#include <boost/asio.hpp>

#include "NetworkBase.h"

using namespace std;
using boost::asio::ip::udp;

namespace mentics { namespace network {

class GameClient : public NetworkBase {
public:
	GameClient(string remoteHost, unsigned short remotePort) :
		NetworkBase("Client", 0), // port 0 will allow OS to choose port
		clientId(-1),
		serverHost(remoteHost), serverPort(remotePort)
	{}

	void start() override;

private:
	ThatType clientId;
	string serverHost;
	unsigned short serverPort;
	udp::endpoint serverEndpoint;

	void handleReceive(const boost::system::error_code& error, size_t numBytes) override;
	void handleSubscribe(ThatType clientId);
	void handleMessage(string message);

	void sendMessage(string message);

	void sendSubscribe();
};

}}