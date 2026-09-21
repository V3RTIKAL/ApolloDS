#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace stream {
  struct stream_target_t {
    std::string_view type;
    std::uint32_t index;
  };

  [[nodiscard]] std::optional<stream_target_t> parse_stream_target(std::string_view target);

  [[nodiscard]] std::pair<int, int> budget_dual_video_bitrates(
    int primary_kbps,
    int secondary_kbps,
    int ceiling_kbps
  );

  inline std::string canonical_codec_name(std::string_view codec) {
    if (codec.empty()) {
      return {};
    }
    std::string lowered(codec);
    for (char &ch : lowered) {
      if (ch >= 'A' && ch <= 'Z') {
        ch = static_cast<char>(ch - 'A' + 'a');
      }
    }
    if (lowered == "h264" || lowered == "h.264") return "H.264";
    if (lowered == "h265" || lowered == "hevc") return "HEVC";
    if (lowered == "av1") return "AV1";
    return std::string(codec);
  }

  struct control_packet_view_t {
    std::uint16_t type = 0;
    std::string_view payload;
  };

  enum class secondary_stream_state_e : std::uint8_t {
    disabled,
    negotiated,
    connecting,
    running,
    ended,
  };

  struct ref_frame_invalidation_t {
    std::int64_t first_frame = 0;
    std::int64_t last_frame = 0;
    std::uint8_t stream_index = 0;
  };

  [[nodiscard]] std::optional<std::uint8_t> validate_display_index(std::uint16_t encoded_index);
  [[nodiscard]] std::uint32_t touch_pointer_id(std::uint32_t pointer_id, std::uint8_t display_index);
  [[nodiscard]] bool secondary_stream_accepts_control(secondary_stream_state_e state);
  [[nodiscard]] std::optional<std::uint8_t> parse_idr_stream_index(std::string_view payload);
  [[nodiscard]] std::optional<ref_frame_invalidation_t> parse_ref_frame_invalidation(std::string_view payload);

  std::optional<control_packet_view_t> decode_control_packet(std::string_view packet_bytes);
  std::vector<std::uint8_t> concat_and_insert(
    std::uint64_t insert_size,
    std::uint64_t slice_size,
    std::string_view data1,
    std::string_view data2
  );
}  // namespace stream
