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

// NOTE: may run in external thread
void NetworkBase::submit(const udp::endpoint& endpoint, RealDurationType period, CountType retries,
		const std::string& data, const MessageCallbackType& callback) {
	NetworkMessage msg = { ptime::ptime(), period, retries,
		Control::NewMsg, nextMsgId++, data, callback, endpoint };
	incoming.push(msg);
	// Wake up the timer to run right away
	retryTimer.expires_from_now(ptime::seconds(0));
	//retryTimer.cancel();
}

// NOTE: may run in external thread
void NetworkBase::submitReply(const udp::endpoint& endpoint, RealDurationType period, CountType retries,
	const std::string& data, const MessageCallbackType& callback, MsgIdType msgId) {
	NetworkMessage msg = { ptime::ptime(), period, retries,
		Control::Reply, msgId, data, callback, endpoint };
	incoming.push(msg);
	// Wake up the timer to run right away
	retryTimer.expires_from_now(ptime::seconds(0));
}

void NetworkBase::submitAck(const udp::endpoint& endpoint, const MsgIdType msgId) {
	NetworkMessage msg = { ptime::ptime(), ptime::millisec(200), 5,
		Control::Ack, msgId, cmn::EMPTY_STRING, nullptr, endpoint };
	incoming.push(msg);
	// NOTE: we could send it right away and add and update expires at
	retryTimer.expires_from_now(ptime::seconds(0));
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
	try {
	if (!error) {
		byte* raw = currentInput.data();
		byte control = *raw;
		MsgIdType msgId = ntohl(*(uint32_t*)(raw + 1));
		byte headerLen = 1 + sizeof(MsgIdType);
		std::string data(reinterpret_cast<char const*>(currentInput.data() + headerLen), numBytes - headerLen);

		if (control == Control::Ack || control == Control::Reply) {
			handleAck(currentEndpoint, msgId, data);
		}
		if (control != Control::Ack) {
			if (handler->handle(currentEndpoint, msgId, data)) {
				submitAck(currentEndpoint, msgId);
			}
		}
	} else {
		handler->handleError(currentEndpoint, error);
	}
	// TODO: if we get that "forcibly closed" error, we can remove that client--see example networklib code on github, maybe it does this
	} catch (const std::exception& ex) {
		LOG(lvl::error) << "Exception during handleReceive handling: " << ex.what();
	}
	
	listen();
}

void NetworkBase::handleAck(const udp::endpoint& endpoint, const MsgIdType msgId, const std::string& data) {
	LOG(lvl::trace) << "received ack or reply and so removing " << msgId;// << " finding in " << inFlight.top().msgId;
	auto s = inFlight.size();
	
	//MessageCallbackType callback = nullptr;
	inFlight.remove([msgId](const NetworkMessage& msg) {
		//inFlight.remove([msgId, &callback](const NetworkMessage& msg) {
		if (msgId == msg.msgId) {
			//callback = msg.callback;
			return true;
		}
		return false;
	});
	//if (callback != nullptr) {
	//	callback(endpoint, data);
	//}

	auto s2 = inFlight.size();
	if (s == s2) {
		LOG(lvl::error) << "could not remove message from inFlight";
	}
}

void NetworkBase::send(const udp::endpoint& endpoint, const NetworkMessage& item) {
	LOG(lvl::trace) << "sending to " << endpoint.address().to_string() << " msgId=" << item.msgId;
	BOOST_ASSERT(!endpoint.address().is_unspecified());
	MsgIdType netId = htonl(item.msgId);
	boost::array<asio::const_buffer, 3> bufs = {
		asio::buffer(&item.control, 1), asio::buffer(&netId, sizeof(MsgIdType)), asio::buffer(item.data)
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