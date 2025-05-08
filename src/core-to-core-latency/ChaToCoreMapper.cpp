#include "core-to-core-latency/ChaToCoreMapper.hpp"
#include "core-to-core-latency/PcmRingCounters.hpp"
#include "firestarter/CPUTopology.hpp"

#include <algorithm>
#include <cpucounters.h>
#include <cstddef>
#include <cstdint>
#include <sys/types.h>

namespace cclat {

auto ChaToCoreMapper::run(const ChaToCachelinesMap& ChaToCachelines, const std::size_t NumberOfCachelineReads,
                          const std::set<uint64_t>& Cpus, const uint64_t SocketIndex) -> ChaToCoreMap {
  ChaToCoreMap ChaToCore;
  firestarter::CPUTopology Topology;

  // start the counter on all CHAs
  auto* Pcm = pcm::PCM::getInstance();

  PcmRingCounters::programmADRingCounters(Pcm);

  // for each core access cache lines of all cha boxes and find the cache lines with the lowest read latency.
  for (const auto& Cpu : Cpus) {
    // Map from CHA to the sum of all ring counter values
    std::map<uint64_t, uint64_t> ChaToCounterValueSum;

    // Read Cache lines into L3.
    Topology.bindCallerToOsIndex(Cpu);

    for (const auto& [Cha, Cachelines] : ChaToCachelines) {
      auto Before = Pcm->getServerUncoreCounterState(SocketIndex);

      for (auto I = 0; I < NumberOfCachelineReads; I++) {
        for (const auto& VoidCacheline : Cachelines) {
          auto* Cacheline = static_cast<uint8_t*>(VoidCacheline);
          // read/write cache lines. lookups into l3 will occur here.
          *Cacheline = *Cacheline + 1;
        }
      }

      auto After = Pcm->getServerUncoreCounterState(SocketIndex);

      // create the difference of the measurement value.
      std::array<pcm::uint64, 4> RingCounterDifferences = {0, 0, 0, 0};
      for (auto I = 0; I < 4; I++) {
        RingCounterDifferences.at(I) = After.Counters[pcm::PCM::UncorePMUIDs::CBO_PMU_ID][0][Cha][I] -
                                       Before.Counters[pcm::PCM::UncorePMUIDs::CBO_PMU_ID][0][Cha][I];
      }
      ChaToCounterValueSum[Cha] = RingCounterDifferences.at(PcmRingCounters::Direction::Left) +
                                  RingCounterDifferences.at(PcmRingCounters::Direction::Right) +
                                  RingCounterDifferences.at(PcmRingCounters::Direction::Up) +
                                  RingCounterDifferences.at(PcmRingCounters::Direction::Down);
      std::cout << "CHA: " << Cha << "Core: " << Cpu << " difference = " << ChaToCounterValueSum[Cha] << "\n";
    }

    // Select the cha based on the minmal counter value.
    auto MinValueIterator = std::ranges::min_element(
        ChaToCounterValueSum, [](auto& Lhs, auto& Rhs) -> bool { return Lhs.second < Rhs.second; });

    ChaToCore[MinValueIterator->first] = Cpu;
  }

  return ChaToCore;
}

} // namespace cclat
