/**
 * @file tests/unit/test_stream.cpp
 * @brief Test src/stream.*
 */

#include "../tests_common.h"
#include "src/stream_protocol.h"

#include <climits>

TEST(VideoFormatNameTests, CanonicalCodecNameNormalizesKnownAliases) {
  EXPECT_EQ(stream::canonical_codec_name("h264"), "H.264");
  EXPECT_EQ(stream::canonical_codec_name("H.264"), "H.264");
  EXPECT_EQ(stream::canonical_codec_name("hevc"), "HEVC");
  EXPECT_EQ(stream::canonical_codec_name("H265"), "HEVC");
  EXPECT_EQ(stream::canonical_codec_name("av1"), "AV1");
}

TEST(VideoFormatNameTests, CanonicalCodecNamePreservesUnknownValues) {
  EXPECT_EQ(stream::canonical_codec_name("vp9"), "vp9");
  EXPECT_TRUE(stream::canonical_codec_name({}).empty());
}

TEST(ConcatAndInsertTests, ConcatNoInsertionTest) {
  char b1[] = {'a', 'b'};
  char b2[] = {'c', 'd', 'e'};
  auto res = stream::concat_and_insert(0, 2, std::string_view {b1, sizeof(b1)}, std::string_view {b2, sizeof(b2)});
  auto expected = std::vector<uint8_t> {'a', 'b', 'c', 'd', 'e'};
  ASSERT_EQ(res, expected);
}

TEST(ConcatAndInsertTests, ConcatLargeStrideTest) {
  char b1[] = {'a', 'b'};
  char b2[] = {'c', 'd', 'e'};
  auto res = stream::concat_and_insert(1, sizeof(b1) + sizeof(b2) + 1, std::string_view {b1, sizeof(b1)}, std::string_view {b2, sizeof(b2)});
  auto expected = std::vector<uint8_t> {0, 'a', 'b', 'c', 'd', 'e'};
  ASSERT_EQ(res, expected);
}

TEST(ConcatAndInsertTests, ConcatSmallStrideTest) {
  char b1[] = {'a', 'b'};
  char b2[] = {'c', 'd', 'e'};
  auto res = stream::concat_and_insert(1, 1, std::string_view {b1, sizeof(b1)}, std::string_view {b2, sizeof(b2)});
  auto expected = std::vector<uint8_t> {0, 'a', 0, 'b', 0, 'c', 0, 'd', 0, 'e'};
  ASSERT_EQ(res, expected);
}

TEST(ControlPacketParsing, RejectsRuntPacketsBeforeReadingType) {
  EXPECT_FALSE(stream::decode_control_packet({}));

  const char one_byte[] = {'\x34'};
  EXPECT_FALSE(stream::decode_control_packet(std::string_view {one_byte, sizeof(one_byte)}));
}

TEST(ControlPacketParsing, DecodesTypeAndPayloadSafely) {
  const char packet[] = {'\x34', '\x12', 'a', 'b'};

  const auto decoded = stream::decode_control_packet(std::string_view {packet, sizeof(packet)});

  ASSERT_TRUE(decoded);
  EXPECT_EQ(decoded->type, 0x1234);
  EXPECT_EQ(decoded->payload, "ab");
}

TEST(ControlPacketParsing, AllowsTypeOnlyPacketWithoutPayloadUnderflow) {
  const char packet[] = {'\x34', '\x12'};

  const auto decoded = stream::decode_control_packet(std::string_view {packet, sizeof(packet)});

  ASSERT_TRUE(decoded);
  EXPECT_EQ(decoded->type, 0x1234);
  EXPECT_TRUE(decoded->payload.empty());
}

TEST(StreamTargetParsing, PreservesLegacyTargetsAsStreamZero) {
  for (const auto target : {"video", "audio", "control", "rtsp://host/streamid=video"}) {
    const auto parsed = stream::parse_stream_target(target);
    ASSERT_TRUE(parsed) << target;
    EXPECT_EQ(parsed->index, 0U) << target;
  }
}

TEST(StreamTargetParsing, AcceptsBothSupportedIndexedVideoTargets) {
  const auto primary = stream::parse_stream_target("rtsp://host/streamid=video/0/0");
  const auto secondary = stream::parse_stream_target("streamid=video/1/0");

  ASSERT_TRUE(primary);
  EXPECT_EQ(primary->type, "video");
  EXPECT_EQ(primary->index, 0U);
  ASSERT_TRUE(secondary);
  EXPECT_EQ(secondary->type, "video");
  EXPECT_EQ(secondary->index, 1U);
}

TEST(StreamTargetParsing, RejectsMalformedOrUnsupportedVideoIndices) {
  for (const auto target : {
         "video/2/0",
         "video/-1/0",
         "video/01/0",
         "video/1",
         "video//0",
         "video/1/1",
         "video/1/0/extra",
         "prefixstreamid=video/1/0",
         "streamid=video/1x/0",
       }) {
    EXPECT_FALSE(stream::parse_stream_target(target)) << target;
  }
}

TEST(DualVideoBitrateBudget, LeavesRequestsWithinCeilingUnchanged) {
  EXPECT_EQ(stream::budget_dual_video_bitrates(6000, 3000, 10000), (std::pair {6000, 3000}));
  EXPECT_EQ(stream::budget_dual_video_bitrates(6000, 3000, 0), (std::pair {6000, 3000}));
}

TEST(DualVideoBitrateBudget, CapsAggregateAndPreservesPrimaryFirst) {
  EXPECT_EQ(stream::budget_dual_video_bitrates(8000, 8000, 10000), (std::pair {8000, 2000}));
  EXPECT_EQ(stream::budget_dual_video_bitrates(3000, 9000, 10000), (std::pair {3000, 7000}));
}

TEST(DualVideoBitrateBudget, HandlesOverflowWithoutExceedingCeiling) {
  const auto budget = stream::budget_dual_video_bitrates(INT_MAX, INT_MAX, 10000);

  EXPECT_EQ(budget.first, 8000);
  EXPECT_EQ(budget.second, 2000);
  EXPECT_LE(static_cast<std::int64_t>(budget.first) + budget.second, 10000);
}
