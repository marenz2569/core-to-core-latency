#pragma once

#include "core-to-core-latency/ChaIndexAndIngressDirection.hpp"
#include "core-to-core-latency/ChaMeasurementsMap.hpp"
#include <ostream>
#include <sstream>

namespace cclat {

/// The vector of used cha ingress traffic channels and their direction.
using ChaIndexAndIngressDirectionVector = std::vector<ChaIndexAndIngressDirection>;

static auto dump(std::ostream& Stream, const ChaIndexAndIngressDirectionVector& Vec) -> std::ostream& {
  for (const auto& Cid : Vec) {
    dump(Stream, Cid);
  }
  return Stream;
}

/// Convert the CHA measurements to a vector of cha indices and active ingress directions.
/// \arg Cmm The results of the CHA measurement
/// \arg AbsoluteDetectionThreshold The absolute value the counter has to pass for a ingress channel to be classified
/// as active
/// \returns The vector of active CHA ingress directions for all CHAs.
[[nodiscard]] static auto
fromChaMeasurementsMap(const ChaMeasurementsMap& Cmm,
                       const std::size_t AbsoluteDetectionThreshold) -> ChaIndexAndIngressDirectionVector {
  ChaIndexAndIngressDirectionVector Channels;

  for (const auto& [Cha, Measurements] : Cmm) {
    for (auto I = 0; I < 4; I++) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
      if (Measurements[I] > AbsoluteDetectionThreshold) {
        Channels.emplace_back(Cha, I);
      }
    }
  }

  return Channels;
}

}; // namespace cclat

template <> struct std::hash<cclat::ChaIndexAndIngressDirectionVector> {
  auto operator()(cclat::ChaIndexAndIngressDirectionVector const& Vec) const noexcept -> std::size_t {
    std::stringstream Ss;
    std::stringstream Outstream;
    Outstream << dump(Ss, Vec).rdbuf();
    return std::hash<std::string>{}(Outstream.str());
  }
};

namespace nlohmann {

template <> struct adl_serializer<cclat::ChaIndexAndIngressDirectionVector> {
  // NOLINTNEXTLINE(readability-identifier-naming)
  static void to_json(json& j, const cclat::ChaIndexAndIngressDirectionVector& Data) {
    j = json::array();
    for (const auto& Path : Data) {
      json PathJson;
      adl_serializer<cclat::ChaIndexAndIngressDirection>::to_json(PathJson, Path);
      j.emplace_back(PathJson);
    }
  }
};

} // namespace nlohmann