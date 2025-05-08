#pragma once

#include "core-to-core-latency/MeasuredChasAndIngressPaths.hpp"
#include <nlohmann/json.hpp>

namespace cclat {

/// The vector of MeasuredChasAndIngressPaths
using MeasuredChasAndIngressPathsVector = std::vector<MeasuredChasAndIngressPaths>;

} // namespace cclat

namespace nlohmann {

template <> struct adl_serializer<cclat::MeasuredChasAndIngressPathsVector> {
  // NOLINTNEXTLINE(readability-identifier-naming)
  static void to_json(json& j, const cclat::MeasuredChasAndIngressPathsVector& Data) {
    j = json::array();
    for (const auto& ChasWithIngress : Data) {
      json ChasWithIngressJson;
      adl_serializer<cclat::MeasuredChasAndIngressPaths>::to_json(ChasWithIngressJson, ChasWithIngress);
      j.emplace_back(ChasWithIngressJson);
    }
  }
};

} // namespace nlohmann