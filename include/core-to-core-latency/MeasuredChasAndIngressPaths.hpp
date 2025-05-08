#pragma once

#include "core-to-core-latency/ChaIndexAndIngressDirectionVector.hpp"
#include <nlohmann/json.hpp>

namespace cclat {

/// The map of local and remote CHAs and the CHA ingress paths
struct MeasuredChasAndIngressPaths {
  uint64_t LocalCha;
  uint64_t RemoteCha;
  ChaIndexAndIngressDirectionVector IngressPaths;
};

} // namespace cclat

namespace nlohmann {

template <> struct adl_serializer<cclat::MeasuredChasAndIngressPaths> {
  // NOLINTNEXTLINE(readability-identifier-naming)
  static void to_json(json& j, const cclat::MeasuredChasAndIngressPaths& Data) {
    j = json::object();
    j["id_A"] = Data.LocalCha;
    j["id_B"] = Data.RemoteCha;
    adl_serializer<cclat::ChaIndexAndIngressDirectionVector>::to_json(j["path"], Data.IngressPaths);
  }
};

} // namespace nlohmann