#pragma once

#include "core-to-core-latency/CachelineToChaMapper.hpp"
#include "core-to-core-latency/ChaToCoreMapper.hpp"
#include "core-to-core-latency/TestPair.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>

namespace cclat {

/// The map of a core id to cha box id.
using ChaToBusyPathMap = std::map<uint64_t, uint64_t>;

// TODO use std::map with test pair
using CoreToChaBusyPathMap = std::vector<std::pair<TestPair, std::vector<ChaToBusyPathMap>>>;

/// This class creates traffic between cores and records the traffic path on the ring interconnects between the CHA
/// boxes.
class CoreTrafficTest {
public:
  CoreTrafficTest() = default;

  /// Determine the path on the ring which is taken between a local r/w and a remote reading core.
  /// \arg ChaToCachelines Cache lines associtated to cha boxes.
  /// \arg CoreToChaMap Chas associated to cores.
  /// \arg NumberOfCachelineReads The number of times each cachline is read during the benchmark.
  /// \arg SocketIndex The socket that is used for this benchmark
  /// \returns the map of core to cha boxes
  [[nodiscard]] static auto run(const ChaToCachelinesMap& ChaToCachelines, const CoreToChaMap& CoreToCha,
                                std::size_t NumberOfCachelineReads, uint64_t SocketIndex) -> CoreToChaBusyPathMap;

  /// The local thread function that reads and writes to the cachelines.
  /// \arg Cachelines The cacheline to which the thread read and writes
  /// \arg NumberOfCachelineReads The number of times each cachline is read/written to.
  /// \arg CpuId The id of the cpu the thread runs on
  /// \arg ThreadBindFunction The function that is used to bind the thread to the supplied cpu id
  static void localThreadFunction(void* Cacheline, std::size_t NumberOfCachelineReads, uint64_t CpuId,
                                  const std::function<void(unsigned)>& ThreadBindFunction);

  /// The remote thread function that reads the cachelines.
  /// \arg Cachelines The cacheline to which the thread read and writes
  /// \arg NumberOfCachelineReads The number of times each cachline is read
  /// \arg CpuId The id of the cpu the thread runs on
  /// \arg ThreadBindFunction The function that is used to bind the thread to the supplied cpu id
  static void remoteThreadFunction(void* Cacheline, std::size_t NumberOfCachelineReads, uint64_t CpuId,
                                   const std::function<void(unsigned)>& ThreadBindFunction);
};

} // namespace cclat