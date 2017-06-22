#include "stdafx.h"

#include <iostream>

#include "GameClient.h"

int main(int argc, char* argv[])
{
	try
	{
		mentics::network::GameClient client("localhost", 1111);
		client.start();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}