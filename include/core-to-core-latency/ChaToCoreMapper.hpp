#pragma once

#include "core-to-core-latency/CachelineToChaMapper.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
namespace cclat {

/// The map of a cha box id to core id.
using ChaToCoreMap = std::map<uint64_t, uint64_t>;

/// This class maps cache lines to a specific CHA box via the UNC_CHA_REQUESTS.READS_* counter.
class ChaToCoreMapper {
public:
  ChaToCoreMapper() = default;

  /// Determine the mapping of cha boxes to cores on one socket.
  /// \arg ChaToCachelines Cache lines associtated to cha boxes.
  /// \arg NumberOfCachelineReads The number of times each cachline is read during the benchmark.
  /// \arg Cpus The cpus that should be tested.
  /// \arg SocketIndex The socket that is used for this benchmark
  /// \returns the map of cha boxes to cores
  [[nodiscard]] static auto run(const ChaToCachelinesMap& ChaToCachelines, std::size_t NumberOfCachelineReads,
                                const std::set<uint64_t>& Cpus, uint64_t SocketIndex) -> ChaToCoreMap;
};

} // namespace cclat