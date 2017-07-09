#include "stdafx.h"

#include <boost/bind.hpp>

#include "MenticsCommon.h"
#include "NetworkBase.h"

namespace mentics { namespace network {

namespace cmn = mentics::common;
namespace lvl = boost::log::trivial;
namespace asio = boost::asio;
using boost::asio::ip::udp;

void NetworkBase::listen()
{
	socket.async_receive_from(
		asio::buffer(currentInput, MAX_MESSAGE_SIZE), currentEndpoint,
		boost::bind(&NetworkBase::handleReceive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	LOG(lvl::info) << "Listening on port " << cmn::toString(socket.local_endpoint().port());
}

template <typename ConstBufferSequence>
void NetworkBase::send(udp::endpoint& target, const Command cmd, const ConstBufferSequence& buffers) {
	boost::array<asio::const_buffer, 2> bufs = { asio::buffer((byte*)(&cmd), 1), buffers };
	socket.send_to(bufs, target);
}
template void NetworkBase::send<asio::mutable_buffers_1>(udp::endpoint& target, const Command cmd, const asio::mutable_buffers_1& buffers);
template void NetworkBase::send<asio::const_buffers_1>(udp::endpoint& target, const Command cmd, const asio::const_buffers_1& buffers);

}}