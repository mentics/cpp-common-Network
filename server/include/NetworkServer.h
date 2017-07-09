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
	explicit NetworkServer(unsigned short localPort) :
		NetworkBase("Server", localPort),
		rerun(netio),
		clients()
	{}

	void start() override;

	inline void testTimer() {
		LOG(lvl::trace) << "testTimer";
		rerun.expires_from_now(boost::posix_time::milliseconds(100));
		//auto handler = [this](const std::function<void(const boost::system::error_code&)>& cb) {
		//	this->testTimer();
		//};
		//rerun.async_wait(handler);

		//rerun.async_wait(std::bind(&NetworkServer::testTimer, this, std::placeholders::_1));

		rerun.async_wait([this](const boost::system::error_code&) {
			//rerun.expires_from_now(boost::posix_time::milliseconds(1000));
			this->testTimer();
		});
	}

private:
	unsigned short port;
	asio::deadline_timer rerun;
	boost::array<ClientInfo, 1024> clients;

	void handleReceive(const boost::system::error_code& error, size_t numBytes) override;
	ClientIdType findClientInfo(udp::endpoint& endpoint, boost::array<byte, MAX_MESSAGE_SIZE> in);

	void sendMessageAll(const std::string message, const ClientIdType clientId);

	void handleSubscribe(udp::endpoint& endpoint);
	void handleMessage(ClientIdType clientId, std::string message);
	void handleAction(ClientIdType clientId, std::string action);
};

}}