#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>

namespace cclat {

/// The pair of CHA index and ingress direction
struct ChaIndexAndIngressDirection {
  uint64_t ChaIndex;
  uint64_t IngressDirection;
};

} // namespace cclat

namespace nlohmann {

template <> struct adl_serializer<cclat::ChaIndexAndIngressDirection> {
  // NOLINTNEXTLINE(readability-identifier-naming)
  static void to_json(json& j, const cclat::ChaIndexAndIngressDirection& Data) {
    j = json::object();
    j["node_id"] = Data.ChaIndex;
    j["channel"] = Data.IngressDirection;
  }
};

} // namespace nlohmann