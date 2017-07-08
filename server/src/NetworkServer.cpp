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
using namespace std;
using namespace boost::asio;
using boost::asio::ip::udp;

struct Action {
	string value;
	Action(string newValue) : value(value) {}
};

void NetworkServer::start() {
	log("NetworkServer starting...");
	listen();

	while (!netio.stopped()) {
		try {
			netio.run();
		} catch (const exception& e) {
			log("NetworkServer netio listen exception: " + string(e.what()));
		} catch (...) {
			log("NetworkServer unknown netio listen exception");
		}
	}
	log("NetworkServer stopped");
}

void NetworkServer::handleReceive(const boost::system::error_code& error, size_t numBytes) {
	log("handleReceive...");
	if (!error) {
		switch (currentInput[0]) {
		case cmdSubscribe:
			handleSubscribe(currentEndpoint);
			break;
		case cmdAction:
			handleAction(findClientInfo(currentEndpoint, currentInput), 
				string(reinterpret_cast<char const*>(currentInput.data() + 1), numBytes - 1));
			break;
		case cmdMessage:
			handleMessage(findClientInfo(currentEndpoint, currentInput), 
				string(reinterpret_cast<char const*>(currentInput.data() + 1), numBytes - 1));
		default:
			log("unknown code received: " + cmn::toString((int)currentInput[0]));
		}
	} else {
		// TODO: if we get that "forcibly closed" error, we can remove that client--see example networklib code on github, maybe it does this
		log("Error receiving (from " + cmn::toString(currentEndpoint) + "): " + error.message());
	}

	listen();
}

ClientIdType NetworkServer::findClientInfo(udp::endpoint& endpoint, boost::array<byte, MAX_MESSAGE_SIZE> in) {
	ClientIdType clientId = *(ClientIdType*)(in.data() + 1);
	if (clients[clientId].id == INVALID_CLIENT_ID) {
		log("Received message for null client info: " + cmn::toString(clientId) + ", " + cmn::toString(endpoint));
		return INVALID_CLIENT_ID;
	} else if (endpoint != clients[clientId].clientEndpoint) {
		// TODO
		log("Endpoint didn't match: " + cmn::toString(endpoint) + ", " + cmn::toString(clients[clientId].clientEndpoint));
		return INVALID_CLIENT_ID;
	} else {
		return clientId;
	}
}

void NetworkServer::handleSubscribe(udp::endpoint& endpoint) {
	log("handleSubscribe "+cmn::toString(endpoint));
	// TODO: search to see if endpoint is already there
	// TODO: stop early

	for (ClientIdType i = 0; i < clients.size(); i++) {
		if (clients[i].id == INVALID_CLIENT_ID) {
			clients[i] = ClientInfo(i, endpoint);
			byte response[4];
			writeClientId(i, response, 0);
			log("Sending subscribe ack to Client" + cmn::toString(i) + " at " + cmn::toString(endpoint));
			send(clients[i].clientEndpoint, cmdSubscribe, buffer(response, sizeof(ClientIdType)));
			sendMessageAll("New client " + cmn::toString(i) + " joined from " + cmn::toString(endpoint), i);
			return;
		}
	}
}

void NetworkServer::handleMessage(ClientIdType clientId, string message) {
	log("Received from Client" + cmn::toString(clientId) + " at " + cmn::toString(clients[clientId].clientEndpoint) + ", message " + message);
	sendMessageAll(message, clientId);
}

void NetworkServer::handleAction(ClientIdType clientId, string action) {
	log("Received from Client" + cmn::toString(clientId) + " at " + cmn::toString(clients[clientId].clientEndpoint) + " action " + action);
	// TODO
}

void NetworkServer::sendMessageAll(const string message, const ClientIdType clientId) {
	// TODO: stop early
	for (int i = 0; i < clients.size(); i++) {
		if (i != clientId && clients[i].id != INVALID_CLIENT_ID) {
			udp::endpoint& target = clients[i].clientEndpoint;
			log("Sending to Client" + cmn::toString(i) + " at " + cmn::toString(target) + " message: "+message);
			send(target, cmdMessage, buffer(message));
		}
	}
}

}}
