#include "stdafx.h"

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "MenticsCommon.h"
#include "NetworkClient.h"

namespace mentics { namespace network {

namespace cmn = mentics::common;
namespace lvl = boost::log::trivial;
namespace asio = boost::asio;
using boost::asio::ip::udp;

void NetworkClient::start() {
	LOG(lvl::info) << "Client starting...";
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
			LOG(lvl::error) << "Client: network exception: " << cmn::toString(e.what());
		}
		catch (...) {
			LOG(lvl::error) << "Unknown exception in client network thread";
		}
	}
	LOG(lvl::info) << "NetworkClient stopped";
}

void NetworkClient::createGame(std::function<void(const GameInfo&)> callback) {
	sendWithId(cmdCreateGame);
}

void NetworkClient::joinGame(GameIdType gameId) {
	//sendWithId(cmdJoinGame, serialize(gameId));
}


void NetworkClient::sendSubscribe() {
	LOG(lvl::trace) << "Sending subscribe to server " << cmn::toString(serverEndpoint);
	send(serverEndpoint, cmdSubscribe, asio::buffer(this, 0));
}

void NetworkClient::sendWithId(Command cmd) {
	byte response[4];
	writeClientId(clientId, response, 0);
	send(serverEndpoint, cmd, asio::buffer(response, sizeof(clientId)));
}

void NetworkClient::sendWithId(Command cmd, const asio::const_buffer& buffers) {
	byte response[4];
	writeClientId(clientId, response, 0);
	std::array<asio::const_buffer, 2> bufs = { asio::buffer(response, sizeof(clientId)), buffers };
	send(serverEndpoint, cmd, buffer(bufs));
}

void NetworkClient::sendMessage(std::string message) {
	LOG(lvl::trace) << "Sending to server " << cmn::toString(serverEndpoint) << " message: "+message;
	send(serverEndpoint, cmdMessage, asio::buffer(message));
}

void NetworkClient::handleReceive(const boost::system::error_code& error, size_t numBytes) {
	LOG(lvl::trace) << "handleReceive received " << cmn::toString(numBytes);
	byte firstValue = currentInput[0];
	switch (firstValue) {
	case cmdSubscribe:
		handleSubscribe(readClientId(currentInput.data(), 1));
		break;
	case cmdMessage:
		handleMessage(std::string(reinterpret_cast<char const*>(currentInput.data() + 1), numBytes - 1));
		break;
	case cmdAction:
		LOG(lvl::error) << "action handler not implemented yet";
		break;
	default:
		LOG(lvl::error) << "unknown code received: " << cmn::toString((int)currentInput[0]);
	}
	listen();
}

void NetworkClient::handleSubscribe(ClientIdType cid) {
	clientId = cid;
	name.append(cmn::toString(clientId));
	LOG(lvl::info) << "Subscribe acknowledged with clientId=" << cmn::toString(clientId);
}

void NetworkClient::handleMessage(std::string message) {
	LOG(lvl::info) << "Received message: " << message;
}

}}