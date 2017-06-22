#pragma once

#include <stdint.h>
#include <WinSock2.h>
#include <string>
#include <iostream>

typedef uint8_t byte;
typedef uint32_t ThatType;

const int MAX_MESSAGE_SIZE = 1023;

enum Command { cmdSubscribe=1, cmdMessage=2, cmdAction=3 };

namespace mentics { namespace network {

using namespace std;

void stdoutLog(string message);

extern void(*logger)(string);

template <typename T>
inline std::string toString(const T& object) {
	std::ostringstream ss;
	ss << object;
	return ss.str();
}

inline void writeClientId(ThatType value, byte* buffer, uint32_t index) {
	value = htonl(value);
	byte* bytes = (byte*)&value;
	buffer[index] = bytes[0];
	buffer[index + 1] = bytes[1];
	buffer[index + 2] = bytes[2];
	buffer[index + 3] = bytes[3];
}

inline ThatType readClientId(byte* buffer, uint32_t index) {
	ThatType value = *(ThatType*)(buffer + index);
	return ntohl(value);
}

}}