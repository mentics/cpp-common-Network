#pragma once

#include <functional>
#include <boost/asio.hpp>

#include "NetworkBase.h"

namespace mentics { namespace network {

namespace asio = boost::asio;
using boost::asio::ip::udp;

class NetworkClient : public NetworkBase {
public:
	NetworkClient(std::string name, std::string remoteHost, unsigned short remotePort, NetworkHandler* handler) :
		NetworkBase(name, 0, handler), // port 0 will allow OS to choose port
		clientId(-1),
		serverHost(remoteHost), serverPort(remotePort) {}

	NetworkClient(std::string remoteHost, unsigned short remotePort, NetworkHandler* handler) :
		NetworkBase("Client", 0, handler), // port 0 will allow OS to choose port
		clientId(-1),
		serverHost(remoteHost), serverPort(remotePort) {}

	inline void submit(RealDurationType period, CountType retries,
		std::string data, MessageCallbackType callback) {
		NetworkBase::submit(serverEndpoint, period, retries, data, callback);
	}

protected:
	void run() override;

private:
	ClientIdType clientId;
	std::string serverHost;
	unsigned short serverPort;
	udp::endpoint serverEndpoint;
};

}}