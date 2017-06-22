#include "stdafx.h"

#include <string>
#include <iostream>


namespace mentics { namespace network {

void stdoutLog(std::string message) {
	std::cout << message << std::endl;
}

void(*logger)(std::string) = &stdoutLog;

}}