#include "stdafx.h"

#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "GameServer.h"

using namespace std;
using namespace boost::asio;
using boost::asio::ip::udp;


namespace mentics {
namespace network {

struct Action {
	string value;
	Action(string newValue) : value(value) {}
};

void GameServer::start() {
	log("GameServer starting...");
	listen();

	while (!netio.stopped()) {
		try {
			netio.run();
		} catch (const exception& e) {
			log("GameServer netio listen exception: " + string(e.what()));
		} catch (...) {
			log("GameServer unknown netio listen exception");
		}
	}
	log("GameServer netio stopped");
}

void GameServer::handleReceive(const boost::system::error_code& error, size_t numBytes) {
	log("handleReceive...");
	if (!error) {
		switch (currentInput[0]) {
		case cmdSubscribe:
			handleSubscribe(currentEndpoint);
			break;
		case cmdAction:
			handleAction(*findClientInfo(currentEndpoint, currentInput), 
				string(reinterpret_cast<char const*>(currentInput.data() + 1), numBytes - 1));
			break;
		case cmdMessage:
			handleMessage(*findClientInfo(currentEndpoint, currentInput), 
				string(reinterpret_cast<char const*>(currentInput.data() + 1), numBytes - 1));
		default:
			log("unknown code received: " + toString((int)currentInput[0]));
		}
	} else {
		// TODO: if we get that "forcibly closed" error, we can remove that client--see example networklib code on github, maybe it does this
		log("Error receiving (from " + toString(currentEndpoint) + "): " + error.message());
	}

	listen();
}

ClientInfo* GameServer::findClientInfo(udp::endpoint& endpoint, boost::array<byte, MAX_MESSAGE_SIZE> in) {
	int clientId = *(uint32_t*)(in.data() + 1);
	ClientInfo* info = clients[clientId];
	if (info == NULL) {
		log("received message for null client id: " + toString(clientId) + ", " + toString(endpoint));
		return NULL;
	} else if (endpoint != clients[clientId]->clientEndpoint) {
		// TODO
		log("endpoint didn't match: " + toString(endpoint) + ", " + toString(endpoint));
		return NULL;
	} else {
		return clients[clientId];
	}
}

void GameServer::handleSubscribe(udp::endpoint& endpoint) {
	log("handleSubscribe "+toString(endpoint));
	// TODO: search to see if endpoint is already there
	// TODO: stop early

	for (ThatType i = 0; i < clients.size(); i++) {
		if (clients[i] == NULL) {
			clients[i] = new ClientInfo(i, endpoint);
			byte response[4];
			writeClientId(i, response, 0);
			log("Sending subscribe ack to " + toString(endpoint));
			send(clients[i]->clientEndpoint, cmdSubscribe, buffer(response, sizeof(ThatType)));
			sendMessageAll("New client " + toString(i) + " joined from " + toString(endpoint), i);
			return;
		}
	}
}

void GameServer::handleMessage(ClientInfo& clientInfo, string message) {
	log("Received from " + toString(clientInfo.clientEndpoint) + " clientId=" + toString(clientInfo.id) + ", message: " + message);
	sendMessageAll(message, clientInfo.id);
}

void GameServer::handleAction(ClientInfo& clientInfo, string action) {
	log("Received message from " + toString(clientInfo.id) + "::" + toString(clientInfo.clientEndpoint) + ": " + action);
	// TODO
}

void GameServer::sendMessageAll(const string message, const ThatType clientId) {
	// TODO: stop early
	for (int i = 0; i < clients.size(); i++) {
		if (i != clientId && clients[i] != NULL) {
			udp::endpoint& target = clients[i]->clientEndpoint;
			log("Sending to " + toString(target) + " message: "+message);
			send(target, cmdMessage, buffer(message));
		}
	}
}

}}
