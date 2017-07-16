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

#include "MenticsCommon.h"

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
typedef std::function<void(const udp::endpoint&, const std::string&)> MessageCallbackType;

const int MAX_MESSAGE_SIZE = 1023;
const boost::asio::ip::udp::endpoint NULL_ENDPOINT;

enum Control : byte { NewMsg = 1, Ack = 2, Reply = 3 };

struct NetworkMessage {
	RealTimeType nextRunTime;
	RealDurationType period;
	CountType retries;

	Control control;
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
	// Returns true if network layer should send an Ack back.
	// This is typically because no other response is needed so just send Ack.
	virtual bool handle(udp::endpoint& endpoint, MsgIdType msgId, const std::string& data) = 0;
	virtual void handleError(udp::endpoint& endpoint, const boost::system::error_code& error) = 0;
};


// From: https://stackoverflow.com/questions/19467485/how-to-remove-element-not-at-top-from-priority-queue
template<class T, class Container = vector<T>, class Compare = less<typename Container::value_type> >
class PriorityQueue : public std::priority_queue<T, Container, Compare> {
public:
	PriorityQueue(const Compare& compare) : std::priority_queue<T, Container, Compare>(compare) {}

	template<class UnaryPredicate>
	bool find(UnaryPredicate predicate) {
		auto it = std::find_if(this->c.begin(), this->c.end(), predicate);
		if (it != this->c.end()) {
			call(it);
			return true;
		}
		else {
			return false;
		}
	}

	template<class UnaryPredicate>
	bool remove(UnaryPredicate predicate) {
		auto it = std::find_if(this->c.begin(), this->c.end(), predicate);
		if (it != this->c.end()) {
			this->c.erase(it);
			// TODO: next line unnecessary? test
			//std::make_heap(this->c.begin(), this->c.end(), this->comp);
			return true;
		}
		else {
			return false;
		}
	}
};


// Crash on deadline_timer constructor: https://stackoverflow.com/questions/31520493/boostasioio-service-crash-in-win-mutex-lock
class NetworkBase : public cmn::CanLog {
public:
	NetworkBase(std::string name, unsigned int localPort, NetworkHandler* handler) :
		cmn::CanLog(name),
		inFlight(&NetworkMessage::compare),
		netio(),
		retryTimer(netio),
		socket(netio, udp::endpoint(udp::v4(), localPort)),
		handler(handler) {}

	~NetworkBase() {
		stop();
	}

	void start();

	void submit(const udp::endpoint& endpoint, RealDurationType period, CountType retries,
		const std::string& data, const MessageCallbackType& callback);
	void submitReply(const udp::endpoint& endpoint, RealDurationType period, CountType retries,
		const std::string& data, const MessageCallbackType& callback, MsgIdType msgId);

	void sendAndRetry();

	void stop();

protected:
	boost::asio::io_service netio;
	asio::deadline_timer retryTimer;
	udp::socket socket;
	std::thread thread;
	udp::endpoint currentEndpoint;
	boost::array<byte, MAX_MESSAGE_SIZE> currentInput;
	NetworkHandler* handler;
	MsgIdType nextMsgId = 1;
	boost::lockfree::spsc_queue<NetworkMessage, boost::lockfree::capacity<1024>> incoming;
	PriorityQueue<NetworkMessage, std::vector<NetworkMessage>, decltype(&NetworkMessage::compare)> inFlight;

	void runOnce();
	virtual void run() = 0;
	void listen();

	void send(const udp::endpoint& target, const NetworkMessage& item);
	void submitAck(const udp::endpoint& endpoint, const MsgIdType msgId);

	void handleReceive(const boost::system::error_code& error, const size_t numBytes);
	void handleAck(const udp::endpoint& endpoint, const MsgIdType msgId, const std::string& data);
};

}}