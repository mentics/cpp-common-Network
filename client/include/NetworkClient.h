#pragma once

#include <functional>
#include <boost/asio.hpp>

#include "NetworkBase.h"

namespace mentics { namespace network {

using namespace std;
using boost::asio::ip::udp;

class NetworkClient : public NetworkBase {
public:
	NetworkClient(string remoteHost, unsigned short remotePort) :
		NetworkBase("Client", 0), // port 0 will allow OS to choose port
		clientId(-1),
		serverHost(remoteHost), serverPort(remotePort) {}

	void start() override;

	void createGame(std::function<void(const GameInfo&)> callback);
	void joinGame(GameIdType gameId);

private:
	ClientIdType clientId;
	string serverHost;
	unsigned short serverPort;
	udp::endpoint serverEndpoint;

	void handleReceive(const boost::system::error_code& error, size_t numBytes) override;
	void handleSubscribe(ClientIdType clientId);
	void handleMessage(string message);

	void sendWithId(Command cmd);
	void sendWithId(Command cmd, const const_buffer& buffers);

	void sendMessage(string message);

	void sendSubscribe();
};

}}