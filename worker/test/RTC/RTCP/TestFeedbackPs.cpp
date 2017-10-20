#include "common.hpp"
#include "catch.hpp"
#include "RTC/RTCP/FeedbackPsRemb.hpp"
#include "RTC/RTCP/FeedbackPsTst.hpp"
#include "RTC/RTCP/FeedbackPsVbcm.hpp"
#include <cstring> // std::memcmp()

using namespace RTC::RTCP;

namespace TestFeedbackPsRemb
{
	// RTCP REMB packet.

	// clang-format off
	uint8_t buffer[] =
	{
		0x8f, 0xce, 0x00, 0x06, // Type: 206 (Payload Specific), Count: 15 (AFB), Length: 6
		0xfa, 0x17, 0xfa, 0x17, // Sender SSRC: 0xfa17fa17
		0x00, 0x00, 0x00, 0x00, // Media source SSRC: 0x00000000
		0x52, 0x45, 0x4d, 0x42, // Unique Identifier: REMB
		0x02, 0x01, 0xdf, 0x82, // SSRCs: 2, BR exp: 0, Mantissa: 122754
		0x02, 0xd0, 0x37, 0x02, // SSRC1: 0x02d03702
		0x04, 0xa7, 0x67, 0x47  // SSRC2: 0x04a76747
	};
	// clang-format on

	// REMB values.
	uint32_t senderSsrc = 0xfa17fa17;
	uint32_t mediaSsrc  = 0;
	uint64_t bitrate    = 122754;
	std::vector<uint32_t> ssrcs{ 0x02d03702, 0x04a76747 };

	void verify(FeedbackPsRembPacket* packet)
	{
		REQUIRE(packet->GetSenderSsrc() == senderSsrc);
		REQUIRE(packet->GetMediaSsrc() == mediaSsrc);
		REQUIRE(packet->GetBitrate() == bitrate);
		REQUIRE(packet->GetSsrcs() == ssrcs);
	}
}

SCENARIO("RTCP Feedback PS parsing", "[parser][rtcp][feedback-ps]")
{
	SECTION("parse FeedbackPsTstnItem")
	{
		// clang-format off
		uint8_t buffer[] =
		{
			0x00, 0x00, 0x00, 0x00, // SSRC
			0x08,                   // Seq nr.
			0x00, 0x00, 0x08        // Reserved | Index
		};
		// clang-format on

		uint32_t ssrc = 0;
		uint8_t seq   = 8;
		uint8_t index = 1;

		FeedbackPsTstnItem* item = FeedbackPsTstnItem::Parse(buffer, sizeof(buffer));

		REQUIRE(item);
		REQUIRE(item->GetSsrc() == ssrc);
		REQUIRE(item->GetSequenceNumber() == seq);
		REQUIRE(item->GetIndex() == index);

		delete item;
	}

	SECTION("parse FeedbackPsVbcmItem")
	{
		// clang-format off
		uint8_t buffer[] =
		{
			0x00, 0x00, 0x00, 0x00, // SSRC
			0x08,                   // Seq nr.
			0x02,                   // Zero | Payload Vbcm
			0x00, 0x01,             // Length
			0x01,                   // VBCM Octet String
			0x00, 0x00, 0x00        // Padding
		};
		// clang-format on

		uint32_t ssrc       = 0;
		uint8_t seq         = 8;
		uint8_t payloadType = 1;
		uint16_t length     = 1;
		uint8_t valueMask   = 1;

		FeedbackPsVbcmItem* item = FeedbackPsVbcmItem::Parse(buffer, sizeof(buffer));

		REQUIRE(item);
		REQUIRE(item->GetSsrc() == ssrc);
		REQUIRE(item->GetSequenceNumber() == seq);
		REQUIRE(item->GetPayloadType() == payloadType);
		REQUIRE(item->GetLength() == length);
		REQUIRE((item->GetValue()[item->GetLength() - 1] & 1) == valueMask);

		delete item;
	}

	SECTION("parse FeedbackPsRembPacket")
	{
		using namespace TestFeedbackPsRemb;

		FeedbackPsRembPacket* packet = FeedbackPsRembPacket::Parse(buffer, sizeof(buffer));

		REQUIRE(packet);

		verify(packet);

		SECTION("serialize packet instance")
		{
			uint8_t serialized[sizeof(buffer)] = { 0 };

			packet->Serialize(serialized);

			SECTION("compare serialized packet with original buffer")
			{
				REQUIRE(std::memcmp(buffer, serialized, sizeof(buffer)) == 0);
			}
		}

		delete packet;
	}

	SECTION("create FeedbackPsRembPacket")
	{
		using namespace TestFeedbackPsRemb;

		// Create local report and check content.
		FeedbackPsRembPacket packet(senderSsrc, mediaSsrc);

		packet.SetSsrcs(ssrcs);
		packet.SetBitrate(bitrate);

		verify(&packet);
	}
}