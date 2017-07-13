#include "stdafx.h"

#include <iostream>

#include "NetworkServer.h"

int main()
{
	try
	{
		mentics::network::NetworkServer server(1111, nullptr);
		server.start();
		std::cout << "After run";
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
