#pragma once

#include "core-to-core-latency/PcmRingCounters.hpp"
#include "types.h"
#include <cstdint>
#include <map>

namespace cclat {
/// Map from the CHA index to the array of PCM measurement values.
using ChaMeasurementsMap = std::map<uint64_t, std::array<pcm::uint64, 4>>;

/// Print a ChaMeasurementsMap
/// \arg Cmm The ChaMeasurementsMap that should be printed
static void dump(const ChaMeasurementsMap& Cmm) {
  for (const auto& [Cha, Values] : Cmm) {
    std::cout << "Cha: " << Cha << " Left: " << Values.at(PcmRingCounters::Direction::Left)
              << " Right: " << Values.at(PcmRingCounters::Direction::Right)
              << " Up: " << Values.at(PcmRingCounters::Direction::Up)
              << " Down: " << Values.at(PcmRingCounters::Direction::Down) << "\n";
  }
}
}; // namespace cclat