#pragma once

// TODO: delete this file

#include <cereal/archives/portable_binary.hpp>
#include <string>
#include <iostream>
#include <boost/asio.hpp>

typedef uint8_t byte;
typedef uint32_t ClientIdType;
const ClientIdType INVALID_CLIENT_ID = 10000000;
const boost::asio::ip::udp::endpoint NULL_ENDPOINT;

const int MAX_MESSAGE_SIZE = 1023;

enum Control : byte { AppLevel = 1, Ack = 2 };

namespace mentics { namespace network {

typedef uint16_t GameIdType;

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

//template <typename T>
//inline boost::asio::const_buffer serialize(T& obj) {
//	// TODO: stream to mutable_buffer instead?
//	yas::mem_ostream os;
//	yas::binary_oarchive<yas::mem_ostream> oa(os);
//	oa & obj;
//	yas::intrusive_buffer buf = os.get_intrusive_buffer();
//	return buffer(buf.data, buf.size);
//	//return buffer(os);
//}
//
//template <typename T>
//inline void deserialize(boost::array<byte, MAX_MESSAGE_SIZE> currentInput, uint16_t index, T& obj) {
//	yas::mem_istream is;
//	yas::binary_iarchive<yas::mem_ostream> ia(is);
//	ia & obj;
//}

struct GameInfo {
	GameIdType gameId;

	GameInfo(GameIdType gameId) : gameId(gameId) {}

	template<typename Archive>
	void serialize(Archive& ar) {
		ar(gameId);
	}
};

}}