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

udp::endpoint NetworkClient::endpointFor(std::string remoteHost, unsigned short remotePort) {
	// NOTE: do we really have to create a new io_service here? or is it not expensive?
	asio::io_service io;
	udp::resolver resolver(io);
	udp::resolver::query query(udp::v4(), remoteHost, std::to_string(remotePort));
	udp::endpoint endpoint = *resolver.resolve(query);
	BOOST_ASSERT(!endpoint.address().is_unspecified());
	BOOST_ASSERT(endpoint.port() != 0);
	return endpoint;
}

void NetworkClient::run() {
	LOG(lvl::info) << "starting...";

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