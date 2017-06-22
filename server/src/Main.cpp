#include "stdafx.h"

#include <iostream>

#include "GameServer.h"

int main()
{
	try
	{
		mentics::network::GameServer server(1111);
		server.start();
		cout << "After run";
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
