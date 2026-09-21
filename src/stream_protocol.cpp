#include "stream_protocol.h"

#include <algorithm>
#include <limits>

namespace stream {
  std::optional<stream_target_t> parse_stream_target(std::string_view target) {
    using namespace std::literals;

    constexpr auto marker = "streamid="sv;
    const auto marker_pos = target.find(marker);
    if (marker_pos != std::string_view::npos) {
      if (marker_pos != 0 && target[marker_pos - 1] != '/') {
        return std::nullopt;
      }
      target.remove_prefix(marker_pos + marker.size());
    }

    target = target.substr(0, target.find_first_of("? "sv));
    const auto first_slash = target.find('/');
    const auto type = target.substr(0, first_slash);
    if (type != "video"sv && type != "audio"sv && type != "control"sv) {
      return std::nullopt;
    }
    if (first_slash == std::string_view::npos) {
      return stream_target_t {type, 0};
    }

    const auto second_slash = target.find('/', first_slash + 1);
    if (second_slash == std::string_view::npos || target.substr(second_slash + 1) != "0"sv) {
      return std::nullopt;
    }
    const auto encoded_index = target.substr(first_slash + 1, second_slash - first_slash - 1);
    if (encoded_index.empty() || (encoded_index.size() > 1 && encoded_index.front() == '0')) {
      return std::nullopt;
    }

    std::uint32_t index = 0;
    for (const char ch : encoded_index) {
      if (ch < '0' || ch > '9') {
        return std::nullopt;
      }
      const auto digit = static_cast<std::uint32_t>(ch - '0');
      if (index > (std::numeric_limits<std::uint32_t>::max() - digit) / 10) {
        return std::nullopt;
      }
      index = index * 10 + digit;
    }
    if (type == "video"sv && index > 1) {
      return std::nullopt;
    }
    return stream_target_t {type, index};
  }

  std::pair<int, int> budget_dual_video_bitrates(
    int primary_kbps,
    int secondary_kbps,
    int ceiling_kbps
  ) {
    if (primary_kbps <= 0 || secondary_kbps <= 0 || ceiling_kbps < 2 ||
        static_cast<std::int64_t>(primary_kbps) + secondary_kbps <= ceiling_kbps) {
      return {primary_kbps, secondary_kbps};
    }

    const auto reserved_secondary = std::min(secondary_kbps, std::max(1, ceiling_kbps / 5));
    auto primary_budget = std::min(primary_kbps, ceiling_kbps - reserved_secondary);
    auto secondary_budget = std::min(secondary_kbps, ceiling_kbps - primary_budget);
    auto remaining = ceiling_kbps - primary_budget - secondary_budget;
    const auto primary_extra = std::min(primary_kbps - primary_budget, remaining);
    primary_budget += primary_extra;
    remaining -= primary_extra;
    secondary_budget += std::min(secondary_kbps - secondary_budget, remaining);
    return {primary_budget, secondary_budget};
  }

  std::optional<control_packet_view_t> decode_control_packet(std::string_view packet_bytes) {
    if (packet_bytes.size() < sizeof(std::uint16_t)) {
      return std::nullopt;
    }
    const auto lo = static_cast<std::uint8_t>(packet_bytes[0]);
    const auto hi = static_cast<std::uint8_t>(packet_bytes[1]);
    const auto type = static_cast<std::uint16_t>(lo | (static_cast<std::uint16_t>(hi) << 8));
    return control_packet_view_t {type, packet_bytes.substr(sizeof(type))};
  }

  std::vector<std::uint8_t> concat_and_insert(
    std::uint64_t insert_size,
    std::uint64_t slice_size,
    std::string_view data1,
    std::string_view data2
  ) {
    if (slice_size == 0) {
      return {};
    }
    std::string joined;
    joined.reserve(data1.size() + data2.size());
    joined.append(data1);
    joined.append(data2);

    const auto slices = (joined.size() + slice_size - 1) / slice_size;
    std::vector<std::uint8_t> result;
    result.reserve(joined.size() + slices * insert_size);
    for (std::size_t offset = 0; offset < joined.size(); offset += slice_size) {
      result.insert(result.end(), insert_size, 0);
      const auto count = std::min<std::size_t>(slice_size, joined.size() - offset);
      result.insert(result.end(), joined.begin() + offset, joined.begin() + offset + count);
    }
    return result;
  }
}  // namespace stream
