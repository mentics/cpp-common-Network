#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "NetworkBase.h"

namespace mentics { namespace network {

namespace lvl = boost::log::trivial;
namespace asio = boost::asio;
using boost::asio::ip::udp;

class NetworkServer : public NetworkBase {
public:
	explicit NetworkServer(unsigned short localPort, NetworkHandler* handler) :
		NetworkBase("Server", localPort, handler) {}

protected:
	void run() override;
};

}}