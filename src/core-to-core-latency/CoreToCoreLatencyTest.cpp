#include "core-to-core-latency/CoreToCoreLatencyTest.hpp"
#include "core-to-core-latency/CoreToCoreLatencyResults.hpp"
#include "firestarter/AlignedAlloc.hpp"
#include "firestarter/CPUTopology.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

namespace cclat {

auto CoreToCoreLatencyTest::run(const cclat::Config& Cfg) -> CoreToCoreLatencyResults {
  std::vector<CoreToCoreLatencyResult> Results;
  firestarter::CPUTopology Topology;

  for (const auto& Test : Cfg.Tests.List) {
    const auto Result = runPair(Test, Topology, Cfg.UnrollCount, Cfg.InnerIterations, Cfg.OuterIterations);
    Results.emplace_back(Result);
  }

  return CoreToCoreLatencyResults{.Results = Results};
}

auto CoreToCoreLatencyTest::runPair(const cclat::TestPair& Pair, const firestarter::CPUTopology& Topology,
                                    unsigned UnrollCount, unsigned InnerIterations,
                                    unsigned OuterIterations) -> CoreToCoreLatencyResult {

  // 1. start two threads and pin them to the local and remote cpus.
  // 2. create two aligned memory regions and perform first touch on local and remote cpus respectively.

  // 3. LOCAL THREAD read on the remote memory for a while, perform a write and flush.
  // 4. REMOTE THREAD spin loop on memory until the signal is read.
  // 5. repeat with local and remote threads switched.
  // 6. LOCAL THREAD read time between setting the signal and receiving the signal.

  uint64_t MinValue = std::numeric_limits<uint64_t>::max();

  for (auto I = 0; I < InnerIterations; I++) {

    auto* LocalMemory = static_cast<uint64_t*>(firestarter::AlignedAlloc::malloc(64));
    auto* RemoteMemory = static_cast<uint64_t*>(firestarter::AlignedAlloc::malloc(64));

    auto ThreadBindFunction = [&Topology](auto&& PH1) {
      Topology.bindCallerToOsIndex(std::forward<decltype(PH1)>(PH1));
    };

    std::barrier SyncPoint(2);
    uint64_t DurationNs{};

    std::thread LocalThread(localThreadFunction, LocalMemory, RemoteMemory, Pair.LocalCpu, ThreadBindFunction,
                            std::ref(SyncPoint), std::ref(DurationNs));
    std::thread RemoteThread(remoteThreadFunction, LocalMemory, RemoteMemory, Pair.RemoteCpu, ThreadBindFunction,
                             std::ref(SyncPoint));

    RemoteThread.join();
    LocalThread.join();

    MinValue = std::min(DurationNs, MinValue);
  }

  std::cout << "Local: " << Pair.LocalCpu << " Remote: " << Pair.RemoteCpu << " MinDurationNs: " << MinValue << '\n';

  return CoreToCoreLatencyResult{};
}

void CoreToCoreLatencyTest::localThreadFunction(uint64_t* LocalMemory, uint64_t* RemoteMemory, uint64_t CpuId,
                                                std::function<void(unsigned)> ThreadBindFunction,
                                                std::barrier<>& SyncPoint, uint64_t& DurationNs) {
  ThreadBindFunction(CpuId);

  // first touch
  *LocalMemory = 0;
  SyncPoint.arrive_and_wait();

  // read remote memory. write to some ptr, so the compiler does not optimize this code
  *LocalMemory = *RemoteMemory;
  SyncPoint.arrive_and_wait();

  std::this_thread::sleep_for(std::chrono::microseconds(1));

  auto Start = std::chrono::high_resolution_clock::now();

  // send the signal
  *RemoteMemory = 1;
  __asm__ __volatile__("mfence;");

  // wait for the back signal
  while (*LocalMemory == 0) {
  }

  auto Stop = std::chrono::high_resolution_clock::now();

  DurationNs = std::chrono::duration_cast<std::chrono::nanoseconds>(Stop - Start).count();
}

void CoreToCoreLatencyTest::remoteThreadFunction(uint64_t* LocalMemory, uint64_t* RemoteMemory, uint64_t CpuId,
                                                 std::function<void(unsigned)> ThreadBindFunction,
                                                 std::barrier<>& SyncPoint) {
  ThreadBindFunction(CpuId);

  // first touch
  *RemoteMemory = 0;
  SyncPoint.arrive_and_wait();

  // read local memory. write to some ptr, so the compiler does not optimize this code
  *RemoteMemory = *LocalMemory;
  SyncPoint.arrive_and_wait();

  // wait for signal
  while (*RemoteMemory == 0) {
  }

  // signal back
  *LocalMemory = 1;
  __asm__ __volatile__("mfence;");
}

} // namespace cclat