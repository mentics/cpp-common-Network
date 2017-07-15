#include "stdafx.h"

#include <boost/bind.hpp>

#include "MenticsCommon.h"
#include "NetworkBase.h"

namespace mentics { namespace network {

namespace cmn = mentics::common;
namespace lvl = boost::log::trivial;
namespace asio = boost::asio;
using boost::asio::ip::udp;

void NetworkBase::start() {
	thread = std::thread(&NetworkBase::runOnce, this);
}

void NetworkBase::runOnce() {
	sendAndRetry();
	run();
}

void NetworkBase::listen() {
	if (netio.stopped()) {
		return;
	}
	socket.async_receive_from(
		asio::buffer(currentInput, MAX_MESSAGE_SIZE), currentEndpoint,
		boost::bind(&NetworkBase::handleReceive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	LOG(lvl::info) << "Listening on port " << cmn::toString(socket.local_endpoint().port());
}

// NOTE: runs in external thread
void NetworkBase::submit(udp::endpoint endpoint, RealDurationType period, CountType retries,
	std::string data, MessageCallbackType callback) {
	NetworkMessage msg = { ptime::ptime(), period, retries, nextMsgId++, data, callback, endpoint };
	incoming.push(msg);
	// Wake up the timer to run right away
	retryTimer.cancel();
}

void NetworkBase::sendAndRetry() {
	if (netio.stopped()) {
		return;
	}
	// TODO: we could combine messages into single send to optimize
	// Have a batch at top of method that we add to, then at the end if it's not empty, send it.
	ptime::ptime now = ptime::microsec_clock::universal_time();
	LOG(lvl::trace) << "sendAndRetry at " << now;
	ptime::ptime nextRetryTime = now + ptime::minutes(5);
	while (!incoming.empty()) {
		NetworkMessage message;
		if (incoming.pop(message)) {
			send(message.endpoint, message);
			message.nextRunTime = now + message.period;
			if (message.nextRunTime < nextRetryTime) {
				nextRetryTime = message.nextRunTime;
			}
			inFlight.push(message);
		}
	}

	// NOTE: do we need to update now?
	while (!inFlight.empty()) {
		NetworkMessage top = inFlight.top();
		if (top.nextRunTime <= now) {
			inFlight.pop();
			if (top.retries <= 0) {
				// TODO: call callback with no data indicating no ack
				LOG(lvl::warning) << " retry count exceeded, dropping msgId " << top.msgId;
			}
			else {
				send(top.endpoint, top);
				top.nextRunTime = now + top.period;
				if (top.nextRunTime < nextRetryTime) {
					nextRetryTime = top.nextRunTime;
				}
				top.retries--;
				inFlight.push(top);
			}		}
		else {
			break;
		}
	}
	LOG(lvl::trace) << "sleeping until " << nextRetryTime;
	retryTimer.expires_at(nextRetryTime);
	retryTimer.async_wait([this](const boost::system::error_code&) {
		this->sendAndRetry();
	});
}

void NetworkBase::handleReceive(const boost::system::error_code& error, const size_t numBytes) {
	LOG(lvl::trace) << "handleReceive...";
	if (!error) {
		byte* raw = currentInput.data();
		byte control = *raw;
		MsgIdType msgId = ntohl(*(uint32_t*)(raw + 1));
		if (control == Control::Ack) {
			handleAck(currentEndpoint, msgId);
		} else {
			byte headerLen = 1 + sizeof(MsgIdType);
			std::string data(reinterpret_cast<char const*>(currentInput.data() + headerLen), numBytes - headerLen);
			handler->handle(currentEndpoint, data);
			sendAck(currentEndpoint, msgId);
		}
	} else {
		handler->handleError(currentEndpoint, error);
	}
	// TODO: if we get that "forcibly closed" error, we can remove that client--see example networklib code on github, maybe it does this

	listen();
}

void NetworkBase::handleAck(const udp::endpoint& endpoint, const MsgIdType msgId) {
	LOG(lvl::trace) << "received ack and so removing " << msgId << " finding in " << inFlight.top().msgId;
	auto s = inFlight.size();
	inFlight.remove([msgId](const NetworkMessage& msg) {
		return msgId == msg.msgId;
	});
	auto s2 = inFlight.size();
	if (s == s2) {
		LOG(lvl::error) << "could not remove message from inFlight";
	}
}

void NetworkBase::send(const udp::endpoint& endpoint, const NetworkMessage& item) {
	LOG(lvl::trace) << "sending to " << endpoint.address().to_string() << " msgId=" << item.msgId;
	byte control = Control::AppLevel;
	MsgIdType netId = htonl(item.msgId);
	boost::array<asio::const_buffer, 3> bufs = {
		asio::buffer(&control, 1), asio::buffer(&netId, sizeof(MsgIdType)), asio::buffer(item.data)
	};
	socket.send_to(bufs, endpoint);
}

void NetworkBase::sendAck(const udp::endpoint& endpoint, const MsgIdType msgId) {
	LOG(lvl::trace) << "sending Ack to " << endpoint.address().to_string() << " for msgId=" << msgId;
	byte control = Control::Ack;
	MsgIdType netId = htonl(msgId);
	boost::array<asio::const_buffer, 2> bufs = {
		asio::buffer(&control, 1), asio::buffer(&netId, sizeof(MsgIdType))
	};
	socket.send_to(bufs, endpoint);
}

void NetworkBase::stop() {
	if (!netio.stopped()) {
		retryTimer.cancel();
		try {
			netio.stop();
		}
		catch (const boost::system::error_code& ec) {
			LOG(lvl::error) << "error_code " << ec.message();
		}
		catch (const boost::system::system_error& ex) {
			LOG(lvl::error) << "system_error " << ex.code() << ", " << ex.what();
		}
	}
	if (thread.joinable()) {
		thread.join();
	}
}

}}