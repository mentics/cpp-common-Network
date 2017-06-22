#include "stdafx.h"

#include <boost/bind.hpp>

#include "NetworkBase.h"

using namespace std;
using namespace boost::asio;
using boost::asio::ip::udp;

namespace mentics { namespace network {

void NetworkBase::listen()
{
	socket.async_receive_from(
		buffer(currentInput, MAX_MESSAGE_SIZE), currentEndpoint,
		boost::bind(&NetworkBase::handleReceive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	log("Listening on port " + toString(socket.local_endpoint().port()));
}

template <typename ConstBufferSequence>
void NetworkBase::send(udp::endpoint& target, const Command cmd, const ConstBufferSequence& buffers) {
	boost::array<const_buffer, 2> bufs = { buffer((byte*)(&cmd), 1), buffers };
	socket.send_to(bufs, target);
}
template void NetworkBase::send<mutable_buffers_1>(udp::endpoint& target, const Command cmd, const mutable_buffers_1& buffers);
template void NetworkBase::send<const_buffers_1>(udp::endpoint& target, const Command cmd, const const_buffers_1& buffers);

}}