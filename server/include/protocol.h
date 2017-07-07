#pragma once

#include <stdint.h>
#include <WinSock2.h>
#include <string>
#include <iostream>

typedef uint8_t byte;
typedef uint32_t ClientIdType;

const int MAX_MESSAGE_SIZE = 1023;

enum Command : byte { cmdSubscribe=1, cmdMessage=2, cmdAction=3 };

namespace mentics { namespace network {

void stdoutLog(std::string message);

extern void(*logger)(std::string);

inline void writeClientId(ClientIdType value, byte* buffer, uint32_t index) {
	value = htonl(value);
	byte* bytes = (byte*)&value;
	buffer[index] = bytes[0];
	buffer[index + 1] = bytes[1];
	buffer[index + 2] = bytes[2];
	buffer[index + 3] = bytes[3];
}

inline ClientIdType readClientId(byte* buffer, uint32_t index) {
	ClientIdType value = *(ClientIdType*)(buffer + index);
	return ntohl(value);
}

}}