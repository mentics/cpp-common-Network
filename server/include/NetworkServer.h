#pragma once

#include "NetworkBase.h"

namespace mentics { namespace network {

class NetworkServer : public NetworkBase {
public:
	explicit NetworkServer(unsigned short localPort, NetworkHandler* handler) :
		NetworkBase("Server", localPort, handler) {}

protected:
	void run() override;
};

}}