#pragma once

#include <functional>
#include <boost/asio.hpp>

#include "NetworkBase.h"

namespace mentics { namespace network {

namespace asio = boost::asio;
using boost::asio::ip::udp;

class NetworkClient : public NetworkBase {
public:
	static udp::endpoint endpointFor(std::string remoteHost, unsigned short remotePort);

	NetworkClient(std::string name, const udp::endpoint& serverEndpoint, NetworkHandler* handler) :
		serverEndpoint(serverEndpoint),
		NetworkBase(name, 0, handler) // port 0 will allow OS to choose port
	{
		BOOST_ASSERT(!this->serverEndpoint.address().is_unspecified());
	}

	NetworkClient(const udp::endpoint& endpoint, NetworkHandler* handler) :
		NetworkClient("Client", endpoint, handler) {}

	inline void submit(RealDurationType period, CountType retries,
		std::string data, MessageCallbackType callback) {
		BOOST_ASSERT(!serverEndpoint.address().is_unspecified());
		NetworkBase::submit(serverEndpoint, period, retries, data, callback);
	}

protected:
	void run() override;

private:
	udp::endpoint serverEndpoint;
};

}}