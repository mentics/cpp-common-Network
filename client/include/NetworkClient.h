#pragma once

#include <functional>
#include <boost/asio.hpp>

#include "NetworkBase.h"

namespace mentics { namespace network {

namespace asio = boost::asio;
using boost::asio::ip::udp;

class NetworkClient : public NetworkBase {
public:
	NetworkClient(std::string remoteHost, unsigned short remotePort) :
		NetworkBase("Client", 0), // port 0 will allow OS to choose port
		clientId(-1),
		serverHost(remoteHost), serverPort(remotePort) {}

	void start() override;

	void createGame(std::function<void(const GameInfo&)> callback);
	void joinGame(GameIdType gameId);

private:
	ClientIdType clientId;
	std::string serverHost;
	unsigned short serverPort;
	udp::endpoint serverEndpoint;

	void handleReceive(const boost::system::error_code& error, size_t numBytes) override;
	void handleSubscribe(ClientIdType clientId);
	void handleMessage(std::string message);

	void sendWithId(Command cmd);
	void sendWithId(Command cmd, const asio::const_buffer& buffers);

	void sendMessage(std::string message);

	void sendSubscribe();
};

}}