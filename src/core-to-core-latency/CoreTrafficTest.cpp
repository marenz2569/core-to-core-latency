#include "core-to-core-latency/CoreTrafficTest.hpp"
#include "core-to-core-latency/PcmRingCounters.hpp"
#include "firestarter/CPUTopology.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <thread>
#include <unordered_map>

namespace cclat {

auto CoreTrafficTest::measureCacheline(pcm::PCM& Pcm, void* Cacheline, const std::size_t NumberOfCachelineReads,
                                       const TestPair& Cores, const uint64_t SocketIndex,
                                       const std::function<void(unsigned)>& ThreadBindFunction) -> ChaMeasurementsMap {
  ChaMeasurementsMap ChaMeasurements;

  auto Before = Pcm.getServerUncoreCounterState(SocketIndex);

  std::thread LocalThread(localThreadFunction, Cacheline, NumberOfCachelineReads, Cores.LocalCpu,
                          std::cref(ThreadBindFunction));
  std::thread RemoteThread(remoteThreadFunction, Cacheline, NumberOfCachelineReads, Cores.RemoteCpu,
                           std::cref(ThreadBindFunction));

  RemoteThread.join();
  LocalThread.join();

  auto After = Pcm.getServerUncoreCounterState(SocketIndex);

  // create the differnece of the measurement value.
  for (auto ChaIndex = 0; ChaIndex < Pcm.getMaxNumOfUncorePMUs(pcm::PCM::UncorePMUIDs::CBO_PMU_ID, SocketIndex);
       ChaIndex++) {
    for (auto I = 0; I < 4; I++) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
      ChaMeasurements[ChaIndex][I] = After.Counters[pcm::PCM::UncorePMUIDs::CBO_PMU_ID][0][ChaIndex][I] -
                                     Before.Counters[pcm::PCM::UncorePMUIDs::CBO_PMU_ID][0][ChaIndex][I];
    }
  }

  return ChaMeasurements;
}

auto CoreTrafficTest::run(const ChaToCachelinesMap& ChaToCachelines, const CoreToChaMap& CoreToCha,
                          const std::size_t NumberOfCachelineReads, const float ClusteringThreshold,
                          const float DetectionThreshold, const uint64_t SocketIndex) -> CoreToChaBusyPathMap {
  CoreToChaBusyPathMap CoreToChaBusyPath;

  firestarter::CPUTopology Topology;

  const auto AbsoluteClusteringThreshold =
      static_cast<std::size_t>(static_cast<float>(NumberOfCachelineReads) * ClusteringThreshold);
  const auto AbsoluteDetectionThreshold =
      static_cast<std::size_t>(static_cast<float>(NumberOfCachelineReads) * DetectionThreshold);

  auto ThreadBindFunction = [&Topology](auto&& PH1) { Topology.bindCallerToOsIndex(std::forward<decltype(PH1)>(PH1)); };

  // start the counter on all CHAs
  auto* Pcm = pcm::PCM::getInstance();

  PcmRingCounters::programmBLRingCounters(Pcm);

  for (const auto& [LocalCore, LocalCha] : CoreToCha) {
    const auto& Cachelines = ChaToCachelines.at(LocalCha);

    // Create the min of measurement values for all cache lines.
    for (const auto& [RemoteCore, RemoteCha] : CoreToCha) {
      // skip self edge
      if (LocalCore == RemoteCore) {
        continue;
      }

      const auto Cores = TestPair{.LocalCpu = LocalCore, .RemoteCpu = RemoteCore};

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

      auto ValidResult = false;

      // Run the measurement for one cacheline and repeat with the next if the result is not valid.
      for (const auto& Cacheline : Cachelines) {
        auto Result = measureCacheline(*Pcm, Cacheline, NumberOfCachelineReads, Cores, SocketIndex, ThreadBindFunction);

        // The result of the measurement is valid if there are only a couple of distinct counter value clusters.
        // If there are too many counter value clusters, we must assume that the mapping of the
        // cachelines to the CHA is wrong.

        std::set<uint64_t> CounterValues;
        for (const auto& [Cha, Values] : Result) {
          CounterValues.emplace(Values.at(PcmRingCounters::Direction::Left) / AbsoluteClusteringThreshold);
          CounterValues.emplace(Values.at(PcmRingCounters::Direction::Right) / AbsoluteClusteringThreshold);
          CounterValues.emplace(Values.at(PcmRingCounters::Direction::Up) / AbsoluteClusteringThreshold);
          CounterValues.emplace(Values.at(PcmRingCounters::Direction::Down) / AbsoluteClusteringThreshold);
        }

        // too many clusters, retry the measurement
        if (CounterValues.size() > 3) {
          continue;
        }

        std::cout << "Local core: " << LocalCore << " Local cha: " << LocalCha << "\n";
        std::cout << "Remote core: " << RemoteCore << " Remote cha: " << RemoteCha << "\n";
        std::cout << "We have " << CounterValues.size() << " unique clusters.\n";

        std::cout << "Values: ";
        for (const auto& Value : CounterValues) {
          std::cout << Value << " ";
        }
        std::cout << "\n";

        for (const auto& [Cha, Values] : Result) {
          std::cout << "LocalCore: " << LocalCore << " RemoteCore: " << RemoteCore << " Cha: " << Cha
                    << " Left: " << Values.at(PcmRingCounters::Direction::Left)
                    << " Right: " << Values.at(PcmRingCounters::Direction::Right)
                    << " Up: " << Values.at(PcmRingCounters::Direction::Up)
                    << " Down: " << Values.at(PcmRingCounters::Direction::Down) << "\n";
        }

        ValidResult = true;
        break;
      }

      if (!ValidResult) {
        throw std::runtime_error("Could not find setting for Local core: " + std::to_string(LocalCore) +
                                 " Local cha: " + std::to_string(LocalCha) + " Remote core: " +
                                 std::to_string(RemoteCore) + " Remote cha: " + std::to_string(RemoteCha));
      }
    }
  }

  return CoreToChaBusyPath;
}

void CoreTrafficTest::localThreadFunction(void* VoidCacheline, const std::size_t NumberOfCachelineReads, uint64_t CpuId,
                                          const std::function<void(unsigned)>& ThreadBindFunction) {
  ThreadBindFunction(CpuId);

  for (auto I = 0; I < NumberOfCachelineReads; I++) {
    volatile auto* Cacheline = static_cast<uint8_t*>(VoidCacheline);
    // read/write cache lines.
    *Cacheline = *Cacheline + 1;
  }
}

void CoreTrafficTest::remoteThreadFunction(void* VoidCacheline, const std::size_t NumberOfCachelineReads,
                                           uint64_t CpuId, const std::function<void(unsigned)>& ThreadBindFunction) {
  ThreadBindFunction(CpuId);

  uint8_t Sum{};

  for (auto I = 0; I < NumberOfCachelineReads; I++) {
    volatile auto* Cacheline = static_cast<uint8_t*>(VoidCacheline);
    // read cache lines. we will see ingress from the local cha to this core.
    Sum += *Cacheline;
  }

  (void)Sum;
}

} // namespace cclat
