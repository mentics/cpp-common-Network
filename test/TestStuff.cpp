#include "stdafx.h"
#include "CppUnitTest.h"

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include "protocol.h"


using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace mentics { namespace network {

	struct MyClass
	{
		int x, y, z;

		// This method lets cereal know which data members to serialize
		template<class Archive>
		void serialize(Archive & archive)
		{
			archive(x, y, z); // serialize things by passing them to the archive
		}
	};

TEST_CLASS(NetworkTest)
{
public:
	TEST_METHOD(TestSerialize2) {
		std::stringstream ss; // any stream can be used

		{
			cereal::BinaryOutputArchive oarchive(ss); // Create an output archive

			MyClass m1, m2, m3;
			m1.x = 1345;
			oarchive(m1, m2, m3); // Write the data to the archive
		} // archive goes out of scope, ensuring all contents are flushed

		{
			cereal::BinaryInputArchive iarchive(ss); // Create an input archive

			MyClass m1, m2, m3;
			iarchive(m1, m2, m3); // Read the data from the archive

			Assert::AreEqual(1345, m1.x);
		}
	}

	TEST_METHOD(TestSerialize) {
		GameInfo saveInfo(37), loadInfo(-1);

		std::string saved = serialize(saveInfo);
		deserialize(loadInfo, saved);

		//std::istringstream is(saved);
		//cereal::BinaryInputArchive iarchive(is);
		//iarchive(loadInfo);

		Assert::AreEqual<int>(37, loadInfo.gameId);
	}

	template <typename T>
	std::string serialize(const T& obj) {
		std::stringstream ss;
		cereal::BinaryOutputArchive out(ss);
		out(obj);
		return ss.str();
	}


	//void deserialize(GameInfo& obj, std::string data) {
	//	cereal::BinaryInputArchive iarchive(std::istringstream(data));
	//	iarchive(obj);
	//}

	template <typename T>
	void deserialize(T& obj, std::string data) {
		std::istringstream is(data);
		cereal::BinaryInputArchive in(is);
		in(obj);
	}
	
};

}}