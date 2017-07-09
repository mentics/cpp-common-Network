#include "stdafx.h"
#include "CppUnitTest.h"

#include <thread>
#include <chrono>

#include "NetworkServer.h"
#include "NetworkClient.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace mentics::network;


namespace mentics { namespace network {

TEST_CLASS(ClientServerTest)
{
#define MAX_CLIENTS 10

	int numClients;
	NetworkServer* server;
	thread* serverThread;
	array<NetworkClient*, MAX_CLIENTS> clients;
	array<thread*, MAX_CLIENTS> threads;

	TEST_METHOD_INITIALIZE(before) {
	}

	TEST_METHOD_CLEANUP(after) {
		destroyClients();
		destroyServer();
	}

	void createServer() {
		server = new NetworkServer(1111);
		serverThread = new thread(&NetworkServer::start, server);
	}

	void destroyServer() {
		server->stop();
		serverThread->join();
		delete server;
		delete serverThread;
	}

	void createClients(int num) {
		numClients = num;
		for (int i = 0; i < numClients; i++) {
			clients[i] = new NetworkClient("localhost", 1111);
			threads[i] = new thread(&NetworkClient::start, clients[i]);
		}
	}

	void destroyClients() {
		for (int i = 0; i < numClients; i++) {
			clients[i]->stop();
		}
		for (int i = 0; i < numClients; i++) {
			threads[i]->join();
			delete clients[i];
			delete threads[i];
		}
		//std::this_thread::sleep_for(std::chrono::milliseconds(200));
		//for (int i = 0; i < numClients; i++) {
		//}
	}

public:
	TEST_METHOD(TestClientServer)
	{
		createServer();
		createClients(5);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
};

}}