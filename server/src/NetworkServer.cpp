#include "stdafx.h"

#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "MenticsCommon.h"
#include "NetworkServer.h"

namespace mentics { namespace network {

namespace cmn = mentics::common;
namespace lvl = boost::log::trivial;
namespace asio = boost::asio;
using boost::asio::ip::udp;

struct Action {
	std::string value;
	Action(std::string newValue) : value(value) {}
};

void NetworkServer::start() {
	LOG(lvl::info) << "NetworkServer starting...";
	listen();

	while (!netio.stopped()) {
		try {
			netio.run();
		} catch (const std::exception& e) {
			LOG(lvl::error) << "NetworkServer netio listen exception: " << std::string(e.what());
		} catch (...) {
			LOG(lvl::error) << "NetworkServer unknown netio listen exception";
		}
	}
	LOG(lvl::info) << "NetworkServer stopped";
}

void NetworkServer::handleReceive(const boost::system::error_code& error, size_t numBytes) {
	LOG(lvl::trace) << "handleReceive...";
	if (!error) {
		switch (currentInput[0]) {
		case cmdSubscribe:
			handleSubscribe(currentEndpoint);
			break;
		case cmdAction:
			handleAction(findClientInfo(currentEndpoint, currentInput), 
				std::string(reinterpret_cast<char const*>(currentInput.data() + 1), numBytes - 1));
			break;
		case cmdMessage:
			handleMessage(findClientInfo(currentEndpoint, currentInput), 
				std::string(reinterpret_cast<char const*>(currentInput.data() + 1), numBytes - 1));
		default:
			LOG(lvl::error) << "unknown code received: " << cmn::toString((int)currentInput[0]);
		}
	} else {
		// TODO: if we get that "forcibly closed" error, we can remove that client--see example networklib code on github, maybe it does this
		LOG(lvl::error) << "Error receiving (from " << cmn::toString(currentEndpoint) << "): " << error.message();
	}

	listen();
}

ClientIdType NetworkServer::findClientInfo(udp::endpoint& endpoint, boost::array<byte, MAX_MESSAGE_SIZE> in) {
	ClientIdType clientId = *(ClientIdType*)(in.data() + 1);
	if (clients[clientId].id == INVALID_CLIENT_ID) {
		LOG(lvl::warning) << "Received message for null client info: " << clientId << ", " << cmn::toString(endpoint);
		return INVALID_CLIENT_ID;
	} else if (endpoint != clients[clientId].clientEndpoint) {
		// TODO
		LOG(lvl::error) << "Endpoint didn't match: " << cmn::toString(endpoint) << ", " << cmn::toString(clients[clientId].clientEndpoint);
		return INVALID_CLIENT_ID;
	} else {
		return clientId;
	}
}

void NetworkServer::handleSubscribe(udp::endpoint& endpoint) {
	LOG(lvl::info) << "handleSubscribe " << endpoint;
	// TODO: search to see if endpoint is already there
	// TODO: stop early

	for (ClientIdType i = 0; i < clients.size(); i++) {
		if (clients[i].id == INVALID_CLIENT_ID) {
			clients[i] = ClientInfo(i, endpoint);
			byte response[4];
			writeClientId(i, response, 0);
			LOG(lvl::trace) << "Sending subscribe ack to Client" << i << " at " << endpoint;
			send(clients[i].clientEndpoint, cmdSubscribe, asio::buffer(response, sizeof(ClientIdType)));
			sendMessageAll("New client " + cmn::toString(i) + " joined from " + cmn::toString(endpoint), i);
			return;
		}
	}
}

void NetworkServer::handleMessage(ClientIdType clientId, std::string message) {
	LOG(lvl::info) << "Received from Client" << clientId << " at " << cmn::toString(clients[clientId].clientEndpoint) + ", message " + message;
	sendMessageAll(message, clientId);
}

void NetworkServer::handleAction(ClientIdType clientId, std::string action) {
	LOG(lvl::info) << "Received from Client" << clientId << " at " << cmn::toString(clients[clientId].clientEndpoint) << " action " << action;
	// TODO
}

void NetworkServer::sendMessageAll(const std::string message, const ClientIdType clientId) {
	// TODO: stop early
	for (int i = 0; i < clients.size(); i++) {
		if (i != clientId && clients[i].id != INVALID_CLIENT_ID) {
			udp::endpoint& target = clients[i].clientEndpoint;
			LOG(lvl::info) << "Sending to Client" << i << " at " << target << " message: " << message;
			send(target, cmdMessage, asio::buffer(message));
		}
	}
}

}}
