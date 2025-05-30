#pragma once

#include "core-to-core-latency/CachelineToChaMapper.hpp"
#include "core-to-core-latency/ChaMeasurementsMap.hpp"
#include "core-to-core-latency/ChaToCoreMapper.hpp"
#include "core-to-core-latency/MeasuredChasAndIngressPathsVector.hpp"
#include "core-to-core-latency/TestPair.hpp"

#include <cpucounters.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <types.h>

namespace cclat {

/// This class creates traffic between cores and records the traffic path on the ring interconnects between the CHA
/// boxes.
class CoreTrafficTest {
public:
  CoreTrafficTest() = default;

  /// Determine the path on the ring which is taken between a local r/w and a remote reading core.
  /// \arg ChaToCachelines Cache lines associtated to cha boxes.
  /// \arg ChaToCoreMap Chas associated to cores.
  /// \arg NumberOfCachelineReads The number of times one cachline is read/written to during the benchmark.
  /// \arg ClusteringThreshold The percentage between 0 and 1 of detection events relative to the
  /// NumberOfCachelineReads that is taken as the threshold for counter values to be clusted into seperate
  /// buckets for checking measurement validity.
  /// \arg DetectionThreshold The percentage between 0 and 1 of detection events relative to the
  /// NumberOfCachelineReads that need to be counted for the path to be marked as busy.
  /// \arg SocketIndex The socket that is used for this benchmark
  /// \returns the vector of measurement measured cha boxes and the ingress of all chas
  [[nodiscard]] static auto run(const ChaToCachelinesMap& ChaToCachelines, const ChaToCoreMap& ChaToCore,
                                std::size_t NumberOfCachelineReads, float ClusteringThreshold, float DetectionThreshold,
                                uint64_t SocketIndex) -> MeasuredChasAndIngressPathsVector;

  [[nodiscard]] static auto
  measureCacheline(pcm::PCM& Pcm, void* Cacheline, std::size_t NumberOfCachelineReads, const TestPair& Cores,
                   uint64_t SocketIndex, const std::function<void(unsigned)>& ThreadBindFunction) -> ChaMeasurementsMap;

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