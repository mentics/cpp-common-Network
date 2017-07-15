#include "stdafx.h"

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "MenticsCommon.h"
#include "NetworkClient.h"

namespace mentics { namespace network {

namespace cmn = mentics::common;
namespace lvl = boost::log::trivial;
namespace asio = boost::asio;
using boost::asio::ip::udp;

void NetworkClient::run() {
	LOG(lvl::info) << "starting...";
	udp::resolver resolver(netio);
	udp::resolver::query query(udp::v4(), serverHost, std::to_string(serverPort));
	serverEndpoint = *resolver.resolve(query);

	listen();

	while (!netio.stopped()) {
		try {
			netio.run();
		}
		catch (const std::exception& e) {
			LOG(lvl::error) << "Network exception: " << cmn::toString(e.what());
		}
		catch (...) {
			LOG(lvl::error) << "Unknown exception in client network thread";
		}
	}
	LOG(lvl::info) << "NetworkClient stopped";
}

}}