#include "stdafx.h"

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "GameClient.h"

namespace mentics { namespace network {

using namespace std;
using namespace boost::asio;
using boost::asio::ip::udp;

void GameClient::start() {
	log("Client starting...");
	udp::resolver resolver(netio);
	udp::resolver::query query(udp::v4(), serverHost, std::to_string(serverPort));
	serverEndpoint = *resolver.resolve(query);

	listen();

	sendSubscribe();

	while (!netio.stopped()) {
		try {
			netio.run();
		}
		catch (const std::exception& e) {
			log("Client: network exception: " + toString(e.what()));
		}
		catch (...) {
			log("Unknown exception in client network thread");
		}
	}
	log("GameClient stopped");
}

void GameClient::handleReceive(const boost::system::error_code& error, size_t numBytes) {
	log("handleReceive received "+toString(numBytes));
	byte firstValue = currentInput[0];
	switch (firstValue) {
	case cmdSubscribe:
		handleSubscribe(readClientId(currentInput.data(), 1));
		break;
	case cmdMessage:
		handleMessage(string(reinterpret_cast<char const*>(currentInput.data() + 1), numBytes - 1));
		break;
	case cmdAction:
		log("action handler not implemented yet");
		break;
	default:
		log("unknown code received: " + toString((int)currentInput[0]));
	}
	listen();
}

void GameClient::handleSubscribe(ThatType cid) {
	clientId = cid;
	name.append(toString(clientId));
	log("Subscribe acknowledged with clientId=" + toString(clientId));
}

void GameClient::handleMessage(string message) {
	log("Received message: " + message);
}

void GameClient::sendSubscribe() {
	log("Sending subscribe to server " + toString(serverEndpoint));
	send(serverEndpoint, cmdSubscribe, buffer(this, 0));
}

void GameClient::sendMessage(string message) {
	log("Sending to server " + toString(serverEndpoint) + " message: "+message);
	send(serverEndpoint, cmdMessage, buffer(message));
}

}}