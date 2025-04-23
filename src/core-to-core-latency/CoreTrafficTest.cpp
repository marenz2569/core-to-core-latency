#include "core-to-core-latency/CoreTrafficTest.hpp"
#include "core-to-core-latency/PcmRingCounters.hpp"
#include "firestarter/CPUTopology.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <thread>
#include <unordered_map>

namespace cclat {

auto CoreTrafficTest::run(const ChaToCachelinesMap& ChaToCachelines, const CoreToChaMap& CoreToCha,
                          const std::size_t NumberOfCachelineReads,
                          const uint64_t SocketIndex) -> CoreToChaBusyPathMap {
  CoreToChaBusyPathMap CoreToChaBusyPath;
  using CoresToChaMeasurementsMap = std::unordered_map<TestPair, std::map<uint64_t, std::array<pcm::uint64, 4>>>;
  CoresToChaMeasurementsMap Measurements;

  firestarter::CPUTopology Topology;

  auto ThreadBindFunction = [&Topology](auto&& PH1) { Topology.bindCallerToOsIndex(std::forward<decltype(PH1)>(PH1)); };

  // start the counter on all CHAs
  auto* Pcm = pcm::PCM::getInstance();

  PcmRingCounters::programmADRingCounters(Pcm);

  for (const auto& [LocalCore, LocalCha] : CoreToCha) {
    const auto& Cachelines = ChaToCachelines.at(LocalCha);

    // Create the min of measurement values for all cache lines.
    for (const auto& [RemoteCore, RemoteCha] : CoreToCha) {
      auto& ChaMeasurements = Measurements[TestPair{.LocalCpu = LocalCore, .RemoteCpu = RemoteCore}];

      // flush before use
      for (const auto& VoidCacheline : Cachelines) {
        auto* Cacheline = static_cast<uint8_t*>(VoidCacheline);
        asm __volatile__("mfence\n"
                         "lfence\n"
                         "clflush (%[addr])\n"
                         "mfence\n"
                         "lfence" ::[addr] "r"(Cacheline)
                         : "memory");
      }

      for (const auto& Cacheline : Cachelines) {
        auto Before = Pcm->getServerUncoreCounterState(SocketIndex);

        std::thread LocalThread(localThreadFunction, Cacheline, NumberOfCachelineReads, LocalCore,
                                std::cref(ThreadBindFunction));
        std::thread RemoteThread(remoteThreadFunction, Cacheline, NumberOfCachelineReads, RemoteCore,
                                 std::cref(ThreadBindFunction));

        RemoteThread.join();
        LocalThread.join();

        auto After = Pcm->getServerUncoreCounterState(SocketIndex);

        // create the differnece of the measurement value.
        for (auto ChaIndex = 0; ChaIndex < Pcm->getMaxNumOfUncorePMUs(pcm::PCM::UncorePMUIDs::CBO_PMU_ID, SocketIndex);
             ChaIndex++) {
          std::array<pcm::uint64, 4> RingCounterDifferences = {0, 0, 0, 0};
          for (auto I = 0; I < 4; I++) {
            RingCounterDifferences.at(I) = After.Counters[pcm::PCM::UncorePMUIDs::CBO_PMU_ID][0][ChaIndex][I] -
                                           Before.Counters[pcm::PCM::UncorePMUIDs::CBO_PMU_ID][0][ChaIndex][I];
          }

          // initialize for std::min
          if (!ChaMeasurements.contains(ChaIndex)) {
            ChaMeasurements[ChaIndex] = {
                std::numeric_limits<pcm::uint64>::max(), std::numeric_limits<pcm::uint64>::max(),
                std::numeric_limits<pcm::uint64>::max(), std::numeric_limits<pcm::uint64>::max()};
          }

          ChaMeasurements[ChaIndex] = std::min(ChaMeasurements[ChaIndex], RingCounterDifferences);
        }
      }

      std::cout << "Local core: " << LocalCore << " Local cha: " << LocalCha << "\n";
      std::cout << "Remote core: " << RemoteCore << " Remote cha: " << RemoteCha << "\n";

      for (const auto& [Cha, MinValues] : ChaMeasurements) {
        std::cout << "LocalCore: " << LocalCore << " RemoteCore: " << RemoteCore << " Cha: " << Cha
                  << " Left: " << MinValues.at(PcmRingCounters::Direction::Left)
                  << " Right: " << MinValues.at(PcmRingCounters::Direction::Right)
                  << " Up: " << MinValues.at(PcmRingCounters::Direction::Up)
                  << " Down: " << MinValues.at(PcmRingCounters::Direction::Down) << "\n";
      }
    }
  }

  for (const auto& [Cores, ChaMeasurements] : Measurements) {
  }

  return CoreToChaBusyPath;
}

void CoreTrafficTest::localThreadFunction(void* VoidCacheline, const std::size_t NumberOfCachelineReads, uint64_t CpuId,
                                          const std::function<void(unsigned)>& ThreadBindFunction) {
  ThreadBindFunction(CpuId);

  for (auto I = 0; I < NumberOfCachelineReads; I++) {
    auto* Cacheline = static_cast<uint8_t*>(VoidCacheline);
    // read/write cache lines. lookups into l3 will occur here.
    *Cacheline = *Cacheline + 1;
  }
}

void CoreTrafficTest::remoteThreadFunction(void* VoidCacheline, const std::size_t NumberOfCachelineReads,
                                           uint64_t CpuId, const std::function<void(unsigned)>& ThreadBindFunction) {
  ThreadBindFunction(CpuId);

  uint8_t Sum{};

  for (auto I = 0; I < NumberOfCachelineReads; I++) {
    auto* Cacheline = static_cast<uint8_t*>(VoidCacheline);
    // read/write cache lines. lookups into l3 will occur here.
    Sum += *Cacheline;
  }

  (void)Sum;
}

} // namespace cclat