#include "stdafx.h"
#include "CppUnitTest.h"

#include <string>
#include <thread>
#include <chrono>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "NetworkServer.h"
#include "NetworkClient.h"

#include "MenticsCommonTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace mentics { namespace network {

TEST_CLASS(ClientServerTest)
{
	static boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> lg;
	const std::string name = "ClientServerTest";

public:
	TEST_CLASS_INITIALIZE(BeforeClass) {
		mentics::test::setupLog();
	}

	TEST_METHOD(TestClientServer)
	{
		class ServerNetworkHandler : public NetworkHandler {
			boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> lg;
			const std::string name = "ServerNetworkHandler";
		public:
			void handle(udp::endpoint& endpoint, const std::string& data) override {
				LOG(lvl::trace) << "handle(" << endpoint << ", " << data << ")";
			}
			void handleError(udp::endpoint& endpoint, const boost::system::error_code& error) override {
				LOG(lvl::trace) << "handleError(" << endpoint << ", " << error << ")";
			}
		};

		class ClientNetworkHandler : public NetworkHandler {
			boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> lg;
			const std::string name = "ClientNetworkHandler";
		public:
			void handle(udp::endpoint& endpoint, const std::string& data) override {
				LOG(lvl::trace) << "handle(" << endpoint << ", " << data << ")";
			}
			void handleError(udp::endpoint& endpoint, const boost::system::error_code& error) override {
				LOG(lvl::trace) << "handleError(" << endpoint << ", " << error << ")";
			}
		};

		ServerNetworkHandler serverHandler;
		ClientNetworkHandler clientHandler;

		//NetworkServer server(1111, &serverHandler);
		//std::vector<NetworkClient> clients;
		//for (int i = 0; i < 5; i++) {
		//	clients.emplace_back("localhost", 1111, &clientHandler);
		//}

		//clients[0].submit(ptime::milliseconds(100), 0, std::string("test"),
		//	[](udp::endpoint endpoint, std::string data) { 
		//});

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}



//#define MAX_CLIENTS 10
//
//	int numClients;
//	NetworkServer* server;
//	thread* serverThread;
//	array<NetworkClient*, MAX_CLIENTS> clients;
//	array<thread*, MAX_CLIENTS> threads;
//
//	TEST_METHOD_INITIALIZE(before) {
//	}
//
//	TEST_METHOD_CLEANUP(after) {
//		destroyClients();
//		destroyServer();
//	}
//
//	void createServer() {
//		server = new NetworkServer(1111);
//		serverThread = new thread(&NetworkServer::start, server);
//	}
//
//	void destroyServer() {
//		server->stop();
//		serverThread->join();
//		delete server;
//		delete serverThread;
//	}
//
//	void createClients(int num) {
//		numClients = num;
//		for (int i = 0; i < numClients; i++) {
//			clients[i] = new NetworkClient("localhost", 1111);
//			threads[i] = new thread(&NetworkClient::start, clients[i]);
//		}
//	}
//
//	void destroyClients() {
//		for (int i = 0; i < numClients; i++) {
//			clients[i]->stop();
//		}
//		for (int i = 0; i < numClients; i++) {
//			threads[i]->join();
//			delete clients[i];
//			delete threads[i];
//		}
//		//std::this_thread::sleep_for(std::chrono::milliseconds(200));
//		//for (int i = 0; i < numClients; i++) {
//		//}
//	}
//
//public:
//	TEST_METHOD(TestClientServer)
//	{
//		createServer();
//		createClients(5);
//		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//	}
};

}}