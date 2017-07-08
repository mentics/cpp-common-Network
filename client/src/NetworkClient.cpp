#include "stdafx.h"

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "MenticsCommon.h"
#include "NetworkClient.h"

namespace mentics { namespace network {

namespace cmn = mentics::common;
using namespace std;
using namespace boost::asio;
using boost::asio::ip::udp;

void NetworkClient::start() {
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
			log("Client: network exception: " + cmn::toString(e.what()));
		}
		catch (...) {
			log("Unknown exception in client network thread");
		}
	}
	log("NetworkClient stopped");
}

void NetworkClient::createGame(std::function<void(const GameInfo&)> callback) {
	sendWithId(cmdCreateGame);
}

void NetworkClient::joinGame(GameIdType gameId) {
	//sendWithId(cmdJoinGame, serialize(gameId));
}


void NetworkClient::sendSubscribe() {
	log("Sending subscribe to server " + cmn::toString(serverEndpoint));
	send(serverEndpoint, cmdSubscribe, buffer(this, 0));
}

void NetworkClient::sendWithId(Command cmd) {
	byte response[4];
	writeClientId(clientId, response, 0);
	send(serverEndpoint, cmd, buffer(response, sizeof(clientId)));
}

void NetworkClient::sendWithId(Command cmd, const const_buffer& buffers) {
	byte response[4];
	writeClientId(clientId, response, 0);
	std::array<const_buffer, 2> bufs = { buffer(response, sizeof(clientId)), buffers };
	send(serverEndpoint, cmd, buffer(bufs));
}

void NetworkClient::sendMessage(string message) {
	log("Sending to server " + cmn::toString(serverEndpoint) + " message: "+message);
	send(serverEndpoint, cmdMessage, buffer(message));
}

void NetworkClient::handleReceive(const boost::system::error_code& error, size_t numBytes) {
	log("handleReceive received " + cmn::toString(numBytes));
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
		log("unknown code received: " + cmn::toString((int)currentInput[0]));
	}
	listen();
}

void NetworkClient::handleSubscribe(ClientIdType cid) {
	clientId = cid;
	name.append(cmn::toString(clientId));
	log("Subscribe acknowledged with clientId=" + cmn::toString(clientId));
}

void NetworkClient::handleMessage(string message) {
	log("Received message: " + message);
}

}}