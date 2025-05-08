#pragma once

#include "core-to-core-latency/ChaToCoreMapper.hpp"
#include "core-to-core-latency/MeasuredChasAndIngressPathsVector.hpp"
#include <nlohmann/json.hpp>

namespace cclat {

struct ChaCoreMapperResults {
  ChaToCoreMap ChaToCore;
  MeasuredChasAndIngressPathsVector ChasWithIngressPathsVector;
};

} // namespace cclat

namespace nlohmann {

template <> struct adl_serializer<cclat::ChaCoreMapperResults> {
  // NOLINTNEXTLINE(readability-identifier-naming)
  static void to_json(json& j, const cclat::ChaCoreMapperResults& Data) {
    j = json::object();

    auto& ChaToOs = j["CHA_to_os"];
    ChaToOs = json::array();
    // NOLINTNEXTLINE(modernize-loop-convert)
    for (auto I = 0; I < Data.ChaToCore.size(); I++) {
      ChaToOs.emplace_back(Data.ChaToCore.at(I));
    }

    adl_serializer<cclat::MeasuredChasAndIngressPathsVector>::to_json(j["busy_paths"], Data.ChasWithIngressPathsVector);
  }
};

} // namespace nlohmann