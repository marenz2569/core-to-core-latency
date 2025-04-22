#include "core-to-core-latency/CachelineToChaMapper.hpp"
#include "types.h"

#include <cpucounters.h>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace cclat {

auto CachelineToChaMapper::run(void* Cachelines, std::size_t NumberOfCachelines, std::size_t NumberOfCachelineReads,
                               uint64_t SocketIndex) -> ChaToCachelinesMap {
  ChaToCachelinesMap ChaToCachelines;

  // start the counter on all CHAs
  auto* Pcm = pcm::PCM::getInstance();
  auto Pmu = pcm::ServerUncorePMUs(/*socket_=*/SocketIndex, /*pcm=*/Pcm);

  {
    std::array<pcm::uint64, 4> CboConfigMap = {0, 0, 0, 0};

    switch (Pcm->getCPUFamilyModel()) {
    // Skylake-X
    // perfmon/SKX/events/skylakex_uncore.json
    // UNC_CHA_REQUESTS.READS
    case pcm::PCM::SKX:
    // Sapphire Rappids
    // perfmon/SPR/events/sapphirerapids_uncore_experimental.json
    // UNC_CHA_REQUESTS.READS_LOCAL + UNC_CHA_REQUESTS.READS_REMOTE
    case pcm::PCM::SPR:
      CboConfigMap[0] = CBO_MSR_PMON_CTL_EVENT(0x50) + CBO_MSR_PMON_CTL_UMASK((1 + 2));
      break;
    default:
      throw std::runtime_error("CachelineToChaMapper not implemented for this CPUFamilyModel");
    }

    Pcm->programCboRaw(CboConfigMap.data(), 0, 0);
  }

  for (auto CachelineIndex = 0; CachelineIndex < NumberOfCachelines; CachelineIndex++) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto* Cacheline = static_cast<uint8_t*>(Cachelines) + static_cast<ptrdiff_t>(64 * CachelineIndex);

    // flush, read, flush and repeat. the uncore counter for CHA reads will increment if this cacheline is in the
    // counter.

    auto Before = Pcm->getServerUncoreCounterState(SocketIndex);

    volatile uint8_t Sum = 0;
    for (auto I = 0; I < NumberOfCachelineReads; I++) {
      Sum = Sum + *Cacheline;
      asm __volatile__("mfence\n"
                       "lfence\n"
                       "clflush (%[addr])\n"
                       "mfence\n"
                       "lfence" ::[addr] "r"(Cacheline)
                       : "memory");
    }

    (void)Sum;

    // stop the counter on all CHAs
    auto After = Pcm->getServerUncoreCounterState(SocketIndex);

    // map the difference of counter values index by the CHA box.
    std::map<uint64_t, uint64_t> ChaToCounterValueMap;

    // create the differnece of the measurement value.
    for (auto ChaIndex = 0; ChaIndex < Pcm->getMaxNumOfUncorePMUs(pcm::PCM::UncorePMUIDs::CBO_PMU_ID, SocketIndex);
         ChaIndex++) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
      ChaToCounterValueMap[ChaIndex] = After.Counters[pcm::PCM::UncorePMUIDs::CBO_PMU_ID][0][ChaIndex][0] -
                                       Before.Counters[pcm::PCM::UncorePMUIDs::CBO_PMU_ID][0][ChaIndex][0];
    }

    // find the CHA index where approximatly NumberOfCachelineReads occured.
    uint64_t SelectedChaIndex{};

    for (const auto& [Cha, Value] : ChaToCounterValueMap) {
      if (Value > static_cast<std::size_t>(0.9 * static_cast<double>(NumberOfCachelineReads))) {
        SelectedChaIndex = Cha;
      }
    }

    ChaToCachelines[SelectedChaIndex].emplace_back(Cacheline);
  }

  return ChaToCachelines;
}

} // namespace cclat
