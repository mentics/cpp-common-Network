#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <vector>
#include <queue>
#include <boost/log/trivial.hpp>

#include "MenticsCommon.h"
#include "protocol.h"

namespace mentics { namespace network {

namespace lvl = boost::log::trivial;
namespace asio = boost::asio;
namespace ptime = boost::posix_time;
using boost::asio::ip::udp;
namespace cmn = mentics::common;

typedef uint32_t MsgIdType;
typedef ptime::ptime RealTimeType;
typedef ptime::time_duration RealDurationType;
typedef uint16_t CountType;
typedef std::function<void(udp::endpoint, std::string)> MessageCallbackType;

struct NetworkMessage {
	RealTimeType nextRunTime;
	RealDurationType period;
	CountType retries;
	MsgIdType msgId;
	std::string data;
	MessageCallbackType callback;
	// TODO: clients don't need endpoint so we can reduce this size
	udp::endpoint endpoint;

	//NetworkMessage() {}
	//NetworkMessage(udp::endpoint endpoint, RealDurationType period, CountType retries,
	//		MsgIdType msgId, std::string data, MessageCallbackType callback) :
	//				period(period), retries(retries), msgId(msgId), data(data), endpoint(endpoint) {}

	static bool compare(NetworkMessage& ev1, NetworkMessage& ev2) {
		return ev1.nextRunTime > ev2.nextRunTime;
	}
};

//struct NetworkMessageServer : NetworkMessage {
//	udp::endpoint target;
//
//	NetworkMessageServer() : NetworkMessage() {}
//	NetworkMessageServer(RealDurationType period, CountType retries,
//		MsgIdType msgId, std::string message, MessageCallbackType callback, udp::endpoint target) :
//		NetworkMessage(period, retries, msgId, message, callback),
//		target(target) {}
//};

class NetworkHandler {
public:
	virtual void handle(udp::endpoint& endpoint, const std::string& data) = 0;
	virtual void handleError(udp::endpoint& endpoint, const boost::system::error_code& error) = 0;
};

class NetworkBase : public cmn::CanLog {
public:
	NetworkBase(std::string name, unsigned int localPort, NetworkHandler* handler) :
		cmn::CanLog(name),
		socket(netio, udp::endpoint(udp::v4(), localPort)),
		thread(&NetworkBase::start, this),
		retryTimer(netio),
		handler(handler) {}
	~NetworkBase() {
		if (!netio.stopped()) {
			stop();
		}
	}

	virtual void start() = 0;

	inline void submit(udp::endpoint endpoint, RealDurationType period, CountType retries,
			std::string data, MessageCallbackType callback) {
		// TODO: use ringbuffer instead of pointers?
		NetworkMessage msg = { ptime::ptime(), period, retries, nextMsgId++, data, callback, endpoint };
		incoming.push(msg);
		// Wake up the timer to run right away
		retryTimer.cancel();
	}

	inline void sendAndRetry() {
		// TODO: we could combine messages into single send to optimize
		// Have a batch at top of method that we add to, then at the end if it's not empty, send it.
		ptime::ptime now = ptime::microsec_clock::universal_time();
		LOG(lvl::trace) << "sendAndRetry at " << now;
		while (!incoming.empty()) {
			NetworkMessage message;
			incoming.pop(message);
			send(message.endpoint, message);
			message.nextRunTime = now + message.period;
			inFlight.push(message);
		}

		// TODO: update now?
		ptime::ptime nextRetryTime;
		while (!inFlight.empty()) {
			NetworkMessage top = inFlight.top();
			if (top.nextRunTime <= now) {
				inFlight.pop();
				if (top.retries <= 0) {
					// TODO: call callback with no data indicating no ack
				} else {
					send(top.endpoint, top);
					top.nextRunTime = now + top.period;
					top.retries--;
					inFlight.push(top);
				}			} else {
				break;
			}
		}
		retryTimer.expires_at(nextRetryTime);
		retryTimer.async_wait([this](const boost::system::error_code&) {
			this->sendAndRetry();
		});
	}

	void stop() {
		netio.stop();
		thread.join();
	}

protected:
	std::thread thread;
	asio::deadline_timer retryTimer;
	boost::asio::io_service netio;
	udp::socket socket;
	udp::endpoint currentEndpoint;
	boost::array<byte, MAX_MESSAGE_SIZE> currentInput;
	NetworkHandler* handler;
	//std::function<void(udp::endpoint&, const std::string&)> handler;
	//std::function<void(udp::endpoint&, const boost::system::error_code& error)> errorHandler;

	MsgIdType nextMsgId = 1;
	boost::lockfree::spsc_queue<NetworkMessage, boost::lockfree::capacity<1024>> incoming;
	std::priority_queue<NetworkMessage, std::vector<NetworkMessage>, decltype(&NetworkMessage::compare)> inFlight;

	//virtual udp::endpoint endpointFor(NetworkMessage message) = 0;
	void listen();
	void send(udp::endpoint& target, NetworkMessage& item);
	void handleReceive(const boost::system::error_code& error, size_t numBytes);
};

}}