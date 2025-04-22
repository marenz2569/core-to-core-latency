#include "core-to-core-latency/ChaToCoreMapper.hpp"
#include "firestarter/CPUTopology.hpp"

#include <cpucounters.h>
#include <cstddef>
#include <cstdint>

namespace cclat {

auto ChaToCoreMapper::run(const ChaToCachelinesMap& ChaToCachelines, const std::size_t NumberOfCachelineReads,
                          const std::set<uint64_t>& Cpus) -> CoreToChaMap {
  CoreToChaMap CoreToCha;
  firestarter::CPUTopology Topology;
  auto SocketIndex = 0U;

  // start the counter on all CHAs
  auto* Pcm = pcm::PCM::getInstance();
  // TODO: add socket index
  auto Pmu = pcm::ServerUncorePMUs(/*socket_=*/SocketIndex, /*pcm=*/Pcm);

  {
    std::array<pcm::uint64, 4> CboConfigMap = {0, 0, 0, 0};

    switch (Pcm->getCPUFamilyModel()) {
    // Skylake-X
    // perfmon/SKX/events/skylakex_uncore_experimental.json
    case pcm::PCM::SKX:
      // UNC_CHA_HORZ_RING_AD_IN_USE.LEFT_EVEN + UNC_CHA_HORZ_RING_AD_IN_USE.LEFT_ODD
      CboConfigMap[0] = CBO_MSR_PMON_CTL_EVENT(0xa7) + CBO_MSR_PMON_CTL_UMASK((1 + 2));
      // UNC_CHA_HORZ_RING_AD_IN_USE.RIGHT_EVEN + UNC_CHA_HORZ_RING_AD_IN_USE.RIGHT_ODD
      CboConfigMap[1] = CBO_MSR_PMON_CTL_EVENT(0xa7) + CBO_MSR_PMON_CTL_UMASK((4 + 8));
      // UNC_CHA_VERT_RING_AD_IN_USE.UP_EVEN + UNC_CHA_VERT_RING_AD_IN_USE.UP_ODD
      CboConfigMap[2] = CBO_MSR_PMON_CTL_EVENT(0xa6) + CBO_MSR_PMON_CTL_UMASK((1 + 2));
      // UNC_CHA_VERT_RING_AD_IN_USE.DN_EVEN + UNC_CHA_VERT_RING_AD_IN_USE.DN_ODD
      CboConfigMap[3] = CBO_MSR_PMON_CTL_EVENT(0xa6) + CBO_MSR_PMON_CTL_UMASK((4 + 8));
      break;
    case pcm::PCM::ICX:
    case pcm::PCM::SPR:
      // UNC_CHA_HORZ_RING_AD_IN_USE.LEFT_EVEN + UNC_CHA_HORZ_RING_AD_IN_USE.LEFT_ODD
      CboConfigMap[0] = CBO_MSR_PMON_CTL_EVENT(0xb6) + CBO_MSR_PMON_CTL_UMASK((1 + 2));
      // UNC_CHA_HORZ_RING_AD_IN_USE.RIGHT_EVEN + UNC_CHA_HORZ_RING_AD_IN_USE.RIGHT_ODD
      CboConfigMap[1] = CBO_MSR_PMON_CTL_EVENT(0xb6) + CBO_MSR_PMON_CTL_UMASK((4 + 8));
      // UNC_CHA_VERT_RING_AD_IN_USE.UP_EVEN + UNC_CHA_VERT_RING_AD_IN_USE.UP_ODD
      CboConfigMap[2] = CBO_MSR_PMON_CTL_EVENT(0xb0) + CBO_MSR_PMON_CTL_UMASK((1 + 2));
      // UNC_CHA_VERT_RING_AD_IN_USE.DN_EVEN + UNC_CHA_VERT_RING_AD_IN_USE.DN_ODD
      CboConfigMap[3] = CBO_MSR_PMON_CTL_EVENT(0xb0) + CBO_MSR_PMON_CTL_UMASK((4 + 8));
    default:
      static_assert(true, "Error: CachelineToChaMapper not implemented for this CPUFamilyModel");
    }

    Pcm->programCboRaw(CboConfigMap.data(), 0, 0);
  }

  // for each core access cache lines of all cha boxes and find the cache lines with the lowest read latency.
  for (const auto& Cpu : Cpus) {

    // Read Cache lines into L3.
    Topology.bindCallerToOsIndex(Cpu);

    for (const auto& [Cha, Cachelines] : ChaToCachelines) {
      auto Before = Pcm->getServerUncoreCounterState(SocketIndex);

      for (auto J = 0; J < 1000; J++)
      for (auto I = 0; I < NumberOfCachelineReads; I++) {
        auto* Cacheline = static_cast<uint8_t*>(Cachelines[I]);
        // read/write cache lines. lookups into l3 will occur here.
        *Cacheline = *Cacheline + 1;
      }

      auto After = Pcm->getServerUncoreCounterState(SocketIndex);

      // create the differnece of the measurement value.
      std::array<pcm::uint64, 4> RingCounterDifferences = {0, 0, 0, 0};
      for (auto I = 0; I < 4; I++) {
        RingCounterDifferences.at(I) = After.Counters[pcm::PCM::UncorePMUIDs::CBO_PMU_ID][0][Cha][I] -
                                       Before.Counters[pcm::PCM::UncorePMUIDs::CBO_PMU_ID][0][Cha][I];
        // std::cout << "Core: " << Cpu << " CHA: " << Cha << " CounterID: " << I
        //           << " difference = " << RingCounterDifferences.at(I) << "\n";
      }
        std::cout << "Core: " << Cpu << " CHA: " << Cha
                  << " difference = " << RingCounterDifferences.at(0) + RingCounterDifferences.at(1) + RingCounterDifferences.at(2) + RingCounterDifferences.at(3)  << "\n";
    }

    // TODO: select the correct cha based on core.
  }

  return CoreToCha;
}

} // namespace cclat
