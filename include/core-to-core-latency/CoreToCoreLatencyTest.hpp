#pragma once

#include "core-to-core-latency/Config.hpp"
#include "core-to-core-latency/CoreToCoreLatencyResults.hpp"
#include "core-to-core-latency/TestPair.hpp"
#include "firestarter/CPUTopology.hpp"

#include <barrier>
#include <functional>

namespace cclat {

class CoreToCoreLatencyTest {
public:
  CoreToCoreLatencyTest() = default;

  /// Run the latency tests
  /// \arg Cgf The config for the core to core latency run
  /// \returns the latency results for all the pairs in the config
  [[nodiscard]] auto run(const cclat::Config& Cfg) -> CoreToCoreLatencyResults;

  /// Run the latency test for one pair of local and remote cpus.
  /// \arg Pair The pair of local and remote cpus.
  /// \arg UnrollCount The number of unrolls of the experiment loop
  /// \arg InnerIterations The number of iteration of the experiment loop
  /// \arg OuterIterations The number of iterations used to average the result
  /// \returns a latency result of this run
  [[nodiscard]] auto runPair(const cclat::TestPair& Pair, const firestarter::CPUTopology& Topology,
                             unsigned UnrollCount, unsigned InnerIterations,
                             unsigned OuterIterations) -> CoreToCoreLatencyResult;

private:
  /// The thread that runs on the local cpu.
  static void localThreadFunction(uint64_t* LocalMemory, uint64_t* RemoteMemory, uint64_t CpuId,
                                  std::function<void(unsigned)> ThreadBindFunction, std::barrier<>& SyncPoint,
                                  uint64_t& DurationNs);
  /// The thread that runs on the remote cpu.
  static void remoteThreadFunction(uint64_t* LocalMemory, uint64_t* RemoteMemory, uint64_t CpuId,
                                   std::function<void(unsigned)> ThreadBindFunction, std::barrier<>& SyncPoint);
};

} // namespace cclat