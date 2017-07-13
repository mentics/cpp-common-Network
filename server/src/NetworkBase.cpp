#include "stdafx.h"

#include <boost/bind.hpp>

#include "MenticsCommon.h"
#include "NetworkBase.h"

namespace mentics { namespace network {

namespace cmn = mentics::common;
namespace lvl = boost::log::trivial;
namespace asio = boost::asio;
using boost::asio::ip::udp;

void NetworkBase::listen() {
	socket.async_receive_from(
		asio::buffer(currentInput, MAX_MESSAGE_SIZE), currentEndpoint,
		boost::bind(&NetworkBase::handleReceive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	LOG(lvl::info) << "Listening on port " << cmn::toString(socket.local_endpoint().port());
}

void NetworkBase::handleReceive(const boost::system::error_code& error, size_t numBytes) {
	LOG(lvl::trace) << "handleReceive...";
	if (!error) {
		handler->handle(currentEndpoint, std::string(reinterpret_cast<char const*>(currentInput.data() + 1), numBytes - 1));
	} else {
		handler->handleError(currentEndpoint, error);
	}
	// TODO: if we get that "forcibly closed" error, we can remove that client--see example networklib code on github, maybe it does this

	listen();
}

void NetworkBase::send(udp::endpoint& endpoint, NetworkMessage& item) {
	MsgIdType netId = htonl(item.msgId);
	boost::array<asio::const_buffer, 2> bufs = {
		asio::buffer(&netId, sizeof(MsgIdType)), asio::buffer(item.data)
	};
	socket.send_to(bufs, endpoint);
}

}}