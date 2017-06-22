#include "stdafx.h"
#include "CppUnitTest.h"

#include <thread>
#include <chrono>

#include "GameServer.h"
#include "GameClient.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace mentics::network;

void log(std::string message) {
	Logger::WriteMessage(message.c_str());
}

namespace mentics { namespace network {
	void outputLog(std::string message) {
		Logger::WriteMessage(message.c_str());
	}

	TEST_CLASS(ClientServerTest)
	{
	public:
		TEST_METHOD(TestClientServer)
		{
			logger = &outputLog;
			GameServer server(1111);
			GameClient client("localhost", 1111);
			std::thread serverThread(&GameServer::start, &server);
			std::thread clientThread(&GameClient::start, &client);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			client.stop();
			server.stop();
			clientThread.join();
			serverThread.join();
		}
	};


}}